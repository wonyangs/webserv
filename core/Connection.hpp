#ifndef __CONNECTION_HPP__
#define __CONNECTION_HPP__

#include <ctime>
#include <string>

#include "http/Request.hpp"
#include "http/RequestParser.hpp"

// 클라이언트 연결을 관리하는 클래스
// - 임시 객체 (구현 예정)
class Connection {
 private:
  int _fd;
  std::time_t _lastCallTime;
  std::string _requestString;

  Request _request;
  RequestParser _requestParser;

 public:
  Connection(int fd);
  Connection(Connection const& connection);
  ~Connection(void);

  Connection& operator=(Connection const& connection);

  void receive(void);
  void send(void);

  void close(void);

  int getFd(void) const;
  long getElapsedTime(void) const;

 private:
  static int const BUFFER_SIZE = 1024;

  void updateLastCallTime(void);

  Connection(void);
};

#endif