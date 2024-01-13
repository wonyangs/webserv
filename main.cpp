#include <unistd.h>

#include <iostream>

#include "core/EventLoop.hpp"

int main(void) {
  try {
    EventLoop loop;
    loop.run();
  } catch (std::exception const& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
