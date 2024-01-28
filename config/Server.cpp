#include "Server.hpp"

#include <stdexcept>

// Constructor & Destructor

Server::Server(void) {}

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

// Public method - getter

std::string const& Server::getHostIp(void) const { return _hostIp; }

int Server::getPort(void) const { return _port; }

// Public Method - setter

void Server::setHostIp(std::string const& hostIp) { _hostIp = hostIp; }

void Server::setPort(int port) { _port = port; }

// - server name을 추가
// - 이미 있는 server name이 들어올 경우 예외 발생
void Server::addServerName(std::string const& serverName) {
  if (_serverNames.find(serverName) != _serverNames.end()) {
    throw std::runtime_error(
        "[1000] Server: addServerName - duplicate server_name");
  }
  _serverNames.insert(serverName);
}

// - Server 블럭에 포함된 location 블럭 추가
// - 같은 uri를 가진 location 블럭이 들어온 경우 예외 발생
void Server::addLocationBlock(Location const& locationBlock) {
  std::string uri = locationBlock.getUri();

  if (_locationBlocks.find(uri) != _locationBlocks.end()) {
    throw std::runtime_error("[1001] Server: addLocationBlock - duplicate uri");
  }

  _locationBlocks.insert(std::make_pair(uri, locationBlock));
}

// Public Method - method

// - uri와 prefix가 매칭되는 Location 객체를 반환
// - uri가 /인 Location 객체는 반드시 존재한다고 가정
// - uri 매칭에 실패하는 경우는 없음
Location const& Server::getMatchedLocationBlock(std::string const& uri) {
  std::string bestMatch;
  size_t longestMatchLength = 0;

  LocationMap::const_iterator it = _locationBlocks.begin();
  while (it != _locationBlocks.end()) {
    std::string const& key = it->first;
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

// - Server 블록에 uri가 /인 Location 블록이 존재하는지 여부 반환
// - Server 블록에는 uri가 /인 Location 블록이 반드시 존재해야함
bool Server::hasDefaultLocationBlock(void) {
  if (_locationBlocks.find("/") == _locationBlocks.end()) {
    return false;
  }
  return true;
}

// host명이 Server 블록의 ServerName에 존재하는지 여부 반환
bool Server::hasServerName(std::string const& host) {
  return (_serverNames.find(host) != _serverNames.end());
}
