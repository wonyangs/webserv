#include "Kqueue.hpp"

#include <unistd.h>

#include <stdexcept>

int Kqueue::_fd = -1;
struct timespec Kqueue::_timeout;

// Kqueue를 할당
// - 할당에 실패할 경우 예외 발생
void Kqueue::start(void) {
  _fd = kqueue();
  if (_fd == -1) {
    throw std::runtime_error("[3000] Kqueue: start - kqueue failed");
  }
  // Non-blocking을 위해 대기 시간을 0초로 설정
  _timeout.tv_sec = 0;
  _timeout.tv_nsec = 0;
}

// Kqueue가 할당된 fd를 닫음
void Kqueue::close(void) {
  if (_fd != -1) {
    ::close(_fd);
  }
}

// Public method

// READ 이벤트 추가
// - 이벤트 추가에 실패한 경우 예외 발생
void Kqueue::addReadEvent(int fd) {
  struct kevent event;
  EV_SET(&event, fd, Event::READ, ADD, 0, 0, NULL);

  if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
    throw std::runtime_error("[3001] Kqueue: addReadEvent - event add failed");
  }
}

// WRITE 이벤트 추가
// - 이벤트 추가에 실패한 경우 예외 발생
void Kqueue::addWriteEvent(int fd) {
  struct kevent event;
  EV_SET(&event, fd, Event::WRITE, ADD, 0, 0, NULL);

  if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
    throw std::runtime_error("[3002] Kqueue: addWriteEvent - event add failed");
  }
}

// READ 이벤트 제거
// - 이벤트 제거에 실패한 경우 예외 발생
void Kqueue::removeReadEvent(int fd) {
  struct kevent event;
  EV_SET(&event, fd, Event::READ, DELETE, 0, 0, NULL);

  if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
    throw std::runtime_error(
        "[3003] Kqueue: removeReadEvent - event remove failed");
  }
}

// WRITE 이벤트 제거
// - 이벤트 제거에 실패한 경우 예외 발생
void Kqueue::removeWriteEvent(int fd) {
  struct kevent event;
  EV_SET(&event, fd, Event::WRITE, DELETE, 0, 0, NULL);

  if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
    throw std::runtime_error(
        "[3004] Kqueue: removeWriteEvent - event remove failed");
  }
}

// 발생한 이벤트 반환
// - 반환받은 이벤트는 Event::isValid 함수를 통해 올바른지 확인하고 사용해야함
// - kevent에서 반환에 실패한 경우 예외 발생
Event Kqueue::getEvent(void) {
  struct kevent tmp;
  int nev = kevent(_fd, NULL, 0, &tmp, 1, &_timeout);

  if (nev < 0) {
    throw std::runtime_error(
        "[3005] Kqueue: getEvent - event detection failed");
  } else if (nev == 0) {
    return Event();
  }
  return Event(tmp);
}
