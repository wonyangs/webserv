#ifndef __AUTOINDEX_BUILDER_HPP__
#define __AUTOINDEX_BUILDER_HPP__

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ctime>
#include <sstream>

#include "../utils/StatusException.hpp"
#include "../utils/Util.hpp"
#include "AResponseBuilder.hpp"

class AutoindexBuilder : public AResponseBuilder {
 public:
  AutoindexBuilder(Request const& request);
  AutoindexBuilder(AutoindexBuilder const& builder);
  virtual ~AutoindexBuilder(void);

  AutoindexBuilder& operator=(AutoindexBuilder const& builder);

  virtual int build(void);
  virtual void close(void);

 private:
  AutoindexBuilder(void);

  void generateAutoindexPage(std::string const& fullPath,
                             std::string const& dirPath, DIR* directory);
  virtual void buildResponseContent(std::string const& body);
};

#endif