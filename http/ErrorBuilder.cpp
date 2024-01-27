#include "ErrorBuilder.hpp"

/**
 * Constructor & Destructor
 */

ErrorBuilder::ErrorBuilder(void)
    : AResponseBuilder(ERROR, Request()),
      _statusCode(500),
      _recursiveFlag(true),
      _fileFd(-1),
      _fileSize(-1),
      _readIndex(0) {}

ErrorBuilder::ErrorBuilder(Request const& request, int statusCode)
    : AResponseBuilder(ERROR, request),
      _statusCode(statusCode),
      _recursiveFlag(false),
      _fileFd(-1),
      _fileSize(-1),
      _readIndex(0) {}

ErrorBuilder::ErrorBuilder(ErrorBuilder const& builder)
    : AResponseBuilder(builder) {
  _statusCode = builder._statusCode;
  _recursiveFlag = builder._recursiveFlag;
  _fileFd = builder._fileFd;
  _fileSize = builder._fileSize;
  _readIndex = builder._readIndex;
  _storageBuffer = builder._storageBuffer;
}

// TODO: 소멸할 때 관련된 fd를 clear하도록 구현
ErrorBuilder::~ErrorBuilder(void) { close(); }

/**
 * Operator Overloading
 */

ErrorBuilder& ErrorBuilder::operator=(ErrorBuilder const& builder) {
  if (this != &builder) {
    _response = builder._response;
    _isDone = builder._isDone;
    setRequest(builder.getRequest());
    setType(builder.getType());
    _statusCode = builder._statusCode;
    _recursiveFlag = builder._recursiveFlag;
    _fileFd = builder._fileFd;
    _fileSize = builder._fileSize;
    _readIndex = builder._readIndex;
    _storageBuffer = builder._storageBuffer;
  }
  return *this;
}

/**
 * Public method
 */

bool ErrorBuilder::isConnectionClose(void) const {
  if (_statusCode / 100 == 5 or _statusCode == 400 or _statusCode == 413)
    return true;

  Request const& request = getRequest();
  return request.isConnectionClose();
}

// 새로운 이벤트가 등록되었다면 해당 fd 반환, 아니라면 -1 반환
std::vector<int> const ErrorBuilder::build(Event::EventType type) {
  (void)type;
  Request const& request = getRequest();

  if (_recursiveFlag == false and request.getLocationFlag()) {
    Location const& location = request.getLocation();
    if (location.hasErrorPage(_statusCode)) {
      int fd = readStatusCodeFile(location);
      return std::vector<int>(1, fd);
    }
  }
  generateDefaultPage();
  return std::vector<int>();
}

void ErrorBuilder::close(void) {
  if (_fileFd != -1) {
    Kqueue::removeReadEvent(_fileFd);
    ::close(_fileFd);
    _fileFd = -1;
  }
}

/**
 * Private method
 */

// 첫 번째 호출 시 파일을 열고, 이후 호출 시 파일을 계속 읽음
// - location에 상태코드에 해당하는 에러 페이지가 있다고 가정
// - read에 실패한 경우 예외 발생
int ErrorBuilder::readStatusCodeFile(Location const& location) {
  // 첫 번째 호출 시 파일을 열음
  if (_fileFd == -1) {
    openStatusCodeFile(location);
    return _fileFd;
  }

  // 파일 읽기
  octet_t buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  ssize_t bytesRead = read(_fileFd, buffer, sizeof(buffer));
  _readIndex += bytesRead;

  if (bytesRead < 0) {
    throw std::runtime_error(
        "[5100] ErrorBuilder: readStatusCodeFile - read failed");
  }

  for (ssize_t i = 0; i < bytesRead; i++) {
    _storageBuffer.push_back(buffer[i]);
  }

  if (bytesRead == 0 or _fileSize == _readIndex) {  // 파일의 끝까지 읽음
    std::string body(_storageBuffer.begin(), _storageBuffer.end());
    buildResponseContent(body);
    _isDone = true;
  }
  return -1;
}

// 상태코드에 해당하는 에러 파일 열기
// - 파일이 없거나 open에 실패한 경우 예외 발생
void ErrorBuilder::openStatusCodeFile(Location const& location) {
  // 파일 경로 제작
  std::string const& root = location.getRootPath();
  std::string const& path = location.getErrorPagePath(_statusCode);
  std::string const& fullPath = root + path;

  // 파일 접근 권한 확인
  if (access(fullPath.c_str(), F_OK | R_OK) == -1) {
    throw StatusException(
        HTTP_INTERNAL_SERVER_ERROR,
        "[5101] ErrorBuilder: openStatusCodeFile - file permissions denied");
  }

  // 파일 크기 측정
  struct stat statbuf;

  if (stat(fullPath.c_str(), &statbuf) == -1) {
    throw std::runtime_error(
        "[5102] ErrorBuilder: openStatusCodeFile - stat failed");
  }

  _fileSize = statbuf.st_size;

  // 파일 열기
  _fileFd = open(fullPath.c_str(), O_RDONLY);
  if (_fileFd < 0) {
    throw std::runtime_error(
        "[5103] ErrorBuilder: openStatusCodeFile - open failed");
  }

  Socket::setNonBlocking(_fileFd);
  Kqueue::addReadEvent(_fileFd);
}

// 기본 에러 페이지 생성
void ErrorBuilder::generateDefaultPage(void) {
  std::string const& body = Config::defaultErrorPageBody(_statusCode);

  buildResponseContent(body);

  _isDone = true;
}

// body 정보를 받아 response 제작
void ErrorBuilder::buildResponseContent(std::string const& body) {
  _response.setHttpVersion("HTTP/1.1");
  _response.setStatusCode(_statusCode);

  _response.addHeader("Content-Type", "text/html");
  _response.addHeader("Content-Length", Util::itos(body.size()));

  isConnectionClose() ? _response.addHeader("Connection", "close")
                      : _response.addHeader("Connection", "keep-alive");

  if (_statusCode == 405 and getRequest().getLocationFlag() == true)
    _response.addHeader("Allow", makeAllowHeaderValue());

  _response.appendBody(body);

  _response.makeResponseContent();
}

std::string ErrorBuilder::makeAllowHeaderValue(void) {
  Location const& location = getRequest().getLocation();

  std::stringstream ss;
  bool isFirst = true;

  if (location.isAllowMethod(HTTP_GET)) {
    ss << "GET";
    isFirst = false;
  }
  if (location.isAllowMethod(HTTP_POST)) {
    ss << (isFirst ? "POST" : ", POST");
    isFirst = false;
  }
  if (location.isAllowMethod(HTTP_DELETE)) {
    ss << (isFirst ? "DELETE" : ", DELETE");
    isFirst = false;
  }

  return ss.str();
}
