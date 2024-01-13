#ifndef __EVENTLOOP_HPP__
#define __EVENTLOOP_HPP__

#include "Kqueue.hpp"
#include "ServerManager.hpp"

// 전체 흐름 제어 클래스
class EventLoop {
 private:
  ServerManager _manager;  // todo: 이후에 vector로 구현

 public:
  EventLoop(void);  // todo: config 설정 객체를 받아 초기화
  ~EventLoop(void);

  void run(void);

 private:
  EventLoop& operator=(EventLoop const& eventloop);
  EventLoop(EventLoop const& eventloop);
};

#endif