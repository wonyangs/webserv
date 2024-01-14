#include "Connection.hpp"

#include <unistd.h>

#include <cstring>
#include <iostream>

// Constructor & Destructor

Connection::Connection(int fd)
    : _fd(fd), _lastCallTime(std::time(0)), _requestParser(_request) {}

Connection::Connection(Connection const& connection)
    : _requestParser(_request) {
  *this = connection;
}

Connection::~Connection(void) {}

// Operator Overloading

Connection& Connection::operator=(Connection const& connection) {
  if (this != &connection) {
    _fd = connection._fd;
    _lastCallTime = connection._lastCallTime;
    _requestParser = connection._requestParser;
    _request = connection._request;
  }
  return *this;
}

// Public method

// 요청 읽기
// - 임시 메서드
void Connection::receive(void) {
  u_int8_t buffer[BUFFER_SIZE];

  memset(buffer, 0, BUFFER_SIZE);
  ssize_t bytesRead = ::read(_fd, buffer, sizeof(buffer) - 1);

  if (bytesRead < 0) {
    perror("read");
    ::close(_fd);
    return;
  } else if (bytesRead == 0) {
    // 클라이언트가 연결을 종료했음
    ::close(_fd);
    std::cout << "Client: connection closed" << std::endl;
    return;
  } else {
    try {
      _requestParser.parse(buffer, bytesRead);
    } catch (std::exception& e) {
      // TODO: StatusException의 경우 해당하는 에러 코드 전송 및 커넥션 끊기
      std::cerr << "Exception thrown: " << e.what() << std::endl;
      _requestString.clear();
      _requestParser.clear();
      _request.clear();
    }
    std::string str(reinterpret_cast<char*>(buffer), bytesRead);
    _requestString.append(str);
  }

  std::cout << "   Buffer: " << buffer << std::endl;
  this->_request.print();

  if (_requestParser.getParsingStatus() == DONE) {
    std::cout << "[ Server: received request ]\n"
              << "-------------\n"
              << _requestString << "\n-------------" << std::endl;
    send();
    _requestString.clear();
    _requestParser.clear();
    _request.clear();
  }
  updateLastCallTime();
}

// 응답 보내기
// - 임시 메서드
void Connection::send(void) {
  char const* response =
      "HTTP/1.1 200 OK\nContent-Length: 13\nContent-Type: "
      "text/plain\nConnection: keep-alive\n\nHello, "
      "world!\n\n";
  ssize_t bytesSent = write(_fd, response, strlen(response));

  if (bytesSent < 0) {
    throw std::runtime_error("Failed to write to socket");
  }

  std::cout << "[ Server: response sent ]\n"
            << "-------------\n"
            << response << "\n-------------" << std::endl;
  updateLastCallTime();
}

// 커넥션 닫기
void Connection::close(void) { ::close(_fd); }

// 관리 중인 fd 반환
int Connection::getFd(void) const { return _fd; }

// 마지막으로 호출된 이후 경과된 시간 반환
// - 반환된 값은 초 단위
long Connection::getElapsedTime(void) const {
  return std::time(0) - _lastCallTime;
}

// Private method

// 마지막으로 호출된 시간 업데이트
// - timeout 관리를 위해 커넥션이 호출되면 반드시 사용
void Connection::updateLastCallTime(void) { _lastCallTime = std::time(0); }
