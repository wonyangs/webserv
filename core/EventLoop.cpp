#include "EventLoop.hpp"

#include <iostream>

#include "Kqueue.hpp"
#include "Socket.hpp"

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
    executeTimeout();

    Event event = Kqueue::getEvent();
    if (event.isInvalid()) {
      continue;
    }

    int eventFd = event.getFd();

    // todo: 이후에 manager에게 책임 넘기기
    if (isServerFd(eventFd)) {
      int clientFd = Socket::accept(eventFd);
      std::cout << clientFd << std::endl;

      ManagerMap::iterator it = _managers.find(eventFd);

      (it->second).addConnection(clientFd);
      Kqueue::addReadEvent(clientFd);

      std::cout << "connection success" << std::endl;
    } else {
      for (ManagerMap::iterator it = _managers.begin(); it != _managers.end();
           ++it) {
        ServerManager& manager = it->second;

        if (manager.hasFd(eventFd)) {
          manager.handleConnection(event);
          break;
        }
      }

      std::cout << "event received" << std::endl;
    }
  }
}

// Private Method

// 해당하는 fd가 서버의 fd인지 여부 반환
bool EventLoop::isServerFd(int fd) {
  return (_managers.find(fd) != _managers.end());
}

// 서버 전체 timeout 실행
void EventLoop::executeTimeout(void) {
  for (ManagerMap::iterator it = _managers.begin(); it != _managers.end();
       ++it) {
    (it->second).timeout();
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
