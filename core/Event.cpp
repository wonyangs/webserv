#include "Event.hpp"

#include <stdexcept>

// Constructor & Destructor

Event::Event(void) : _fd(-1) {}

Event::Event(struct kevent const& event) {
  _fd = static_cast<int>(event.ident);
  if (_fd < 0) {
    throw std::runtime_error("[3100] Event: Event - invalid fd");
  }

  if (event.filter == EVFILT_READ) {
    _type = READ;
  } else if (event.filter == EVFILT_WRITE) {
    _type = WRITE;
  } else {
    throw std::runtime_error("[3101] Event: Event - undefine event type");
  }
}

Event::Event(Event const& event) { *this = event; }

Event::~Event(void) {}

// Operator overloading

Event& Event::operator=(Event const& event) {
  if (this != &event) {
    this->_fd = event._fd;
    this->_type = event._type;
  }
  return *this;
}

// Public method

// 이벤트가 감시하는 fd 반환
int Event::getFd(void) const { return _fd; }

// 이벤트의 타입을 반환
// - 이벤트의 타입이 설정되어 있지 않은 경우 예외 발생
Event::EventType Event::getType(void) const {
  if (_fd == -1) {
    throw std::runtime_error("[3102] Event: getType - event not set");
  }
  return _type;
}

// 올바른 이벤트인지 여부 반환
// - kevent에서 반환된 이벤트가 없을 경우 올바르지 않은 이벤트가 반환될 수 있음
bool Event::isValid(void) const {
  if (_fd < 0) {
    return false;
  }
  return true;
}
