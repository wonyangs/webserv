#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <map>
#include <set>
#include <string>

#include "Location.hpp"

// config 파일의 Server 블럭
// - request에 맞는 location 블럭을 리턴한다.
class Server {
 private:
  std::string _hostIp;
  int _port;

  std::set<std::string> _serverNames;
  std::map<std::string, Location> _locationBlocks;

 public:
  Server(std::string hostIp, int port);
  Server(Server const& server);
  ~Server(void);

  Server& operator=(Server const& server);

  // setter
  void addServerName(const std::string& serverName);
  void addLocationBlock(const Location& locationBlock);

  // method
  const Location& getMatchedLocationBlock(const std::string& uri);
  bool hasDefaultLocationBlock(void);

 private:
  typedef std::map<std::string, Location> LocationMap;

  Server(void);
};

#endif