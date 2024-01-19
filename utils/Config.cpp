#include "Config.hpp"

const std::map<int, std::string> Config::statusMessages =
    Config::initializeStatusMessages();

std::map<int, std::string> Config::initializeStatusMessages(void) {
  std::map<int, std::string> messages;
  messages[200] = "OK";
  messages[201] = "Created";
  messages[204] = "No Content";
  messages[206] = "Partial Content";
  messages[300] = "Multiple Choices";
  messages[301] = "Moved Permanently";
  messages[302] = "Found";
  messages[304] = "Not Modified";
  messages[307] = "Temporary Redirect";
  messages[400] = "Bad Request";
  messages[401] = "Unauthorized";
  messages[403] = "Forbidden";
  messages[404] = "Not Found";
  messages[405] = "Method Not Allowed";
  messages[406] = "Not Acceptable";
  messages[408] = "Request Timeout";
  messages[409] = "Conflict";
  messages[410] = "Gone";
  messages[413] = "Payload Too Large";
  messages[414] = "URI Too Long";
  messages[415] = "Unsupported Media Type";
  messages[429] = "Too Many Requests";
  messages[500] = "Internal Server Error";
  messages[501] = "Not Implemented";
  messages[502] = "Bad Gateway";
  messages[503] = "Service Unavailable";
  messages[504] = "Gateway Timeout";
  return messages;
}