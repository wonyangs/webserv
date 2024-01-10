#include "Server.hpp"

#include <stdexcept>

// Constructor & Destructor

Server::Server(std::string hostIp, int port)
    : _hostIp(hostIp),
      _port(port){
          // TODO: invalid한 포트 번호 검증 추가
      };

Server::Server(Server const& server) { *this = server; }

Server::~Server(void) {}

// Operator Overloading

Server& Server::operator=(Server const& server) {
  if (this != &server) {
    this->_hostIp = server._hostIp;
    this->_port = server._port;

    this->_serverNames = server._serverNames;
    this->_locationBlocks = server._locationBlocks;
  }
  return *this;
}

// Public Method - setter

// - server name을 추가
// - 이미 있는 server name이 들어올 경우 예외 발생
void Server::addServerName(const std::string& serverName) {
  if (_serverNames.find(serverName) != _serverNames.end()) {
    throw std::runtime_error("Server: addServerName() - duplicate server name");
  }
  _serverNames.insert(serverName);
}

// - Server 블럭에 포함된 location 블럭 추가
// - 같은 uri를 가진 location 블럭이 들어온 경우 예외 발생
void Server::addLocationBlock(const Location& locationBlock) {
  std::string uri = locationBlock.getUri();

  if (_locationBlocks.find(uri) != _locationBlocks.end()) {
    throw std::runtime_error(
        "Server: addLocationBlock() - duplicate location block uri");
  }

  _locationBlocks.insert(std::make_pair(uri, locationBlock));
}

// Public Method - method

const Location& Server::getMatchedLocationBlock(const std::string& uri) {
  std::string bestMatch;
  size_t longestMatchLength = 0;

  LocationMap::const_iterator it = _locationBlocks.begin();
  while (it != _locationBlocks.end()) {
    const std::string& key = it->first;
    if (uri.compare(0, key.length(), key) == 0 and
        key.length() > longestMatchLength) {
      bestMatch = it->first;
      longestMatchLength = key.length();
    }
    ++it;
  }

  it = _locationBlocks.find(bestMatch);
  return it->second;
}
