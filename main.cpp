#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "config/Server.hpp"
#include "server/EventLoop.hpp"

std::vector<Server> exampleParseConfig(void) {
  // Server 1의 Location 블록 설정
  Location location1_1("/", "/Users/wonyang/Project/webserv/www/",
                       "index.html");
  location1_1.addAllowMethod(HTTP_GET);
  location1_1.addAllowMethod(HTTP_POST);
  location1_1.setMaxBodySize(10000000);
  location1_1.setAutoIndex(false);
  location1_1.addErrorPage(404, "/404.html");

  Location location1_2("/images/", "/Users/wonyang/Project/webserv/www/images/",
                       "index.html");
  location1_2.addAllowMethod(HTTP_GET);
  location1_2.setAutoIndex(true);

  Location location1_3("/docs/", "/var/www/docs", "index.html");
  location1_3.addAllowMethod(HTTP_GET);
  location1_3.setAutoIndex(false);

  // Server 2의 Location 블록 설정
  Location location2_1("/", "/var/www/html", "index.html");
  location2_1.addAllowMethod(HTTP_GET);
  location2_1.setMaxBodySize(8000000);
  location2_1.setAutoIndex(true);

  Location location2_2("/media/", "/var/www/media", "index.html");
  location2_2.addAllowMethod(HTTP_GET);
  location2_2.addAllowMethod(HTTP_POST);
  location2_2.setAutoIndex(false);

  // Server 3의 Location 블록 설정
  Location location3_1("/", "/var/www/html", "index.html");
  location3_1.addAllowMethod(HTTP_GET);
  location3_1.addAllowMethod(HTTP_POST);
  location3_1.addAllowMethod(HTTP_DELETE);
  location3_1.setMaxBodySize(15000000);
  location3_1.setAutoIndex(false);
  location3_1.addErrorPage(500, "/500.html");
  location3_1.addErrorPage(403, "/403.html");

  Location location3_2("/api/", "/var/www/api", "index.php");
  location3_2.addAllowMethod(HTTP_GET);
  location3_2.addAllowMethod(HTTP_POST);
  location3_2.setMaxBodySize(10000000);
  location3_2.setAutoIndex(false);

  Location location3_3("/downloads/", "/var/www/downloads", "index.html");
  location3_3.addAllowMethod(HTTP_GET);
  location3_3.setAutoIndex(true);

  // Server 1 생성 및 Location 블록 추가
  Server server1("127.0.0.1", 8080);
  server1.addServerName("test.com");
  server1.addServerName("www.test.com");
  server1.addLocationBlock(location1_1);
  server1.addLocationBlock(location1_2);
  server1.addLocationBlock(location1_3);

  // Server 2 생성 및 Location 블록 추가
  Server server2("127.0.0.1", 8080);
  server2.addServerName("test.org");
  server2.addServerName("www.test.org");
  server2.addLocationBlock(location2_1);
  server2.addLocationBlock(location2_2);

  // Server 3 생성 및 Location 블록 추가
  Server server3("127.0.0.1", 8081);
  server3.addServerName("test.com");
  server3.addServerName("www.test.com");
  server3.addLocationBlock(location3_1);
  server3.addLocationBlock(location3_2);
  server3.addLocationBlock(location3_3);

  // 서버들을 vector에 추가
  std::vector<Server> servers;
  servers.push_back(server1);
  servers.push_back(server2);
  servers.push_back(server3);

  return servers;
}

int main(void) {
  try {
    std::vector<Server> servers = exampleParseConfig();

    EventLoop loop(servers);
    loop.run();
  } catch (std::exception const& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
