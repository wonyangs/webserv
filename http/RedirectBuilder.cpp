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

std::vector<int> const RedirectBuilder::build(Event::EventType type) {
  (void)type;
  generateRedirectPage();
  return std::vector<int>();
}

void RedirectBuilder::close(void) {}

void RedirectBuilder::generateRedirectPage(void) {
  std::stringstream ss;

  ss << "<!DOCTYPE html> <html> <head> <title> REDIRECT </title> </head> "
        "<body> <div class= \"container \"> <h1> 301 Moved Permanently </h1>";

  ss << "<p>The document has moved <a href=\"" << _redirectUri
     << "\"> here</a>.</p> </body> </html>";

  std::string const body = ss.str();
  buildResponseContent(body);
  _isDone = true;
}

// body 정보를 받아 response 제작
void RedirectBuilder::buildResponseContent(std::string const& body) {
  _response.setHttpVersion("HTTP/1.1");
  _response.setStatusCode(301);

  _response.addHeader("Content-Type", "text/html");
  _response.addHeader("Content-Length", Util::itos(body.size()));
  _response.addHeader("Location", _redirectUri);

  _response.addHeader("Connection", "keep-alive");

  _response.appendBody(body);

  _response.makeResponseContent();
}
