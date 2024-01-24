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
  }
  return *this;
}

/**
 * Public method
 */

// 첫 번째 호출 시 CGI 생성
// - 이후 호출부터 이벤트에 따라 읽기 또는 쓰기 진행
std::vector<int> const CgiBuilder::build(Event::EventType type) {
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
      throw std::runtime_error("[] CgiBuilder: build - unknown event");
      break
  }
  return std::vector<int>();
}

void CgiBuilder::close(void) {
  if (_pid != -1 and (_readPipeFd != -1 or _writePipeFd != -1)) {
    kill(pid, SIGKILL);
    _pid = -1;
  }
  if (_readPipeFd != -1) {
    Kqueue::removeReadEvent(_readPipeFd);
    ::close(_readPipeFd);
    _readPipeFd = -1
  }
  if (_writePipeFd != -1) {
    Kqueue::removeWriteEvent(_writePipeFd);
    ::close(_writePipeFd);
    _writePipeFd = -1
  }
}

/**
 * Private method - CGI init
 */

// CGI 초기화
// - 시스템콜 호출에 실패한 경우 예외 발생
std::vector<int> const CgiBuilder::forkCgi(void) {
  int p_to_c[2];
  int c_to_p[2];

  if (pipe(p_to_c) < 0 or pipe(c_to_p) < 0) {
    throw std::runtime_error("[] CgiBuilder: forkCgi - pipe fail");
  }
  if ((_pid = fork()) < 0) {
    throw std::runtime_error("[] CgiBuilder: forkCgi - fork fail");
  }

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

  if (access(cgiPath, F_OK) < 0) {
    throw std::runtime_error("[] CgiBuilder: childProcess - cgi not found");
  }
  if (access(cgiPath, X_OK) < 0) {
    throw std::runtime_error("[] CgiBuilder: childProcess - execve failed");
  }

  // cgi에 인자 및 환경변수 전송
  if (execve(cgiPath.c_str(), NULL, envp) < 0) {
    freeEnvArray(envp);
    throw std::runtime_error("[] CgiBuilder: childProcess - execve failed");
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
    throw std::runtime_error("[] CgiBuilder: handleReadEvent - read failed");
  }

  for (ssize_t i = 0; i < bytesRead; i++) {
    _storageBuffer.push_back(buffer[i]);
  }

  if (bytesRead == 0) {  // eof에 도달
    Kqueue::removeReadEvent(_readPipeFd);
    _readPipeFd = -1;

    std::string body(_storageBuffer.begin(), _storageBuffer.end());
    buildResponseContent(body);
    _isDone = true;
  }
}

// 파이프에 CGI에게 보낼 내용을 쓰기
void CgiBuilder::handleWriteEvent(void) {
  std::string const& body = getRequest().getBody();

  // cgi 표준 입력에 body 전송
  ssize_t bytesWrite = write(_writePipeFd, body.c_str() + _writeIndex,
                             body.size() - _writeIndex);
  _writeIndex += bytesWrite;

  if (bytesWrite < 0) {
    throw std::runtime_error("[] CgiBuilder: handleWriteEvent - write failed");
  }

  if (bytesWrite == 0) {  // 모든 내용 전송 완료
    Kqueue::removeWriteEvent(_writePipeFd);
    _writePipeFd = -1;
  }
}

/**
 * Private method - make response
 */

void CgiBuilder::buildResponseContent(std::string const& body) {}

/**
 * Private method - env
 */

// cgi에 필요한 환경변수를 설정하여 반환
char** CgiBuilder::makeEnv(void) {
  Request const& request = getRequest();
  Location const& location = request.getLocation();
  std::map<std::string, std::string> env;

  env["ROOT_PATH"] = location.getRootPath();
  env["CGI_PATH"] = location.getRootPath();
  env["UPLOAD_PATH"] = location.getUploadDirPath();

  if (request.isHeaderFieldNameExists("Authorization")) {
    env["AUTH_TYPE"] = request.getHeaderFieldValues("Authorization");
  }
  if (request.isHeaderFieldNameExists("Content-Length")) {
    env["CONTENT_LENGTH"] = request.getBody().size();
  }
  if (request.isHeaderFieldNameExists("Content-Type")) {
    env["CONTENT_TYPE"] = request.getHeaderFieldValues("Content-Type");
  }

  env["GATEWAY_INTERFACE"] = "CGI/1.1";

  if (request.isHeaderFieldNameExists("Accept")) {
    env["HTTP_ACCEPT"] = request.getHeaderFieldValues("Accept");
  }
  if (request.isHeaderFieldNameExists("Accept-Charset")) {
    env["HTTP_ACCEPT_CHARSET"] = request.getHeaderFieldValues("Accept-Charset");
  }
  if (request.isHeaderFieldNameExists("Accept-Encoding")) {
    env["HTTP_ACCEPT_ENCODING"] =
        request.getHeaderFieldValues("Accept-Encoding");
  }
  if (request.isHeaderFieldNameExists("Accept-Language")) {
    env["HTTP_ACCEPT_LANGUAGE"] =
        request.getHeaderFieldValues("Accept-Language");
  }
  if (request.isHeaderFieldNameExists("Forwarded")) {
    env["HTTP_FORWARDED"] = request.getHeaderFieldValues("Forwarded");
  }
  if (request.isHeaderFieldNameExists("Host")) {
    env["HTTP_HOST"] = request.getHeaderFieldValues("Host");
  }
  if (request.isHeaderFieldNameExists("Proxy-Authorization")) {
    env["HTTP_PROXY_AUTHORIZATION"] =
        request.getHeaderFieldValues("Proxy-Authorization");
  }
  if (request.isHeaderFieldNameExists("User-Agent")) {
    env["HTTP_USER_AGENT"] = request.getHeaderFieldValues("User-Agent");
  }

  env["PATH_INFO"] = "/";                               // ????
  env["QUERY_STRING"] = urlDecode(request.getQuery());  // Todo: decoding
  // env["REMOTE_ADDR"] = Todo: 클라이언트 IP 주소
  // env["REMOTE_HOST"] = Todo: 클라이언트 IP 주소
  // env["REMOTE_USER"] = "null"
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
      // case HTTP_HEAD:
      //   env["REQUEST_METHOD"] = "HEAD";
      //   break;
  }
  // env["SCRIPT_NAME"] -> ?
  // env["SERVER_NAME"] = server.getServerName(); // Todo: 구현
  // env["SERVER_PORT"] = server.getPort();
  env["SERVER_PROTOCOL"] = "HTTP/1.1";
  env["SERVER_SOFTWARE"] = "webserv/1.0";

  if (request.isHeaderFieldNameExists("Cookie")) {
    env["HTTP_COOKIE"] = request.getHeaderFieldValues("Cookie");
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

std::string const CgiBuilder::urlDecode(std::string const& encoded) {
  std::string decoded;
  std::istringstream iss(encoded);
  char ch;

  while (iss.get(ch)) {
    if (ch == '%') {
      int value;
      iss >> std::hex >> value;  // 16진수로 변환
      decoded += static_cast<char>(value);
      iss.ignore();  // '%' 뒤의 두 문자를 건너뜁니다.
    } else if (ch == '+') {
      decoded += ' ';
    } else {
      decoded += ch;
    }
  }
  return decoded;
}
