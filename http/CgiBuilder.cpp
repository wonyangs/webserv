#include "CgiBuilder.hpp"

/**
 * Constructor & Destructor
 */

CgiBuilder::CgiBuilder(Request const& request)
    : AResponseBuilder(CGI, request),
      _pid(-1),
      _readPipeFd(-1),
      _writePipeFd(-1),
      _writeIndex(0) {}

CgiBuilder::CgiBuilder(CgiBuilder const& builder) : AResponseBuilder(builder) {
  _pid = builder._pid;
  _readPipeFd = builder._readPipeFd;
  _writePipeFd = builder._writePipeFd;
  _writeIndex = builder._writeIndex;
  _storageBuffer = builder._storageBuffer;
  _cgiPathInfo = builder._cgiPathInfo;
}

CgiBuilder::~CgiBuilder(void) { close(); }

/**
 * Operator Overloading
 */

CgiBuilder& CgiBuilder::operator=(CgiBuilder const& builder) {
  if (this != &builder) {
    _response = builder._response;
    _isDone = builder._isDone;
    setRequest(builder.getRequest());
    setType(builder.getType());

    _pid = builder._pid;
    _readPipeFd = builder._readPipeFd;
    _writePipeFd = builder._writePipeFd;
    _writeIndex = builder._writeIndex;
    _storageBuffer = builder._storageBuffer;
    _cgiPathInfo = builder._cgiPathInfo;
  }
  return *this;
}

/**
 * Public method
 */

// 첫 번째 호출 시 CGI 생성
// - 이후 호출부터 이벤트에 따라 읽기 또는 쓰기 진행
std::vector<int> const CgiBuilder::build(Event::EventType type) {
  checkPathInfo();
  if (_pid == -1) {
    return forkCgi();
  }

  switch (type) {
    case Event::READ:
      handleReadEvent();
      break;
    case Event::WRITE:
      handleWriteEvent();
      break;
    default:
      throw std::runtime_error("[5400] CgiBuilder: build - unknown event");
      break;
  }
  return std::vector<int>();
}

void CgiBuilder::close(void) {
  if (_pid != -1 and (_readPipeFd != -1 or _writePipeFd != -1)) {
    kill(_pid, SIGKILL);
    _pid = -1;
  }
  if (_readPipeFd != -1) {
    Kqueue::removeReadEvent(_readPipeFd);
    ::close(_readPipeFd);
    _readPipeFd = -1;
  }
  if (_writePipeFd != -1) {
    Kqueue::removeWriteEvent(_writePipeFd);
    ::close(_writePipeFd);
    _writePipeFd = -1;
  }
}

/**
 * Private method - CGI init
 */

// CGI Builder에게 전달되는 path 정보가 올바른지 검사
// - CGI extension으로 끝나지 않는 경로인 경우 403 예외 발생
void CgiBuilder::checkPathInfo(void) {
  Request const& request = getRequest();

  // 디렉토리로 끝나는 경우 index 정보를 붙여 확인
  if (request.getFullPath().back() == '/') {
    _cgiPathInfo = request.generateIndexPath();
  } else {
    _cgiPathInfo = request.getFullPath();
  }

  // CGI extension 정보 확인
  std::string const& extension = request.getLocation().getCgiExtension();
  if (endsWith(_cgiPathInfo, extension) == false) {
    throw StatusException(
        HTTP_FORBIDDEN, "[5401] CgiBuilder: checkPathInfo - invalid extension");
  }
}

// CGI 초기화
// - 시스템콜 호출에 실패한 경우 예외 발생
std::vector<int> const CgiBuilder::forkCgi(void) {
  int p_to_c[2];
  int c_to_p[2];

  if (pipe(p_to_c) < 0 or pipe(c_to_p) < 0) {
    throw std::runtime_error("[5402] CgiBuilder: forkCgi - pipe fail");
  }
  if ((_pid = fork()) < 0) {
    throw std::runtime_error("[5403] CgiBuilder: forkCgi - fork fail");
  }

  Socket::setNonBlocking(p_to_c[0]);
  Socket::setNonBlocking(p_to_c[1]);
  Socket::setNonBlocking(c_to_p[0]);
  Socket::setNonBlocking(c_to_p[1]);

  if (_pid == 0) {
    childProcess(p_to_c, c_to_p);
  } else {
    parentProcess(p_to_c, c_to_p);
  }

  std::vector<int> fds;
  fds.push_back(_readPipeFd);
  fds.push_back(_writePipeFd);
  return fds;
}

