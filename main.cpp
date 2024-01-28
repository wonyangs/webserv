#include <signal.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "config/ConfigParser.hpp"
#include "config/Server.hpp"
#include "server/EventLoop.hpp"

int main(int argc, char** argv) {
  if (2 < argc) {
    std::cout << "[] main - insufficient number of arguments :" << argc - 1
              << std::endl;
    return 0;
  }

  try {
    signal(SIGCHLD, SIG_IGN);

    char const* configPath = argc == 2 ? argv[1] : "./conf/default.conf";
    ConfigParser configParser(configPath);
    std::vector<Server> servers = configParser.parse();

    EventLoop loop(servers);
    loop.run();
  } catch (std::exception const& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
