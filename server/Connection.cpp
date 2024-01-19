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
  setStatus(ON_RECV);

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
  setStatus(ON_RECV);

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

      Request const& request = _requestParser.getRequest();
      request.print();

      // 응답 만들기
      // WRITE 등록
      _requestParser.clear();
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

// 응답 보내기
// - 임시 메서드
void Connection::send(void) {
  setStatus(ON_SEND);

  char const* response =
      "HTTP/1.1 200 OK\nContent-Length: 13\nContent-Type: "
      "text/plain\nConnection: keep-alive\n\nHello, "
      "world!\n\n";
  ssize_t bytesSent = write(_fd, response, strlen(response));

  if (bytesSent < 0) {
    throw std::runtime_error("[4001] Connection: send - fail to write socket");
  }

  std::cout << "[ Server: response sent ]\n"
            << "-------------\n"
            << response << "\n-------------" << std::endl;
  updateLastCallTime();

  setStatus(ON_WAIT);
}

// 에러 응답 보내기
// - 임시 메서드
void Connection::sendErrorPage(int code) {
  setStatus(ON_SEND);

  // 상태 코드에 해당하는 메시지 찾기
  std::map<int, std::string>::const_iterator it =
      Config::statusMessages.find(code);
  if (it == Config::statusMessages.end()) {
    // 정의되지 않은 코드일 경우 기본 메시지 설정
    code = 500;
    it = Config::statusMessages.find(500);
  }

  std::string codeString = std::to_string(code);

  // HTTP 응답 생성
  std::string const& body =
      Config::defaultErrorPageBody(codeString, it->second);

  std::string response =
      "HTTP/1.1 " + codeString + " " + it->second + "\n" +
      "Content-Length: " + std::to_string(body.size()) +
      "\nContent-Type: text/html\nConnection: keep-alive\n\n" + body;
  ssize_t bytesSent = write(_fd, response.c_str(), response.length());

  if (bytesSent < 0) {
    throw std::runtime_error(
        "[4002] Connection: sendErrorPage - fail to write socket");
  }

  std::cout << "[ Server: response sent ]\n"
            << "-------------\n"
            << response << "\n-------------" << std::endl;

  updateLastCallTime();

  setStatus(ON_WAIT);
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
