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

  for (ServerMap::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
    ServerKey const& key = it->first;
    ServerManager manager(key.first, key.second, it->second);
    manager.init();
    int fd = manager.getServerFd();
    _managers.insert(std::make_pair(fd, manager));
  }
}

EventLoop::~EventLoop(void) {}

// Public method

void EventLoop::run(void) {
  start();

  while (true) {
    try {
      closeTimeoutConnections();

      Event event = Kqueue::getEvent();
      if (event.isInvalid()) {
        continue;
      }

      std::cout << "event!" << std::endl;

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
        throw std::runtime_error("[4200] EventLoop: run - unexpected event fd");
      }
    } catch (std::exception const& e) {
      std::cout << e.what() << std::endl;
      restart();
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

// 서버 시작
// - 실패할 경우 성공할 때까지 재시도
void EventLoop::start(void) {
  bool isStart = false;
  do {
    try {
      Kqueue::start();
      for (ManagerMap::iterator it = _managers.begin(); it != _managers.end();
           ++it) {
        (it->second).run();
      }
      isStart = true;
    } catch (std::exception const& e) {
      std::cout << "Server start retry..." << std::endl;
      std::cout << e.what() << std::endl;
    }
  } while (isStart == false);
}

// 서버 재시작
// - 실패할 경우 성공할 때까지 재시도
void EventLoop::restart(void) {
  bool isRestart = false;
  do {
    std::cout << "Server restarting..." << std::endl;
    try {
      for (ManagerMap::iterator it = _managers.begin(); it != _managers.end();
           ++it) {
        (it->second).clear();
      }
      Kqueue::close();

      Kqueue::start();
      for (ManagerMap::iterator it = _managers.begin(); it != _managers.end();
           ++it) {
        (it->second).run();
      }
      isRestart = true;
    } catch (std::exception const& e) {
      std::cout << e.what() << std::endl;
    }
  } while (isRestart == false);
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
