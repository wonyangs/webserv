#include <signal.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "config/Server.hpp"
#include "server/EventLoop.hpp"

std::vector<Server> exampleParseConfig(void) {
  // Server 1의 Location 블록 설정
  // Location location1_1("/", "/Users/wonyang/Project/webserv/www/",
  //                      "index.html");
  // location1_1.addAllowMethod(HTTP_GET);
  // location1_1.addAllowMethod(HTTP_POST);
  // location1_1.setMaxBodySize(10000000);
  // location1_1.setAutoIndex(true);
  // location1_1.addErrorPage(404, "/404.html");

  // Location location1_2("/images/",
  // "/Users/wonyang/Project/webserv/www/images/",
  //                      "index.html");
  // location1_2.addAllowMethod(HTTP_GET);
  // location1_2.setAutoIndex(true);

  // Location location1_3(
  //     "/cgi-bin/", "/Users/wonyang/Project/webserv/www/cgi-bin/",
  //     "index.html");
  // location1_3.setCgiExtention(".py");
  // location1_3.setCgiPath("/");
  // location1_3.setUploadDir("uploadFile");

  Location location1_1("/", "/Users/wonyang/Project/webserv/www/",
                       "index.html");
  location1_1.addAllowMethod(HTTP_GET);
  location1_1.setAutoIndex(true);
  location1_1.addErrorPage(404, "/404.html");

  Location location1_2("/directory",
                       "/Users/wonyang/Project/webserv/www/YoupiBanane/",
                       "youpi.bad_extension");
  location1_2.addAllowMethod(HTTP_GET);
  location1_2.addAllowMethod(HTTP_POST);
  location1_2.setAutoIndex(false);
  location1_2.setCgiExtention(".bla");
  location1_2.setCgiPath("/Users/wonyang/Project/webserv/tester/cgi_tester");
  location1_2.setUploadDir("/Users/wonyang/Project/webserv/www/");

  Location location1_3("/post_body",
                       "/Users/wonyang/Project/webserv/www/YoupiBanane/",
                       "youpi.bad_extension");
  location1_3.setMaxBodySize(100);

  // Server 1 생성 및 Location 블록 추가
  Server server1("127.0.0.1", 8080);
  server1.addLocationBlock(location1_1);
  server1.addLocationBlock(location1_2);
  server1.addLocationBlock(location1_3);

  // Server 2의 Location 블록 설정
  Location location2_1("/", "/var/www/html", "index.html");
  location2_1.addAllowMethod(HTTP_GET);
  location2_1.setMaxBodySize(8000000);
  location2_1.setAutoIndex(true);

  Location location2_2("/media/", "/var/www/media", "index.html");
  location2_2.addAllowMethod(HTTP_GET);
  location2_2.addAllowMethod(HTTP_POST);
  location2_2.setAutoIndex(false);

  // Server 2 생성 및 Location 블록 추가
  Server server2("127.0.0.1", 8080);
  server2.addServerName("test.org");
  server2.addServerName("www.test.org");
  server2.addLocationBlock(location2_1);
  server2.addLocationBlock(location2_2);

  // Server 3의 Location 블록 설정
  Location location3_1("/", "/Users/wonyang/Project/webserv/webpage/cgi-bin/",
                       "index.py");
  location3_1.setMaxBodySize(15000000);
  location3_1.setAutoIndex(false);
  location3_1.setCgiExtention(".py");
  location3_1.setCgiPath(
      "/Users/wonyang/Project/webserv/webpage/cgi-bin/index.py");
  location3_1.setUploadDir("/uploadFile");

  Location location3_2("/login/",
                       "/Users/wonyang/Project/webserv/webpage/cgi-bin/",
                       "login.html");
  location3_2.addAllowMethod(HTTP_GET);
  location3_2.addAllowMethod(HTTP_POST);
  location3_2.setMaxBodySize(10000000);
  location3_2.setAutoIndex(false);
  location3_2.setCgiExtention(".py");
  location3_2.setCgiPath(
      "/Users/wonyang/Project/webserv/webpage/cgi-bin/login.py");
  location3_2.setUploadDir("/uploadFile");

  Location location3_3("/register/",
                       "/Users/wonyang/Project/webserv/webpage/cgi-bin/",
                       "register.py");
  location3_3.setCgiExtention(".py");
  location3_3.setCgiPath(
      "/Users/wonyang/Project/webserv/webpage/cgi-bin/register.py");
  location3_3.setUploadDir("/uploadFile");

  Location location3_4("/post_create/",
                       "/Users/wonyang/Project/webserv/webpage/cgi-bin/",
                       "post_create.py");
  location3_4.setCgiExtention(".py");
  location3_4.setCgiPath(
      "/Users/wonyang/Project/webserv/webpage/cgi-bin/post_create.py");
  location3_4.setUploadDir("uploadFile");

  // Server 3 생성 및 Location 블록 추가
  Server server3("127.0.0.1", 8081);
  server3.addServerName("test.com");
  server3.addServerName("www.test.com");
  server3.addLocationBlock(location3_1);
  server3.addLocationBlock(location3_2);
  server3.addLocationBlock(location3_3);
  server3.addLocationBlock(location3_4);

  // 서버들을 vector에 추가
  std::vector<Server> servers;
  servers.push_back(server1);
  servers.push_back(server2);
  servers.push_back(server3);

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
