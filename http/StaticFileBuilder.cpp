#include "StaticFileBuilder.hpp"

/**
 * Constructor & Destructor
 */

StaticFileBuilder::StaticFileBuilder(Request const& request)
    : AResponseBuilder(STATIC, request),
      _fileFd(-1),
      _fileSize(-1),
      _readIndex(0) {}

StaticFileBuilder::StaticFileBuilder(StaticFileBuilder const& builder)
    : AResponseBuilder(builder) {
  _fileFd = builder._fileFd;
  _fileSize = builder._fileSize;
  _readIndex = builder._readIndex;
  _storageBuffer = builder._storageBuffer;
}

StaticFileBuilder::~StaticFileBuilder(void) { close(); }

/**
 * Operator Overloading
 */

StaticFileBuilder& StaticFileBuilder::operator=(
    StaticFileBuilder const& builder) {
  if (this != &builder) {
    _response = builder._response;
    _isDone = builder._isDone;
    setRequest(builder.getRequest());
    setType(builder.getType());
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

int StaticFileBuilder::build(void) {
  // 첫 번째 호출 시 파일을 열음
  if (_fileFd == -1) {
    openStaticFile();
    return _fileFd;
  }

  // 두 번째 호출부터는 파일을 읽음
  readStaticFile();
  return -1;
}

void StaticFileBuilder::close(void) {
  if (_fileFd != -1) {
    Kqueue::removeReadEvent(_fileFd);
    ::close(_fileFd);
    _fileFd = -1;
  }
}

/**
 * Private method
 */

// 파일 존재 여부를 확인 후 파일 열기
void StaticFileBuilder::openStaticFile(void) {
  // 파일 경로 제작
  Request const& request = getRequest();
  Location const& location = request.getLocation();

  std::string const& root = location.getRootPath();
  std::string const& path = request.getPath();
  std::string const& fullPath = root + path;

  // 파일 존재 여부 확인
  if (access(fullPath.c_str(), F_OK) == -1) {
    throw StatusException(
        HTTP_NOT_FOUND,
        "[] StaticFileBuilder: openStaticFile - can't find file");
  }

  // 파일 접근 권한 확인
  if (access(fullPath.c_str(), R_OK) == -1) {
    throw StatusException(
        HTTP_FORBIDDEN,
        "[] StaticFileBuilder: openStaticFile - file permissions denied");
  }

  // 파일 크기 측정
  struct stat statbuf;

  if (stat(fullPath.c_str(), &statbuf) == -1) {
    throw std::runtime_error(
        "[] StaticFileBuilder: openStaticFile - stat failed");
  }

  _fileSize = statbuf.st_size;

  // 파일 열기
  _fileFd = open(fullPath.c_str(), O_RDONLY);
  if (_fileFd < 0) {
    throw std::runtime_error(
        "[] StaticFileBuilder: openStaticFile - open failed");
  }

  Socket::setNonBlocking(_fileFd);
  Kqueue::addReadEvent(_fileFd);
}

// 파일 읽기
void StaticFileBuilder::readStaticFile(void) {
  u_int8_t buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  ssize_t bytesRead = read(_fileFd, buffer, sizeof(buffer));
  _readIndex += bytesRead;

  if (bytesRead < 0) {
    throw std::runtime_error(
        "[] ErrorBuilder: readStatusCodeFile - read failed");
  }

  for (ssize_t i = 0; i < bytesRead; i++) {
    _storageBuffer.push_back(buffer[i]);
  }

  if (bytesRead == 0 or _fileSize == _readIndex) {  // 파일의 끝까지 읽음
    std::string body(_storageBuffer.begin(), _storageBuffer.end());
    buildResponseContent(body);
    _isDone = true;
  }
}

// body 정보를 받아 response 제작
void StaticFileBuilder::buildResponseContent(std::string const& body) {
  // Todo: Connection은 request Header 정보 보고 변경되어야 함

  _response.setHttpVersion("HTTP/1.1");
  _response.setStatusCode(200);

  _response.addHeader("Content-Type", "text/html");
  _response.addHeader("Content-Length", Util::itos(body.size()));
  _response.addHeader("Connection", "keep-alive");

  _response.appendBody(body);

  _response.makeResponseContent();
}