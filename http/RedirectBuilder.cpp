#include "RedirectBuilder.hpp"

/**
 * Constructor & Destructor
 */

RedirectBuilder::RedirectBuilder(Request const& request,
                                 std::string const& redirectUri)
    : AResponseBuilder(REDIRECT, request), _redirectUri(redirectUri) {}

RedirectBuilder::RedirectBuilder(RedirectBuilder const& builder)
    : AResponseBuilder(builder), _redirectUri(builder._redirectUri) {}

RedirectBuilder::~RedirectBuilder(void) {}

/**
 * Operator Overloading
 */

RedirectBuilder& RedirectBuilder::operator=(RedirectBuilder const& builder) {
  if (this != &builder) {
    _response = builder._response;
    _isDone = builder._isDone;
    setRequest(builder.getRequest());
    setType(builder.getType());
    _redirectUri = builder._redirectUri;
  }
  return *this;
}

/**
 * Public method
 */

int RedirectBuilder::build(void) {}

void RedirectBuilder::close(void) {}

void RedirectBuilder::generateRedirectPage(void) {}
void RedirectBuilder::buildResponseContent(std::string const& body) {}