// 부모 프로세스 작업
// - pipe 이벤트 등록
void CgiBuilder::parentProcess(int* const p_to_c, int* const c_to_p) {
  ::close(p_to_c[0]);
  ::close(c_to_p[1]);

  _writePipeFd = p_to_c[1];
  _readPipeFd = c_to_p[0];

  Kqueue::addReadEvent(_readPipeFd);
  Kqueue::addWriteEvent(_writePipeFd);
}

// 자식 프로세스 작업
void CgiBuilder::childProcess(int* const p_to_c, int* const c_to_p) {
  try {
    ::close(p_to_c[1]);
    ::close(c_to_p[0]);

    dup2(p_to_c[0], STDIN_FILENO);
    ::close(p_to_c[0]);

    dup2(c_to_p[1], STDOUT_FILENO);
    ::close(c_to_p[1]);

    // 환경변수 설정
    char** envp = makeEnv();

    // cgi 있는지 + 실행권한 확인
    std::string const& cgiPath = getRequest().getLocation().getCgiPath();

    if (access(cgiPath.c_str(), F_OK) < 0 or
        access(cgiPath.c_str(), X_OK) < 0) {
      throw std::runtime_error("CGI Access Fail");
    }

    // cgi에 인자 및 환경변수 전송
    if (execve(cgiPath.c_str(), NULL, envp) < 0) {
      freeEnvArray(envp);
      throw std::runtime_error("CGI Execve Fail");
    }
  } catch (std::exception const& e) {
    std::cout << "Status: 500 " << e.what() << "\r\n";
    std::cout << "Content-Type: text/html\r\n\r\n";
    std::cout << Config::defaultErrorPageBody(500);
    exit(1);
  }
}

/**
 * Private method - handle event
 */

// 파이프에서 CGI의 결과물을 읽기
void CgiBuilder::handleReadEvent(void) {
  // 파일 읽기
  u_int8_t buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  ssize_t bytesRead = read(_readPipeFd, buffer, sizeof(buffer));

  if (bytesRead < 0) {
    throw std::runtime_error(
        "[5404] CgiBuilder: handleReadEvent - read failed");
  }

  for (ssize_t i = 0; i < bytesRead; i++) {
    _storageBuffer.push_back(buffer[i]);
  }

  if (bytesRead == 0) {  // eof에 도달
    Kqueue::removeReadEvent(_readPipeFd);
    ::close(_readPipeFd);
    _readPipeFd = -1;

    std::string cgiResponse(_storageBuffer.begin(), _storageBuffer.end());
    buildResponseContent(cgiResponse);
    _isDone = true;
  }
}

// 파이프에 CGI에게 보낼 내용을 쓰기
void CgiBuilder::handleWriteEvent(void) {
  std::string const& body = getRequest().getBody();

  size_t sendSize;
  if (BUFFER_SIZE < body.size() - _writeIndex) {
    sendSize = BUFFER_SIZE;
  } else {
    sendSize = body.size() - _writeIndex;
  }

  // cgi 표준 입력에 body 전송
  ssize_t bytesWrite =
      write(_writePipeFd, body.c_str() + _writeIndex, sendSize);
  _writeIndex += bytesWrite;

  if (bytesWrite < 0) {
    throw std::runtime_error(
        "[5405] CgiBuilder: handleWriteEvent - write failed");
  }

  if (_writeIndex >= body.size()) {  // 모든 내용 전송 완료
    Kqueue::removeWriteEvent(_writePipeFd);
    ::close(_writePipeFd);
    _writePipeFd = -1;
  }
}

/**
 * Private method - make response
 */

