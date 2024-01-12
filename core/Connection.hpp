#ifndef __CONNECTION_HPP__
#define __CONNECTION_HPP__

#include <string>
#include <ctime>

// 클라이언트 연결을 관리하는 클래스
class Connection {
 private:
  int _fd;
  std::time_t _lastCallTime;
  std::string _request;

 public:
  Connection(int fd);
  Connection(Connection const& connection);
  ~Connection(void);

  Connection& operator=(Connection const& connection);

  void receive(void);
  void send(void);

  void close(void);

  long getElapsedTime(void) const;

 private:
  static const int BUFFER_SIZE = 1024;

  void updateLastCallTime(void);

  Connection(void);
};

#endif