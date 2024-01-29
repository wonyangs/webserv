#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

#include <cstdio>
#include <string>

// 소켓 api를 제공하는 정적 클래스
class Socket {
 public:
  static int socket(void);
  static void bind(int fd, std::string host, int port);
  static void listen(int fd, int queueSize);
  static int accept(int listenFd);

  static void setsockopt(int fd);
  static void setNonBlocking(int fd);

 private:
  Socket(void);
  Socket(Socket const& socket);
  ~Socket(void);
  Socket& operator=(Socket const& socket);
};

#endif