void CgiBuilder::buildResponseContent(std::string const& cgiResponse) {
  bool statusCodeFlag = false;
  int splitDelimiter = 4;

  // 헤더와 본문을 분리하기 위한 위치 찾기
  std::size_t headerEndPos = cgiResponse.find("\r\n\r\n");
  if (headerEndPos == std::string::npos) {
    headerEndPos = cgiResponse.find("\n\n");
    splitDelimiter = 2;
  }

  // 헤더 구분자가 없는 경우, 전체 응답을 본문으로 간주
  if (headerEndPos == std::string::npos) {
    _response.appendBody(cgiResponse);

    _response.setHttpVersion("HTTP/1.1");
    _response.setStatusCode(200);
    _response.addHeader("Content-Length", Util::itos(cgiResponse.size()));
    _response.addHeader("Connection", "keep-alive");

    // 응답 내용 구성
    _response.makeResponseContent();
    return;
  }

  // 헤더 추출
  std::string headerPart = cgiResponse.substr(0, headerEndPos);

  // 본문 추출
  std::string bodyPart = cgiResponse.substr(headerEndPos + splitDelimiter);

  // 헤더 파싱 및 _response 객체에 설정
  std::istringstream headerStream(headerPart);
  std::string headerLine;
  while (std::getline(headerStream, headerLine)) {
    std::size_t colonPos = headerLine.find(':');
    if (colonPos != std::string::npos) {
      std::string headerName = headerLine.substr(0, colonPos);
      std::string headerValue = headerLine.substr(colonPos + 1);

      // 헤더 이름과 값의 앞뒤 공백 제거
      trim(headerName);
      trim(headerValue);

      // "Status" 헤더에서 상태 코드 추출 및 설정 -> todo: 첫줄만 읽기
      if (headerName == "Status") {
        std::istringstream iss(headerValue);
        int statusCode;
        std::string statusText;

        iss >> statusCode;  // 상태 코드를 정수로 읽어들임
        std::getline(iss, statusText);  // 상태 텍스트 읽기 (예: "OK")

        std::cout << "cgi status text: " << statusText << std::endl;  // debug

        if (statusCode > 0) {
          _response.setStatusCode(statusCode);
        }
        statusCodeFlag = true;
      } else {
        // 일반 헤더 처리
        _response.addHeader(headerName, headerValue);
      }
    } else {
      // :이 발견되지 않은 경우
      _response.addHeader(headerLine, "");
    }
  }

  _response.setHttpVersion("HTTP/1.1");
  if (statusCodeFlag == false) {
    _response.setStatusCode(200);
  }

  if (_response.isHeaderFieldNameExists("Content-Length") == false) {
    _response.addHeader("Content-Length", Util::itos(bodyPart.size()));
    _response.appendBody(bodyPart);
  } else {
    int length =
        Util::stoi(_response.getHeader().find("Content-Length")->second);
    _response.appendBody(bodyPart.substr(0, length));
  }

  // 응답 내용 구성
  _response.makeResponseContent();
}

void CgiBuilder::trim(std::string& str) {
  std::size_t start = str.find_first_not_of(" \t\r\n");
  std::size_t end = str.find_last_not_of(" \t\r\n");

  if (start == std::string::npos) {
    // 문자열이 전부 공백인 경우
    str.clear();
  } else {
    // 앞뒤 공백을 제거한 부분 문자열을 추출
    str = str.substr(start, end - start + 1);
  }
}

// 문자열의 마지막이 특정 문자열로 끝나는지 확인
bool CgiBuilder::endsWith(const std::string& fullString,
                          const std::string& ending) {
  if (fullString.length() >= ending.length()) {
    return (0 == fullString.compare(fullString.length() - ending.length(),
                                    ending.length(), ending));
  } else {
    return false;
  }
}

/**
 * Private method - env
 */

