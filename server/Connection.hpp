#ifndef __CONNECTION_HPP__
#define __CONNECTION_HPP__

#include <sys/stat.h>
#include <unistd.h>

#include <ctime>
#include <string>
#include <vector>

#include "../http/AResponseBuilder.hpp"
#include "../http/AutoindexBuilder.hpp"
#include "../http/CgiBuilder.hpp"
#include "../http/ErrorBuilder.hpp"
#include "../http/RedirectBuilder.hpp"
#include "../http/Request.hpp"
#include "../http/RequestParser.hpp"
#include "../http/StaticFileBuilder.hpp"
#include "../utils/Config.hpp"
#include "../utils/Util.hpp"
#include "ServerManager.hpp"

class ServerManager;

// 클라이언트 연결을 관리하는 클래스
class Connection {
 public:
  enum EStatus { ON_WAIT, ON_RECV, TO_SEND, ON_BUILD, ON_SEND, CLOSE };

 private:
  int _fd;
  std::time_t _lastCallTime;
  enum EStatus _status;

  RequestParser _requestParser;
  AResponseBuilder* _responseBuilder;
  std::vector<int> _builderFds;

 public:
  Connection(int fd, ServerManager& manager);
  Connection(Connection const& connection);
  ~Connection(void);

  Connection& operator=(Connection const& connection);

  void readSocket(void);
  void readStorage(void);
  bool isReadStorageRequired(void);

  void selectResponseBuilder(void);
  void buildResponse(Event::EventType eventType);
  void sendResponse(void);

  void resetResponseBuilder(int code);
  void resetResponseBuilder(void);

  void clear(void);
  void close(void);

  int getFd(void) const;
  EStatus getConnectionStatus(void) const;
  long getElapsedTime(void) const;

  bool isSameState(EStatus status);

 private:
  static int const BUFFER_SIZE = 1024;
  ServerManager& _manager;

  void parseRequest(octet_t const* buffer, ssize_t bytesRead);
  void setRequestParserLocation(Request const& request);
  void removeAllBuilderFd(void);

  void updateLastCallTime(void);
  void setStatus(EStatus status);

  Connection(void);
};

#endif