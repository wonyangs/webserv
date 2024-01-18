#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <map>
#include <string>
#include <vector>

#include "../utils/Config.hpp"
#include "../utils/Enum.hpp"
#include "../utils/StatusException.hpp"

// HTTP Request 클래스
class Request {
 private:
  enum EHttpMethod _method;
  std::string _path;
  std::string _query;
  std::string _httpVersion;
  std::map<std::string, std::vector<std::string> > _header;
  std::string _body;

 public:
  Request(void);
  Request(Request const& request);
  ~Request(void);

  Request& operator=(Request const& request);

  enum EHttpMethod const& getMethod(void) const;
  std::string const& getPath(void) const;
  std::string const& getQuery(void) const;
  std::string const& getHttpVersion(void) const;
  std::map<std::string, std::vector<std::string> > const& getHeader(void) const;
  std::string const& getBody(void) const;

  std::vector<std::string> const& getHeaderFieldValues(
      std::string const& fieldName) const;

  void print(void) const;  // debug

  void setMethod(std::string const& method);
  void setPath(std::string const& path);
  void setQuery(std::string const& query);
  void setHttpVersion(std::string const& httpVersion);

  void storeRequestLine(std::vector<std::string> const& result);
  void storeHeaderField(std::vector<std::string> const& result);
  void storeBody(std::string const& result);

  bool isHeaderFieldNameExists(std::string const& fieldName) const;
  bool isHeaderFieldValueExists(std::string const& fieldName,
                                std::string const& fieldValue);

  void clear(void);

 private:
  EHttpMethod matchEHttpMethod(std::string method);
  void splitRequestTarget(std::string& path, std::string& query,
                          const std::string& requestTarget);
};

#endif
