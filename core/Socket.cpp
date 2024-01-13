#include "Socket.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <stdexcept>

// 소켓을 생성
// - 시스템콜이 실패할 경우 예외 발생
int Socket::socket(void) {
  int fd;

  // IPv4, TCP
  if ((fd = ::socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket");
    throw std::runtime_error("[3200] Socket: socket - system call fail");
  }
  return fd;
}

// 소켓을 할당
// - 시스템콜이 실패할 경우 예외 발생
// - host에는 IP 주소만 가능 (localhost, www.example.com 등 도메인은 불가)
void Socket::bind(int fd, std::string host, int port) {
  struct sockaddr_in address;

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(host.c_str());
  address.sin_port = htons(port);

  if (::bind(fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    perror("bind");
    throw std::runtime_error("[3201] Socket: bind - system call fail");
  }
}

// 소켓을 연결 요청 대기 상태로 만듦
// - 시스템콜이 실패할 경우 예외 발생
// - queueSize: 연결 요청 대기열 크기
void Socket::listen(int fd, int queueSize) {
  if (::listen(fd, queueSize) < 0) {
    perror("listen");
    throw std::runtime_error("[3202] Socket: listen - system call fail");
  }
}

// 소켓에 들어온 연결 요청을 수락
// - 시스템콜이 실패할 경우 예외 발생
int Socket::accept(int listenFd) {
  struct sockaddr_in address;
  socklen_t addressLen = sizeof(address);

  // address를 통해 클라이언트의 IP 정보 등을 알 수 있음
  int clientSocket =
      ::accept(listenFd, (struct sockaddr*)&address, (socklen_t*)&addressLen);
  if (clientSocket < 0) {
    perror("accept");
    throw std::runtime_error("[3203] Socket: accept - system call fail");
  }
  return clientSocket;
}

// 소켓 설정
// - 시스템콜이 실패할 경우 예외 발생
void Socket::setsockopt(int fd) {
  int opt = 1;

  // 소켓 주소 재사용 허용
  if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt");
    throw std::runtime_error("[3204] Socket: setsockopt - system call fail");
  }
}

// 해당 fd를 non-blocking으로 만듦
// - 시스템콜이 실패할 경우 예외 발생
void Socket::setNonBlocking(int fd) {
  if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
    perror("fcntl");
    throw std::runtime_error(
        "[3205] Socket: setNonBlocking - system call fail");
  }
}
