#ifndef __KQUEUE_HPP__
#define __KQUEUE_HPP__

#include <sys/event.h>
#include <time.h>

#include "Event.hpp"

// kqueue를 관리하는 정적 클래스
class Kqueue {
 private:
  static int _fd;
  static struct timespec _timeout;

 public:
  static void start(void);
  static void close(void);

  static void addReadEvent(int fd);
  static void addWriteEvent(int fd);
  static void removeReadEvent(int fd);
  static void removeWriteEvent(int fd);
  
  static Event getEvent(void);

 private:
  enum EventAction { ADD = EV_ADD, DELETE = EV_DELETE };

  Kqueue(void);
  Kqueue(Kqueue const& kqueue);
  ~Kqueue(void);
  Kqueue& operator=(Kqueue const& kqueue);
};

#endif