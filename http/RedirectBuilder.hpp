#ifndef __REDIRECT_BUILDER_HPP__
#define __REDIRECT_BUILDER_HPP__

#include <sstream>

#include "../utils/StatusException.hpp"
#include "AResponseBuilder.hpp"

class RedirectBuilder : public AResponseBuilder {
 private:
  std::string _redirectUri;

 public:
  RedirectBuilder(Request const& request, std::string const& redirectUri);
  RedirectBuilder(RedirectBuilder const& builder);
  virtual ~RedirectBuilder(void);

  RedirectBuilder& operator=(RedirectBuilder const& builder);

  virtual std::vector<int> const build(Event::EventType type);
  virtual void close(void);

 private:
  RedirectBuilder(void);

  void generateRedirectPage(void);
  virtual void buildResponseContent(std::string const& body);
};

#endif