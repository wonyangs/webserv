#include "Kqueue.hpp"

#include <unistd.h>

#include <stdexcept>

int Kqueue::_fd = -1;
struct timespec Kqueue::_timeout;
std::map<int, int> Kqueue::_events;

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
  _events.clear();
}

// Public method

// READ 이벤트 추가
// - 이벤트 추가에 실패한 경우 예외 발생
void Kqueue::addReadEvent(int fd) {
  struct kevent event;
  EV_SET(&event, fd, Event::READ, EV_ADD, 0, 0, NULL);

  if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
    throw std::runtime_error("[3001] Kqueue: addReadEvent - event add failed");
  }

  updateEventStatus(fd, READ_EVENT, ADD);
}

// WRITE 이벤트 추가
// - 이벤트 추가에 실패한 경우 예외 발생
void Kqueue::addWriteEvent(int fd) {
  struct kevent event;
  EV_SET(&event, fd, Event::WRITE, EV_ADD, 0, 0, NULL);

  if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
    throw std::runtime_error("[3002] Kqueue: addWriteEvent - event add failed");
  }

  updateEventStatus(fd, WRITE_EVENT, ADD);
}

// PROC 이벤트 추가
// - 이벤트 추가에 실패한 경우 예외 발생
void Kqueue::addProcessEvent(int pid) {
  struct kevent event;
  EV_SET(&event, pid, Event::PROC, EV_ADD, 0, 0, NULL);

  if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
    throw std::runtime_error(
        "[3008] Kqueue: addProcessEvent - event add failed");
  }

  updateEventStatus(-pid, PROC_EVENT, ADD);
}

// READ 이벤트 제거
// - 이벤트 제거에 실패한 경우 예외 발생
void Kqueue::removeReadEvent(int fd) {
  struct kevent event;
  EV_SET(&event, fd, Event::READ, EV_DELETE, 0, 0, NULL);

  if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
    throw std::runtime_error(
        "[3003] Kqueue: removeReadEvent - event remove failed");
  }

  updateEventStatus(fd, READ_EVENT, REMOVE);
}

// WRITE 이벤트 제거
// - 이벤트 제거에 실패한 경우 예외 발생
void Kqueue::removeWriteEvent(int fd) {
  struct kevent event;
  EV_SET(&event, fd, Event::WRITE, EV_DELETE, 0, 0, NULL);

  if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
    throw std::runtime_error(
        "[3004] Kqueue: removeWriteEvent - event remove failed");
  }

  updateEventStatus(fd, WRITE_EVENT, REMOVE);
}

// WRITE 이벤트 제거
// - 이벤트 제거에 실패한 경우 예외 발생
void Kqueue::removeProcessEvent(int pid) {
  struct kevent event;
  EV_SET(&event, pid, Event::PROC, EV_DELETE, 0, 0, NULL);

  if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
    throw std::runtime_error(
        "[3009] Kqueue: removeProcessEvent - event remove failed");
  }

  updateEventStatus(-pid, PROC_EVENT, REMOVE);
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

// Kqueue에서 fd가 가진 모든 이벤트를 제거
// - 이벤트 제거 시스템콜이 실패한 경우 예외 발생
// - 등록되어 있지 않은 이벤트에 대해서는 아무 동작도 하지 않음
void Kqueue::removeAllEvents(int fd) {
  int events = _events[fd];

  // READ 이벤트 제거
  if (events & READ_EVENT) {
    struct kevent event;
    EV_SET(&event, fd, Event::READ, EV_DELETE, 0, 0, NULL);
    if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
      throw std::runtime_error(
          "[3006] Kqueue: removeAllEvents - remove read event failed");
    }
  }

  // WRITE 이벤트 제거
  if (events & WRITE_EVENT) {
    struct kevent event;
    EV_SET(&event, fd, Event::WRITE, EV_DELETE, 0, 0, NULL);
    if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
      throw std::runtime_error(
          "[3007] Kqueue: removeAllEvents - remove write event failed");
    }
  }

  // WRITE 이벤트 제거
  if (events & PROC_EVENT) {
    struct kevent event;
    EV_SET(&event, fd, Event::PROC, EV_DELETE, 0, 0, NULL);
    if (kevent(_fd, &event, 1, NULL, 0, NULL) == -1) {
      throw std::runtime_error(
          "[3010] Kqueue: removeAllEvents - remove process event failed");
    }
  }

  _events.erase(fd);
}

/**
 * Private method
 */

// 이벤트 관리 컨테이너 업데이트
// - fd, 이벤트 타입. 등록/삭제를 선택
void Kqueue::updateEventStatus(int fd, EEventType event, EEventAction action) {
  switch (action) {
    case ADD:
      _events[fd] |= event;
      break;
    case REMOVE:
      _events[fd] &= ~event;
      if (_events[fd] == NO_EVENT) {
        _events.erase(fd);
      }
      break;
  }
}
