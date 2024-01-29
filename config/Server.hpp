#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <cstring>
#include <map>
#include <set>
#include <sstream>
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

  bool _isIncludeRootBlock;

 public:
  Server(void);
  Server(std::string hostIp, int port);
  Server(Server const& server);
  ~Server(void);

  Server& operator=(Server const& server);

  std::string const& getHostIp(void) const;
  int getPort(void) const;

  void setHostIp(std::string const& hostIp);
  void setPort(std::string const& port);

  void addServerName(std::string const& serverName);
  void addLocationBlock(Location const& locationBlock);

  Location const& getMatchedLocationBlock(std::string const& uri);
  bool hasDefaultLocationBlock(void);
  bool hasServerName(std::string const& host);

  bool isRequiredValuesSet(void) const;

 private:
  typedef std::map<std::string, Location> LocationMap;

  bool isValidIpFormat(std::string const& ip);
  bool isValidPort(const std::string& port);
};

#endif