#ifndef __SERVERMANAGER_HPP__
#define __SERVERMANAGER_HPP__

#include <map>
#include <string>
#include <vector>

#include "../config/Server.hpp"
#include "../core/Event.hpp"
#include "Connection.hpp"

// 가상 서버 클래스
class ServerManager {
 private:
  int _fd;
  std::string _hostIp;
  int _port;
  std::vector<Server> _configs;
  std::map<int, Connection> _connections;

 public:
  ServerManager(std::string hostIp, int port, std::vector<Server> configs);
  ServerManager(ServerManager const& manager);
  ~ServerManager(void);

  ServerManager& operator=(ServerManager const& manager);

  void runServer(void);

  int getServerFd(void) const;

  void addConnection(int fd);
  void removeConnection(int fd);
  void handleConnection(Event event);

  bool hasFd(int fd) const;
  void manageTimeoutConnections(void);

 private:
  static long const CONNECTION_LIMIT_TIME = 30;
  typedef std::map<int, Connection> ConnectionMap;

  ServerManager(void);
};

#endif