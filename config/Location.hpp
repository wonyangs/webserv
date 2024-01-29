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
  static int const DEFAULT_MAX_BODY_SIZE = 100000000;

  std::string _projectRootPath;

  std::string _uri;
  std::string _rootPath;
  std::string _indexFile;

  int _maxBodySize;
  std::map<int, std::string> _errorPages;
  std::set<EHttpMethod> _allowMethods;
  bool _autoIndex;
  std::string _redirectUri;

  bool _cgiFlag;
  std::string _cgiExtention;
  std::string _cgiPath;
  std::string _uploadPath;

  bool _hasAllowMethodField;
  bool _hasRedirectField;

 public:
  void printConfiguration();  // debug

  Location(void);
  Location(std::string const& projectRootPath);
  Location(std::string const& uri, std::string const& rootPath,
           std::string const& indexFile);
  Location(Location const& location);
  ~Location(void);

  Location& operator=(Location const& location);

  // setter
  void setUri(std::string const& uri);
  void setRootPath(std::string const& rootPath);
  void setIndexFile(std::string const& indexFile);
  void setMaxBodySize(int size);
  void addErrorPage(int statusNumber, std::string const& path);
  void addAllowMethod(std::string methodString);
  void setAutoIndex(std::string const&);
  void setRedirectUri(std::string const& path);
  void setCgiExtention(std::string const& extention);
  void setCgiPath(std::string const& cgiPath);
  void setUploadDir(std::string const& dirPath);

  // getter
  std::string const& getProjectRootPath(void) const;
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
  bool hasCgiInfo(void) const;
  std::string const& getCgiExtention(void) const;
  std::string const& getCgiPath(void) const;
  std::string const& getUploadDirPath(void) const;

  bool isRequiredValuesSet(void) const;

 private:
  std::string getFullPath(std::string const& path);
  std::string convertPath(std::string const& path);
};

#endif