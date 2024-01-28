#include <signal.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "config/Server.hpp"
#include "server/EventLoop.hpp"

std::vector<Server> exampleParseConfig(void) {
  // 기본 Config
  Location location1_1("/", "/Users/wonyang/Project/webserv/www/",
                       "upload.html");
  location1_1.addAllowMethod(HTTP_GET);
  location1_1.setAutoIndex(false);
  location1_1.addErrorPage(404, "/404.html");
  location1_1.setCgiExtention(".py");
  location1_1.setCgiPath(
      "/Users/wonyang/Project/webserv/cgi-bin/my-python-cgi.py");
  location1_1.setUploadDir("/Users/wonyang/Project/webserv/www/upload/");

  Location location1_2("/upload/", "/Users/wonyang/Project/webserv/www/upload/",
                       "bird.png");
  location1_2.addAllowMethod(HTTP_GET);
  location1_2.addAllowMethod(HTTP_DELETE);

  Location location1_3("/cgi-bin/", "/Users/wonyang/Project/webserv/cgi-bin/",
                       "index.py");
  location1_3.setCgiExtention(".py");
  location1_3.setCgiPath(
      "/Users/wonyang/Project/webserv/cgi-bin/my-python-cgi.py");
  location1_3.setUploadDir("/Users/wonyang/Project/webserv/www/upload/");

  Server server1("127.0.0.1", 8080);
  server1.addLocationBlock(location1_1);
  server1.addLocationBlock(location1_2);
  server1.addLocationBlock(location1_3);

  // 서버들을 vector에 추가
  std::vector<Server> servers;
  servers.push_back(server1);

  return servers;
}

int main(void) {
  try {
    signal(SIGCHLD, SIG_IGN);
    std::vector<Server> servers = exampleParseConfig();

    EventLoop loop(servers);
    loop.run();
  } catch (std::exception const& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
