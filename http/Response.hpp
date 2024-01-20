#ifndef __RESPONSE_HPP__
#define __RESPONSE_HPP__

#include <exception>
#include <map>
#include <sstream>
#include <string>

#include "../utils/Config.hpp"
#include "../utils/Enum.hpp"
#include "../utils/StatusException.hpp"

// HTTP Response 클래스
class Response {
 private:
  std::string _responseContent;
  ssize_t _startIndex;

  std::string _httpVersion;
  int _statusCode;
  std::map<std::string, std::string> _header;
  std::string _body;

 public:
  Response(void);
  Response(Response const& response);
  ~Response(void);

  Response& operator=(Response const& response);

  void print(void) const;  // debug

  std::string const& getHttpVersion(void) const;
  int const& getStatusCode(void) const;
  std::map<std::string, std::string> const& getHeader(void) const;
  std::string const& getBody(void) const;

  void setResponseContent(void);

  void setHttpVersion(std::string const& httpVersion);
  void setStatusCode(int const& statusCode);

  void addHeader(std::string const& fieldName, std::string const& fieldValue);
  void appendBody(std::string const& body);

  void clear(void);

  std::string const& findStatusMessage(int code);

 private:
  bool isHeaderFieldNameExists(std::string const& fieldName) const;
};

#endif
