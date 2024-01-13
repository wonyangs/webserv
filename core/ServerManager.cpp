#include "ServerManager.hpp"

#include <vector>

// Constructor & Destructor

ServerManager::ServerManager(Server config) : _config(config) {}

ServerManager::ServerManager(ServerManager const& manager)
    : _config(manager._config), _connections(manager._connections) {}

ServerManager::~ServerManager(void) {}

// Operator Overloading

ServerManager& ServerManager::operator=(ServerManager const& manager) {
  if (this != &manager) {
    _config = manager._config;
    _connections = manager._connections;
  }
  return *this;
}

// Public Method

// 관리할 fd를 추가
// - 이미 있는 fd가 들어온 경우 예외 발생
void ServerManager::addConnection(int fd) {
  if (_connections.find(fd) != _connections.end()) {
    throw std::runtime_error(
        "[3300] ServerManager: addConnection - duplicate connection fd");
  }
  _connections.insert(std::make_pair(fd, Connection(fd)));
}

// fd를 관리 목록에서 제거
// - 없는 fd를 제거하려고 한 경우 예외 발생
void ServerManager::removeConnection(int fd) {
  ConnectionMap::iterator it = _connections.find(fd);

  if (it == _connections.end()) {
    throw std::runtime_error(
        "[3301] ServerManager: removeConnection - no such connection fd");
  }

  Connection& connection = it->second;
  connection.close();
  _connections.erase(fd);
}

// 해당하는 fd가 있는지 확인
bool ServerManager::hasFd(int fd) const {
  return (_connections.find(fd) != _connections.end());
}

#include <iostream>

// timeout된 커넥션을 관리 목록에서 제거
// - 호출 경과 시간이 CONNECTION_LIMIT_TIME 이상인 경우 제거 대상
void ServerManager::timeout(void) {
  std::vector<int> removeFdVec;

  // timeout 커넥션 탐색
  ConnectionMap::iterator mit = _connections.begin();
  while (mit != _connections.end()) {
    Connection& connection = mit->second;
    if (connection.getElapsedTime() >= CONNECTION_LIMIT_TIME) {
      removeFdVec.push_back(connection.getFd());
    }
    ++mit;
  }

  // timeout 커넥션 제거
  std::vector<int>::iterator vit = removeFdVec.begin();
  while (vit != removeFdVec.end()) {
    removeConnection(*vit);
    ++vit;
  }
}
