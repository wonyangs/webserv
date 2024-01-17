#include "ServerManager.hpp"

#include <unistd.h>

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "../core/Kqueue.hpp"
#include "../core/Socket.hpp"

/**
 * Constructor & Destructor
 */

ServerManager::ServerManager(std::string hostIp, int port,
                             std::vector<Server> configs)
    : _hostIp(hostIp), _port(port), _configs(configs) {}

ServerManager::ServerManager(ServerManager const& manager)
    : _serverFd(manager._serverFd),
      _hostIp(manager._hostIp),
      _port(manager._port),
      _configs(manager._configs),
      _connections(manager._connections) {}

ServerManager::~ServerManager(void) {}

/**
 * Operator Overloading
 */

ServerManager& ServerManager::operator=(ServerManager const& manager) {
  if (this != &manager) {
    _serverFd = manager._serverFd;
    _hostIp = manager._hostIp;
    _port = manager._port;
    _configs = manager._configs;
    _connections = manager._connections;
  }
  return *this;
}

/**
 * Public method
 */

// 서버 소켓 할당 및 listening 시작
// - 서버 시작 실패 시 예외 발생
void ServerManager::runServer(void) {
  try {
    _serverFd = Socket::socket();

    Socket::setsockopt(_serverFd);
    Socket::setNonBlocking(_serverFd);
    Socket::bind(_serverFd, _hostIp, _port);
    Socket::listen(_serverFd, 3);

    Kqueue::addReadEvent(_serverFd);
  } catch (const std::exception& e) {
    if (_serverFd != -1) {
      close(_serverFd);  // Todo: 서버 닫기 메서드 구현
    }
    throw;
  }
}

/**
 * Public method - event
 */

// 이벤트 처리
// - 처리 가능한 fd만 들어온다고 가정
// - 서버 fd인 경우 커넥션 추가
// - 클라이언트 fd인 경우 이벤트 type에 따라 처리
// - 예상치 못한 이벤트 타입이 온 경우 예외 발생
void ServerManager::handleEvent(Event event) {
  int eventFd = event.getFd();

  // 서버 fd인 경우 커넥션 추가
  if (eventFd == _serverFd) {
    int clientFd = Socket::accept(eventFd);

    addConnection(clientFd);
    Kqueue::addReadEvent(clientFd);
    return;
  }

  // 클라이언트 fd인 경우 이벤트 처리
  switch (event.getType()) {
    case Event::READ:
      handleReadEvent(eventFd);
      break;

    case Event::WRITE:
      handleWriteEvent(eventFd);
      break;

    default:
      throw std::runtime_error(
          "[] ServerManager: handleEvent - unknown event type");
  }
}

// 읽기 이벤트 처리
// - 이벤트 처리 과정 중 예외 발생 가능
void ServerManager::handleReadEvent(int eventFd) {
  ConnectionMap::iterator it = _connections.find(eventFd);
  Connection& connection = it->second;

  try {
    connection.readSocket();
    if (connection.getConnectionStatus() == Connection::CLOSE) {
      Kqueue::removeReadEvent(eventFd);
      removeConnection(eventFd);
    } else if (connection.getConnectionStatus() == Connection::TO_SEND) {
      Kqueue::removeReadEvent(eventFd);
      Kqueue::addWriteEvent(eventFd);
    }
  } catch (StatusException const& e) {
    int code = e.getStatusCode();
    connection.sendErrorPage(code);  // 얘도 kqueue로 관리
    removeConnection(connection.getFd());
  } catch (std::exception const& e) {
    connection.sendErrorPage(500);
    removeConnection(connection.getFd());
  }
}

// 쓰기 이벤트 처리
// - 이벤트 처리 과정 중 예외 발생 가능
void ServerManager::handleWriteEvent(int eventFd) {
  ConnectionMap::iterator it = _connections.find(eventFd);
  Connection& connection = it->second;

  try {
    connection.send();
    Kqueue::removeWriteEvent(eventFd);

    if (connection.isReadStorageRequired()) {
      connection.readStorage();
      if (connection.getConnectionStatus() == Connection::TO_SEND) {
        Kqueue::addWriteEvent(eventFd);
      } else {
        Kqueue::addReadEvent(eventFd);
      }
      return;
    }

    Kqueue::addReadEvent(eventFd);

  } catch (StatusException const& e) {
    int code = e.getStatusCode();
    connection.sendErrorPage(code);  // 얘도 kqueue로 관리
    removeConnection(connection.getFd());
  } catch (std::exception const& e) {
    connection.sendErrorPage(500);
    removeConnection(connection.getFd());
  }
}

/**
 * Public method - etc
 */

// 서버 fd를 반환
int ServerManager::getServerFd(void) const { return _serverFd; }

// 서버가 해당 event를 처리할 수 있는지 여부 반환
bool ServerManager::canHandleEvent(Event event) const {
  int eventFd = event.getFd();

  return (eventFd == _serverFd or hasConnectionFd(eventFd));
}

// timeout된 커넥션을 관리 목록에서 제거
// - 호출 경과 시간이 CONNECTION_LIMIT_TIME 이상인 경우 제거 대상
void ServerManager::manageTimeoutConnections(void) {
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

/**
 * Private method
 */

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

// 커넥션 중 해당하는 fd가 있는지 확인
bool ServerManager::hasConnectionFd(int fd) const {
  return (_connections.find(fd) != _connections.end());
}
