#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <map>
#include <string>
#include <vector>

#include "utils/Enum.hpp"
#include "utils/StatusException.hpp"

// HTTP Request 클래스
class Request {
 private:
  enum EHttpMethod _method;
  std::string _path;
  std::string _httpVersion;
  std::map<std::string, std::vector<std::string> > _header;
  std::string _body;

 public:
  Request(void);
  Request(Request const& request);
  ~Request(void);

  Request& operator=(Request const& request);

  void print(void);  // debug

  std::vector<std::string> const& getHeaderFieldValues(
      std::string const& fieldName);

  void storeRequestLine(std::vector<std::string> const& result);
  void storeHeaderField(std::vector<std::string> const& result);
  void storeBody(std::string const& result);

  bool isHeaderFieldNameExists(std::string const& fieldName);
  bool isHeaderFieldValueExists(std::string const& fieldName,
                                std::string const& fieldValue);

  void clear(void);

 private:
  EHttpMethod matchEHttpMethod(std::string method);
};

#endif
