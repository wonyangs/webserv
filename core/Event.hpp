#ifndef __EVENT_HPP__
#define __EVENT_HPP__

#include <sys/event.h>

#include <cstdlib>

// kevent 구조체 정보를 관리하는 클래스
class Event {
 public:
  enum EventType {
    READ = EVFILT_READ,
    WRITE = EVFILT_WRITE,
    PROC = EVFILT_PROC,
    NONE
  };

 private:
  int _fd;
  EventType _type;

 public:
  Event(void);
  Event(struct kevent const& event);
  Event(Event const& event);
  ~Event(void);

  Event& operator=(Event const& event);

  int getFd(void) const;
  EventType getType(void) const;

  bool isInvalid(void) const;

 private:
};

#endif