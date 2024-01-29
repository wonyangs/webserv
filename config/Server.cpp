#include "Server.hpp"

#include <stdexcept>

// Constructor & Destructor

Server::Server(void)
    : _port(-1),
      _isHostIpSet(false),
      _isPortSet(false),
      _isIncludeRootBlock(false) {}

Server::Server(std::string hostIp, int port)
    : _hostIp(hostIp),
      _port(port),
      _isHostIpSet(false),
      _isPortSet(false),
      _isIncludeRootBlock(false) {}

Server::Server(Server const& server) { *this = server; }

Server::~Server(void) {}

// Operator Overloading

Server& Server::operator=(Server const& server) {
  if (this != &server) {
    this->_hostIp = server._hostIp;
    this->_port = server._port;

    this->_serverNames = server._serverNames;
    this->_locationBlocks = server._locationBlocks;

    this->_isHostIpSet = server._isHostIpSet;
    this->_isPortSet = server._isPortSet;
    this->_isIncludeRootBlock = server._isIncludeRootBlock;
  }
  return *this;
}

// Public method - getter

std::string const& Server::getHostIp(void) const { return _hostIp; }

int Server::getPort(void) const { return _port; }

// Public Method - setter

// Host IP를 설정
// - IP가 설정되어 있는데 다시 호출한 경우 예외 발생
// - IP 형식이 잘못된 경우 예외 발생
void Server::setHostIp(std::string const& hostIp) {
  if (_isHostIpSet) {
    throw std::runtime_error("[1002] Server: setHostIp - host ip already set");
  }

  if (isValidIpFormat(hostIp) == false) {
    throw std::runtime_error("[1003] Server: setHostIp - invalid ip format");
  }

  _hostIp = hostIp;
  _isHostIpSet = true;
}

// Port를 설정
// - 이미 포트 번호가 설정되어 있는데 다시 호출한 경우 예외 발생
// - 포트 번호가 잘못된 경우 예외 발생
void Server::setPort(std::string const& port) {
  if (_isPortSet) {
    throw std::runtime_error("[1004] Server: setPort - port already set");
  }

  if (isValidPort(port) == false) {
    throw std::runtime_error("[1005] Server: setPort - invalid port");
  }

  _port = Util::stoi(port);
  _isPortSet = true;
}

// server name을 추가
// - 올바르지 않은 문자를 가진 server name이 들어온 경우 예외 발생
// - 이미 있는 server name이 들어올 경우 예외 발생
void Server::addServerName(std::string const& serverName) {
  for (size_t i = 0; i < serverName.size(); i++) {
    char ch = serverName[i];
    if (Util::isUnreserved(ch) or Util::isSubDelims(ch)) continue;
    if (Util::isPctEncoded(serverName.substr(i, 3))) continue;

    throw std::runtime_error(
        "[1006] Server: addServerName - invalid server_name");
  }

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

  if (uri == "/") _isIncludeRootBlock = true;
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

bool Server::isRequiredValuesSet(void) const {
  return (_hostIp.size() != 0 and _port != -1 and _isIncludeRootBlock);
}

// Private method

// IP 형식이 올바른지 여부 반환
bool Server::isValidIpFormat(std::string const& ip) {
  std::istringstream ss(ip);
  std::string token;
  int segmentCount = 0;

  while (std::getline(ss, token, '.')) {
    segmentCount++;
    if (segmentCount > 4 or token.empty() or token.length() > 3) {
      return false;
    }

    for (size_t j = 0; j < token.length(); j++) {
      if (!isdigit(token[j])) {
        return false;
      }
    }

    int num = Util::stoi(token);
    if (num < 0 or num > 255) {
      return false;
    }
  }
  return ss.eof() and segmentCount == 4 and ip[ip.length() - 1] != '.';
}

// 포트 번호가 올바른지 여부 반환
bool Server::isValidPort(const std::string& port) {
  if (port.empty() or port.length() > 5) {
    return false;
  }

  for (size_t i = 0; i < port.length(); i++) {
    if (!isdigit(port[i])) {
      return false;
    }
  }

  int portNum = Util::stoi(port);
  if (portNum < 0 or portNum > 65535) {
    return false;
  }

  // well known port
  if (portNum <= 1023) {
    return false;
  }

  return true;
}
