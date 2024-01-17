#ifndef __EVENTLOOP_HPP__
#define __EVENTLOOP_HPP__

#include <map>
#include <vector>

#include "../core/Kqueue.hpp"
#include "ServerManager.hpp"

// 전체 흐름 제어 클래스
class EventLoop {
 private:
  std::map<int, ServerManager> _managers;

 public:
  // todo: config 설정 객체를 받아 초기화
  EventLoop(std::vector<Server> const& servers);
  ~EventLoop(void);

  void run(void);

 private:
  typedef std::pair<std::string, int> ServerKey;
  typedef std::vector<Server> ServerList;
  typedef std::map<ServerKey, ServerList> ServerMap;
  typedef std::map<int, ServerManager> ManagerMap;


  void printServerMap(const ServerMap& serverMap);  // debug
  bool isServerFd(int fd);
  void closeTimeoutConnections(void);

  EventLoop(void);
  EventLoop& operator=(EventLoop const& eventloop);
  EventLoop(EventLoop const& eventloop);
};

#endif