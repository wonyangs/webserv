#include "EventLoop.hpp"

#include "Kqueue.hpp"
#include "Socket.hpp"

// Constructor & Destructor

// 임시로 ServerManager를 만들어 넣음
EventLoop::EventLoop(void) : _manager(Server("127.0.0.1", 8080)) {
  Kqueue::start();
  _manager.runServer();
}

EventLoop::~EventLoop(void) {}

#include <iostream>

void EventLoop::run(void) {
  while (true) {
    Event event = Kqueue::getEvent();
    _manager.timeout();
    if (event.isInvalid()) {
      continue;
    }

    // todo: 이후에 manager에게 책임 넘기기
    if (event.getFd() == _manager.getServerFd()) {
      int clientFd = Socket::accept(_manager.getServerFd());
      std::cout << clientFd << std::endl;
      _manager.addConnection(clientFd);
      Kqueue::addReadEvent(clientFd);
      std::cout << "connection success" << std::endl;
    } else {
      _manager.handleConnection(event);
      std::cout << "event received" << std::endl;
    }
  }
}