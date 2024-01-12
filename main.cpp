#include <iostream>

#include "config/Location.hpp"
#include "config/Server.hpp"

int main(void) {
  /**
   *  location / {
        root /var/www/html;
        index index.html;

        limit_except GET POST;
        client_max_body_size 10000000;
        autoindex off;
        error_page 404 /404.html;
      }
  */

  Location location1("/", "/var/www/html", "index.html");

  location1.addAllowMethod(HTTP_GET);
  location1.addAllowMethod(HTTP_POST);
  location1.setMaxBodySize(10000000);
  location1.setAutoIndex(false);
  location1.addErrorPage(404, "/404.html");

  /**
   *  location /images/ {
        root /var/www/images;
        index index.html;

        limit_except GET;
        autoindex on;
      }
  */

  Location location2("/images/", "/var/www/images", "index.html");

  location2.addAllowMethod(HTTP_GET);
  location2.setAutoIndex(true);

  /**
   *  location /api/ {
        root /var/www/api;
        index index.php;

        limit_except GET POST DELETE;
        client_max_body_size 15000000;
        autoindex off;
        error_page 500 /500.html;
        error_page 403 /403.html;
      }
  */

  Location location3("/api/", "/var/www/api", "index.php");

  location3.addAllowMethod(HTTP_GET);
  location3.addAllowMethod(HTTP_POST);
  location3.addAllowMethod(HTTP_DELETE);
  location3.setMaxBodySize(15000000);
  location3.setAutoIndex(false);
  location3.addErrorPage(500, "/500.html");
  location3.addErrorPage(403, "/403.html");

  /**
   * server {
      listen 127.0.0.1:8080;
      server_name example.com www.example.com;
    }
  */

  Server server("127.0.0.1", 8080);

  server.addLocationBlock(location1);
  server.addLocationBlock(location2);
  server.addLocationBlock(location3);

  Location res = server.getMatchedLocationBlock("/api/");

  std::cout << res.getRootPath() << std::endl;

  return 0;
}