// cgi에 필요한 환경변수를 설정하여 반환
char** CgiBuilder::makeEnv(void) {
  Request const& request = getRequest();
  Location const& location = request.getLocation();
  std::map<std::string, std::string> env;

  env["ROOT_PATH"] = location.getRootPath();
  env["CGI_PATH"] = location.getCgiPath();
  env["UPLOAD_PATH"] = location.getUploadDirPath();

  if (request.isHeaderFieldNameExists("authorization")) {
    env["AUTH_TYPE"] = request.getHeaderFieldValues("authorization");
  }

  env["CONTENT_LENGTH"] = Util::itos(request.getBody().size());

  if (request.isHeaderFieldNameExists("content-type")) {
    env["CONTENT_TYPE"] = request.getHeaderFieldValues("content-type");
  }

  env["GATEWAY_INTERFACE"] = "CGI/1.1";

  if (request.isHeaderFieldNameExists("accept")) {
    env["HTTP_ACCEPT"] = request.getHeaderFieldValues("accept");
  }
  if (request.isHeaderFieldNameExists("accept-charset")) {
    env["HTTP_ACCEPT_CHARSET"] = request.getHeaderFieldValues("accept-charset");
  }
  if (request.isHeaderFieldNameExists("accept-encoding")) {
    env["HTTP_ACCEPT_ENCODING"] =
        request.getHeaderFieldValues("accept-encoding");
  }
  if (request.isHeaderFieldNameExists("accept-language")) {
    env["HTTP_ACCEPT_LANGUAGE"] =
        request.getHeaderFieldValues("accept-language");
  }
  if (request.isHeaderFieldNameExists("forwarded")) {
    env["HTTP_FORWARDED"] = request.getHeaderFieldValues("forwarded");
  }
  if (request.isHeaderFieldNameExists("host")) {
    env["HTTP_HOST"] = request.getHeaderFieldValues("host");
  }
  if (request.isHeaderFieldNameExists("proxy-authorization")) {
    env["HTTP_PROXY_AUTHORIZATION"] =
        request.getHeaderFieldValues("proxy-authorization");
  }
  if (request.isHeaderFieldNameExists("user-agent")) {
    env["HTTP_USER_AGENT"] = request.getHeaderFieldValues("user-agent");
  }

  env["PATH_INFO"] = _cgiPathInfo;

  env["QUERY_STRING"] = request.getQuery();
  switch (request.getMethod()) {
    case HTTP_GET:
      env["REQUEST_METHOD"] = "GET";
      break;
    case HTTP_POST:
      env["REQUEST_METHOD"] = "POST";
      break;
    case HTTP_DELETE:
      env["REQUEST_METHOD"] = "DELETE";
      break;
    default:
      break;
  }

  env["SERVER_PROTOCOL"] = "HTTP/1.1";
  env["SERVER_SOFTWARE"] = "webserv/1.0";

  if (request.isHeaderFieldNameExists("cookie")) {
    env["HTTP_COOKIE"] = request.getHeaderFieldValues("cookie");
  }

  std::map<std::string, std::string> headers = getRequest().getHeader();
  std::map<std::string, std::string>::const_iterator it = headers.begin();
  for (; it != headers.end(); ++it) {
    std::string key = it->first;

    // "x-"로 시작하는지 확인
    if (key.length() >= 2 and key.substr(0, 2) == "x-") {
      // "HTTP_" 접두사 추가
      key = "HTTP_" + key;

      // 모든 하이픈('-')을 언더스코어('_')로 변경
      std::replace(key.begin(), key.end(), '-', '_');

      // 대문자 변환
      for (std::string::size_type i = 0; i < key.length(); ++i) {
        key[i] = std::toupper(static_cast<unsigned char>(key[i]));
      }

      // 환경 변수 설정
      env[key] = it->second;
    }
  }

  return createEnvArray(env);
}

// map 형태의 환경변수를 char** 형태로 동적할당하여 반환
char** CgiBuilder::createEnvArray(
    std::map<std::string, std::string> const& env) {
  std::vector<char*> envArray;
  std::map<std::string, std::string>::const_iterator it;

  for (it = env.begin(); it != env.end(); ++it) {
    std::string envEntry = it->first + "=" + it->second;

    char* envEntryCStr = new char[envEntry.size() + 1];
    std::strcpy(envEntryCStr, envEntry.c_str());
    envEntryCStr[envEntry.size()] = '\0';

    envArray.push_back(envEntryCStr);
  }

  envArray.push_back(NULL);

  char** envp = new char*[envArray.size()];
  for (size_t i = 0; i < envArray.size(); ++i) {
    envp[i] = envArray[i];
  }

  return envp;
}

// 동적할당된 환경변수를 해제
void CgiBuilder::freeEnvArray(char** envp) {
  for (int i = 0; envp[i] != NULL; ++i) {
    delete[] envp[i];
  }

  delete[] envp;
}
