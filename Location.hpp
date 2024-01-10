#ifndef __LOCATION_HPP__
#define __LOCATION_HPP__

#include <map>
#include <set>
#include <string>

#include "EMethod.h"

// config 파일의 Location 블럭
// - location 블럭의 정보를 저장한다.
class Location {
 private:
  std::string _uri;
  std::string _rootPath;
  std::string _indexFile;

  int _maxBodySize;
  std::map<int, std::string> _errorPages;
  std::set<EMethod> _allowMethods;
  bool _autoIndex;
  std::string _redirectUri;

 public:
  Location(const std::string& uri, const std::string& rootPath,
           const std::string& indexFile);
  Location(Location const& location);
  ~Location(void);

  Location& operator=(Location const& location);

  // setter
  void setMaxBodySize(int size);
  void addErrorPage(int statusNumber, const std::string& path);
  void addAllowMethod(EMethod method);
  void setAutoIndex(bool setting);
  void setRedirectUri(const std::string& path);

  // getter
  const std::string& getUri(void) const;
  const std::string& getRootPath(void) const;
  const std::string& getIndexFile(void) const;
  int getMaxBodySize(void) const;
  bool hasErrorPage(int statusCode) const;
  const std::string& getErrorPagePath(int statusCode) const;
  bool isAllowMethod(EMethod method) const;
  bool isAutoIndex(void) const;
  bool isRedirectBlock(void) const;
  const std::string& getRedirectUri(void) const;

 private:
  static const int DEFAULT_MAX_BODY_SIZE = 6000;
  static const int MIN_LIMIT_BODY_SIZE = 0;
  static const int MAX_LIMIT_BODY_SIZE = 100000000;

  bool _hasAllowMethodField;
  bool _hasRedirectField;

  Location(void);
};

#endif