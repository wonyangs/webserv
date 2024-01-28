#ifndef __BUILDER_SELECTOR_HPP__
#define __BUILDER_SELECTOR_HPP__

#include <sys/stat.h>
#include <unistd.h>

#include <stdexcept>

#include "../http/AResponseBuilder.hpp"
#include "../http/AutoindexBuilder.hpp"
#include "../http/CgiBuilder.hpp"
#include "../http/DeleteBuilder.hpp"
#include "../http/ErrorBuilder.hpp"
#include "../http/RedirectBuilder.hpp"
#include "../http/Request.hpp"
#include "../http/StaticFileBuilder.hpp"

// 적절한 ResponseBuilder를 선택
class BuilderSelector {
 public:
  static AResponseBuilder* getMatchingBuilder(Request const& request);

 private:
  BuilderSelector(void);
  BuilderSelector(BuilderSelector const& BuilderSelector);
  ~BuilderSelector(void);
  BuilderSelector& operator=(BuilderSelector const& BuilderSelector);

  static bool isAutoindexBuilderSelected(Request const& request);
  static bool isCgiBuilderSelected(Request const& request,
                                   std::string const& fullPath);
  static bool isDirRedirectSelected(std::string const& fullPath);
};

#endif