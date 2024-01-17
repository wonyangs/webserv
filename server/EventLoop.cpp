#include "EventLoop.hpp"

#include <iostream>

#include "../core/Kqueue.hpp"
#include "../core/Socket.hpp"

// Constructor & Destructor

// 임시로 ServerManager를 만들어 넣음
EventLoop::EventLoop(std::vector<Server> const& servers) {
  ServerMap tmp;

  for (ServerList::const_iterator it = servers.begin(); it != servers.end();
       ++it) {
    ServerKey p = std::make_pair((*it).getHostIp(), (*it).getPort());

    ServerMap::iterator mit = tmp.find(p);
    if (mit == tmp.end()) {
      ServerList v;
      v.push_back(*it);
      tmp.insert(std::make_pair(p, v));
    } else {
      mit->second.push_back(*it);
    }
  }

  printServerMap(tmp);  // debug

  Kqueue::start();
  for (ServerMap::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
    ServerKey const& key = it->first;
    ServerManager manager(key.first, key.second, it->second);
    manager.runServer();
    int fd = manager.getServerFd();
    _managers.insert(std::make_pair(fd, manager));
  }
}

EventLoop::~EventLoop(void) {}

// Public method

void EventLoop::run(void) {
  while (true) {
    try {
      closeTimeoutConnections();

      Event event = Kqueue::getEvent();
      if (event.isInvalid()) {
        continue;
      }

      ManagerMap::iterator it = _managers.begin();
      while (it != _managers.end()) {
        ServerManager& manager = it->second;

        if (manager.canHandleEvent(event)) {
          manager.handleEvent(event);
          break;
        }
        ++it;
      }
      if (it == _managers.end()) {
        throw std::runtime_error("EventLoop: run - unexpected event fd");
      }
    } catch (std::exception const& e) {
      std::cout << e.what() << std::endl;
    }
  }
}

// Private Method

// 서버 전체 timeout 실행
void EventLoop::closeTimeoutConnections(void) {
  for (ManagerMap::iterator it = _managers.begin(); it != _managers.end();
       ++it) {
    (it->second).manageTimeoutConnections();
  }
}

// debug
void EventLoop::printServerMap(const ServerMap& serverMap) {
  for (ServerMap::const_iterator it = serverMap.begin(); it != serverMap.end();
       ++it) {
    // 키를 출력합니다 (IP 주소와 포트)
    const ServerKey& key = it->first;
    std::cout << "Key: IP = " << key.first << ", Port = " << key.second
              << std::endl;

    // 각 서버의 정보를 출력합니다
    const ServerList& servers = it->second;
    for (ServerList::const_iterator serverIt = servers.begin();
         serverIt != servers.end(); ++serverIt) {
      std::cout << "    Server IP: " << serverIt->getHostIp()
                << ", Port: " << serverIt->getPort() << std::endl;
    }
  }
}
