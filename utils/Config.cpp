#include "Config.hpp"

const std::map<int, std::string> Config::statusMessages =
    Config::initializeStatusMessages();

std::map<int, std::string> Config::initializeStatusMessages(void) {
  std::map<int, std::string> messages;
  messages[200] = "OK";
  messages[400] = "Bad Request";
  messages[404] = "Not Found";
  messages[405] = "Http Not Allowed";
  messages[500] = "Internal Server Error";
  return messages;
}