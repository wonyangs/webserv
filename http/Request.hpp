#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <cctype>
#include <map>
#include <string>
#include <vector>

#include "../config/Location.hpp"
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
  std::map<std::string, std::string> _header;
  std::string _body;

  bool _locationFlag;
  Location _location;
  std::string _fullPath;

  bool _isConnectionClose;

 public:
  Request(void);
  Request(Request const& request);
  ~Request(void);

  Request& operator=(Request const& request);

  enum EHttpMethod const& getMethod(void) const;
  std::string const& getPath(void) const;
  std::string const& getQuery(void) const;
  std::string const& getHttpVersion(void) const;
  std::map<std::string, std::string> const& getHeader(void) const;
  std::string const& getBody(void) const;
  Location const& getLocation(void) const;
  bool getLocationFlag(void) const;
  std::string const& getFullPath(void) const;

  std::string const& getHeaderFieldValues(std::string const& fieldName) const;

  void setLocation(Location const& location);

  std::string const generateIndexPath(void) const;

  void print(void) const;  // debug

  void storeRequestLine(std::vector<std::string> const& result);
  void storeRequestTarget(std::string const& requestTarget);
  void storeHeaderField(std::vector<std::string> const& result);
  void storeBody(std::string const& result);
  void storeFullPath(void);

  bool isHeaderFieldNameExists(std::string const& fieldName) const;
  bool isConnectionClose(void) const;

  void clear(void);

 private:
  void setMethod(std::string const& method);
  void setPath(std::string const& path);
  void setQuery(std::string const& query);
  void setHttpVersion(std::string const& httpVersion);
  void setFullPath(std::string const& fullPath);

  EHttpMethod matchEHttpMethod(std::string method);
  void splitRequestTarget(std::string& path, std::string& query,
                          const std::string& requestTarget);

  bool isHex(char ch);
  bool isValidRequestTarget(std::string const& requestTarget);
  char hexToChar(std::string const& hexStr);
  std::string pctDecode(std::string const& str);
};

#endif
