#include "StatusException.hpp"

#include <sstream>

// Constructor & Destructor

StatusException::StatusException(EStatusCode statusCode,
                                 std::string const& message)
    : _statusCode(statusCode), _message(message) {
  std::ostringstream oss;
  oss << "(" << _statusCode << ") " << _message;
  _what = oss.str();
}

StatusException::~StatusException(void) throw() {}

// Public method

char const* StatusException::what() const throw() { return _what.c_str(); }

EStatusCode StatusException::getStatusCode(void) const { return _statusCode; }
