#ifndef __CONNECTION_HPP__
#define __CONNECTION_HPP__

#include <ctime>
#include <string>

#include "http/Request.hpp"
#include "http/RequestParser.hpp"

// 클라이언트 연결을 관리하는 클래스
// - 임시 객체 (구현 예정)
class Connection {
 public:
  enum EStatus { ON_WAIT, ON_RECV, TO_SEND, ON_SEND, CLOSE };

 private:
  int _fd;
  std::time_t _lastCallTime;
  enum EStatus _status;

  RequestParser _requestParser;

 public:
  Connection(int fd);
  Connection(Connection const& connection);
  ~Connection(void);

  Connection& operator=(Connection const& connection);

  void readSocket(void);
  void readStorage(void);
  bool isReadStorageRequired(void);

  void send(void);
  void sendErrorPage(int code);

  void close(void);

  int getFd(void) const;
  EStatus getConnectionStatus(void) const;
  long getElapsedTime(void) const;

 private:
  static int const BUFFER_SIZE = 1024;

  void parseRequest(u_int8_t* buffer, ssize_t bytesRead);

  void updateLastCallTime(void);
  void changeStatus(EStatus status);

  Connection(void);
};

#endif