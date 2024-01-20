#ifndef __SERVERMANAGER_HPP__
#define __SERVERMANAGER_HPP__

#include <map>
#include <string>
#include <vector>

#include "../config/Server.hpp"
#include "../core/Event.hpp"
#include "Connection.hpp"

class Connection;

// 가상 서버 클래스
class ServerManager {
 private:
  int _serverFd;
  std::string _hostIp;
  int _port;
  std::vector<Server> _configs;
  std::map<int, Connection> _connections;

 public:
  ServerManager(std::string hostIp, int port, std::vector<Server> configs);
  ServerManager(ServerManager const& manager);
  ~ServerManager(void);

  ServerManager& operator=(ServerManager const& manager);

  void init(void);
  void run(void);
  void clear(void);

  void handleEvent(Event event);
  void manageTimeoutConnections(void);

  bool canHandleEvent(Event event) const;
  int getServerFd(void) const;

  Location const& getDefaultLocation(void);
  Location const& getLocation(std::string const& path, std::string const& host);

 private:
  static long const CONNECTION_LIMIT_TIME = 30;
  typedef std::map<int, Connection> ConnectionMap;

  void addConnection(int fd);
  void removeConnection(int fd);
  bool hasConnectionFd(int fd) const;

  void handleServerEvent(void);
  void handleReadEvent(int eventFd);
  void handleWriteEvent(int eventFd);

  ServerManager(void);
};

#endif