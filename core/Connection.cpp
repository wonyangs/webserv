#include "Connection.hpp"

#include <unistd.h>

#include <cstring>
#include <iostream>

// Constructor & Destructor

Connection::Connection(int fd)
    : _fd(fd), _lastCallTime(std::time(0)), _requestParser() {}

Connection::Connection(Connection const& connection) { *this = connection; }

Connection::~Connection(void) {}

// Operator Overloading

Connection& Connection::operator=(Connection const& connection) {
  if (this != &connection) {
    _fd = connection._fd;
    _lastCallTime = connection._lastCallTime;
    _requestParser = connection._requestParser;
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
      _requestString.clear();
      _requestParser.clear();
      throw;
    }
    std::string str(reinterpret_cast<char*>(buffer), bytesRead);
    _requestString.append(str);
  }

  if (_requestParser.getParsingStatus() == DONE) {
    Request const& request = _requestParser.getRequest();
    request.print();

    std::cout << "[ Server: received request ]\n"
              << "-------------\n"
              << _requestString << "\n-------------" << std::endl;
    send();
    _requestString.clear();
    _requestParser.clear();
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

// 에러 응답 보내기
// - 임시 메서드
void Connection::sendErrorPage(int code) {
  // 상태 코드와 상태 메시지를 매핑
  std::map<int, std::string> statusMessages;
  statusMessages[200] = "OK";
  statusMessages[400] = "Bad Request";
  statusMessages[404] = "Not Found";
  statusMessages[405] = "Http Not Allowed";
  statusMessages[500] = "Internal Server Error";

  // 상태 코드에 해당하는 메시지 찾기
  std::map<int, std::string>::iterator it = statusMessages.find(code);
  if (it == statusMessages.end()) {
    // 정의되지 않은 코드일 경우 기본 메시지 설정
    code = 500;
    it = statusMessages.find(500);
  }

  // HTTP 응답 생성
  std::string response =
      "HTTP/1.1 " + std::to_string(code) + " " + it->second + "\n" +
      "Content-Length: 13\n"  // 내용 길이는 실제 내용에 맞게 조정 필요
      + "Content-Type: text/plain\n" + "Connection: keep-alive\n\n" +
      "Hello, world!\n\n";

  ssize_t bytesSent = write(_fd, response.c_str(), response.length());

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
