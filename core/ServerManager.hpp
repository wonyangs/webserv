#ifndef __SERVERMANAGER_HPP__
#define __SERVERMANAGER_HPP__

#include <map>

#include "../config/Server.hpp"
#include "Connection.hpp"

// 가상 서버 클래스
class ServerManager {
 private:
  Server _config;
  std::map<int, Connection> _connections;

 public:
  ServerManager(Server config);
  ServerManager(ServerManager const& manager);
  ~ServerManager(void);

  ServerManager& operator=(ServerManager const& manager);

  void addConnection(int fd);
  void removeConnection(int fd);
  bool hasFd(int fd) const;
  void timeout(void);

 private:
  static long const CONNECTION_LIMIT_TIME = 30;
  typedef std::map<int, Connection> ConnectionMap;

  ServerManager(void);
};

#endif