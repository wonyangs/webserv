#include "Connection.hpp"

#include <unistd.h>

#include <cstring>
#include <iostream>

#include "../config/Location.hpp"
#include "../core/Kqueue.hpp"
#include "../utils/Config.hpp"

/**
 * Constructor & Destructor
 */

Connection::Connection(int fd, ServerManager& manager)
    : _fd(fd),
      _lastCallTime(std::time(0)),
      _status(ON_WAIT),
      _requestParser(),
      _responseBuilder(ErrorBuilder()),
      _manager(manager) {}

Connection::Connection(Connection const& connection)
    : _manager(connection._manager) {
  *this = connection;
}

Connection::~Connection(void) {}

/**
 * Operator Overloading
 */

Connection& Connection::operator=(Connection const& connection) {
  if (this != &connection) {
    _fd = connection._fd;
    _lastCallTime = connection._lastCallTime;
    _status = connection._status;
    _requestParser = connection._requestParser;
    _manager = connection._manager;
    _responseBuilder = connection._responseBuilder;
  }
  return *this;
}

/**
 * Public method - request
 */

// 요청 읽기
// - 읽기에 실패한 경우 예외 발생
// - 클라이언트 연결 종료가 감지된 경우 status를 CLOSE로 바꿈
void Connection::readSocket(void) {
  if (_status == ON_WAIT) {
    _requestParser.clear();
    setStatus(ON_RECV);
  }

  u_int8_t buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  ssize_t bytesRead = read(_fd, buffer, sizeof(buffer) - 1);

  if (bytesRead < 0) {
    throw std::runtime_error("[4000] Connection: readSocket - read fail");
  } else if (bytesRead == 0) {  // 클라이언트가 연결을 종료했음
    setStatus(CLOSE);
    std::cout << "Client: connection closed" << std::endl;  // debug
    updateLastCallTime();
    return;
  }

  parseRequest(buffer, bytesRead);
  updateLastCallTime();
}

// storage에 있는 요청 읽기
// - 읽기에 실패한 경우 예외 발생
void Connection::readStorage(void) {
  if (_status == ON_WAIT) {
    _requestParser.clear();
    setStatus(ON_RECV);
  }

  u_int8_t tmp[1];

  parseRequest(tmp, 0);
  updateLastCallTime();
}

// RequestParser에서 요청 읽기
// - bytesRead를 0으로 하면 storage에 남아있는 내용을 파싱
void Connection::parseRequest(u_int8_t const* buffer, ssize_t bytesRead) {
  try {
    _requestParser.parse(buffer, bytesRead);

    // 헤더 읽기 완료
    if (_requestParser.getParsingStatus() == HEADER_FIELD_END) {
      // Location 블록 할당
      Request const& request = _requestParser.getRequest();
      setRequestParserLocation(request);
      // 다시 파싱
      _requestParser.parse(buffer, 0);
    }

    // 전체 요청 읽기 완료
    if (_requestParser.getParsingStatus() == DONE) {
      setStatus(TO_SEND);

      // debug
      Request const& request = _requestParser.getRequest();
      request.print();

      // _requestParser.clear();
    }

  } catch (std::exception& e) {
    // TODO: StatusException의 경우 해당하는 에러 코드 전송 및 커넥션 끊기
    _requestParser.clear();
    throw;
  }
}

// RequestParser의 storage가 남아있는지 여부 반환
// - storage가 남아있는 경우 readStorage 메서드를 호출해야 함
bool Connection::isReadStorageRequired() {
  return _requestParser.isStorageBufferNotEmpty();
}

/**
 * Public method - response
 */

// HTTP 요청 + Location 블록을 보고 분기
// - 적절한 ResponseBuilder 선택
void Connection::selectResponseBuilder(void) {
  std::cout << "selected!" << std::endl;
  _responseBuilder = ErrorBuilder(_requestParser.getRequest(), 200);

  _responseBuilder.build();
  setStatus(ON_BUILD);
}

// HTTP 응답 만들기
void Connection::build() {
  _responseBuilder.build();

  if (_responseBuilder.isDone()) {
    setStatus(ON_SEND);
    _responseBuilder.close();
  }
}

// 응답 보내기
// - 임시 메서드
void Connection::send(void) {
  Response const& response = _responseBuilder.getResponse();
  std::string const& responseContent = response.toString();

  ssize_t bytesSent =
      write(_fd, responseContent.c_str(), responseContent.length());

  if (bytesSent < 0) {
    throw std::runtime_error(
        "[4002] Connection: sendErrorPage - fail to write socket");
  }

  std::cout << "[ Server: response sent ]\n"
            << "-------------\n"
            << responseContent << "\n-------------" << std::endl;

  updateLastCallTime();

  // ERROR BUILDER or Request -> Connection: close
  Request const& request = _requestParser.getRequest();
  if (_responseBuilder.getType() == AResponseBuilder::ERROR or
      request.isHeaderFieldValueExists("connection", "Close")) {
    setStatus(CLOSE);
  } else {
    setStatus(ON_WAIT);
  }
}

/**
 * Public method - etc
 */

// 커넥션 닫기
void Connection::close(void) { ::close(_fd); }

// 관리 중인 fd 반환
int Connection::getFd(void) const { return _fd; }

// 현재 커넥션의 상태 반환
Connection::EStatus Connection::getConnectionStatus(void) const {
  return _status;
}

// 마지막으로 호출된 이후 경과된 시간 반환
// - 반환된 값은 초 단위
long Connection::getElapsedTime(void) const {
  return std::time(0) - _lastCallTime;
}

/**
 * Private method
 */

// path와 host 정보를 가지고 알맞은 location 블럭을 할당
void Connection::setRequestParserLocation(Request const& request) {
  std::string const& path = request.getPath();
  std::string const& host = request.getHeaderFieldValues("host").front();

  Location const& location = _manager.getLocation(path, host);
  _requestParser.setRequestLocation(location);
}

// 마지막으로 호출된 시간 업데이트
// - timeout 관리를 위해 커넥션이 호출되면 반드시 사용
void Connection::updateLastCallTime(void) { _lastCallTime = std::time(0); }

// Connection의 현재 상태 변경
void Connection::setStatus(EStatus status) { _status = status; }
