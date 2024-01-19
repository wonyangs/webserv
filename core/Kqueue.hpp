#ifndef __KQUEUE_HPP__
#define __KQUEUE_HPP__

#include <sys/event.h>
#include <time.h>

#include <map>

#include "Event.hpp"

// kqueue를 관리하는 정적 클래스
class Kqueue {
 private:
  static int _fd;
  static struct timespec _timeout;
  static std::map<int, int> _events;

 public:
  static void start(void);
  static void close(void);

  static void addReadEvent(int fd);
  static void addWriteEvent(int fd);
  static void removeReadEvent(int fd);
  static void removeWriteEvent(int fd);

  static void removeAllEvents(int fd);

  static Event getEvent(void);

 private:
  enum EEventAction { ADD, REMOVE };
  enum EEventType { NO_EVENT = 0, READ_EVENT = 1 << 0, WRITE_EVENT = 1 << 1 };

  static void updateEventStatus(int fd, EEventType event, EEventAction action);

  Kqueue(void);
  Kqueue(Kqueue const& kqueue);
  ~Kqueue(void);
  Kqueue& operator=(Kqueue const& kqueue);
};

#endif