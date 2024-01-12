#ifndef __LOCATION_HPP__
#define __LOCATION_HPP__

#include <map>
#include <set>
#include <string>

#include "../utils/Config.hpp"
#include "../utils/Enum.hpp"

// config 파일의 Location 블럭
// - location 블럭의 정보를 저장한다.
class Location {
 private:
  std::string _uri;
  std::string _rootPath;
  std::string _indexFile;

  int _maxBodySize;
  std::map<int, std::string> _errorPages;
  std::set<EHttpMethod> _allowMethods;
  bool _autoIndex;
  std::string _redirectUri;

 public:
  Location(std::string const& uri, std::string const& rootPath,
           std::string const& indexFile);
  Location(Location const& location);
  ~Location(void);

  Location& operator=(Location const& location);

  // setter
  void setMaxBodySize(int size);
  void addErrorPage(int statusNumber, std::string const& path);
  void addAllowMethod(EHttpMethod method);
  void setAutoIndex(bool setting);
  void setRedirectUri(std::string const& path);

  // getter
  std::string const& getUri(void) const;
  std::string const& getRootPath(void) const;
  std::string const& getIndexFile(void) const;
  int getMaxBodySize(void) const;
  bool hasErrorPage(int statusCode) const;
  std::string const& getErrorPagePath(int statusCode) const;
  bool isAllowMethod(EHttpMethod method) const;
  bool isAutoIndex(void) const;
  bool isRedirectBlock(void) const;
  std::string const& getRedirectUri(void) const;

 private:
  static int const DEFAULT_MAX_BODY_SIZE = 6000;

  bool _hasAllowMethodField;
  bool _hasRedirectField;

  Location(void);
};

#endif