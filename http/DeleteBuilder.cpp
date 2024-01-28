#include "DeleteBuilder.hpp"

/**
 * Constructor & Destructor
 */

DeleteBuilder::DeleteBuilder(Request const& request)
    : AResponseBuilder(STATIC, request) {}

DeleteBuilder::DeleteBuilder(DeleteBuilder const& builder)
    : AResponseBuilder(builder) {}

DeleteBuilder::~DeleteBuilder(void) { close(); }

/**
 * Operator Overloading
 */

DeleteBuilder& DeleteBuilder::operator=(DeleteBuilder const& builder) {
  if (this != &builder) {
    _response = builder._response;
    _isDone = builder._isDone;
    setRequest(builder.getRequest());
    setType(builder.getType());
  }
  return *this;
}

/**
 * Public method
 */

std::vector<int> const DeleteBuilder::build(Event::EventType type) {
  (void)type;

  if (getRequest().getMethod() != HTTP_DELETE) {
    throw StatusException(HTTP_INTERNAL_SERVER_ERROR,
                          "[] DeleteBuilder: invalid method");
  }

  char const* fullPath = getRequest().getFullPath().c_str();

  // 파일이 없는 경우 204
  if (access(fullPath, F_OK) == -1) {
    buildResponseContent("");
    _isDone = true;
    return std::vector<int>();
  }

  // 파일 삭제 권한이 없는 경우 -> 403
  if (access(fullPath, W_OK) == -1) {
    throw StatusException(HTTP_FORBIDDEN,
                          "[] DeleteBuilder: file permission error");
  }

  // 파일 삭제 성공 -> 204
  // 파일 삭제 실패 -> 500
  if (std::remove(fullPath) == 0) {
    buildResponseContent("");
  } else {
    throw StatusException(HTTP_INTERNAL_SERVER_ERROR,
                          "[] DeleteBuilder: file delete fail");
  }

  _isDone = true;

  return std::vector<int>();
}

void DeleteBuilder::close(void) {}

/**
 * Private method
 */

void DeleteBuilder::buildResponseContent(std::string const& body) {
  (void)body;
  _response.setHttpVersion("HTTP/1.1");
  _response.setStatusCode(204);

  _response.addHeader("Content-Type", "text/html");
  _response.addHeader("Content-Length", Util::itos(body.size()));

  isConnectionClose() ? _response.addHeader("Connection", "close")
                      : _response.addHeader("Connection", "keep-alive");

  _response.makeResponseContent();
}
