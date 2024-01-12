#ifndef __STATUS_EXCEPTION_HPP__
#define __STATUS_EXCEPTION_HPP__

#include <exception>
#include <string>

#include "Enum.hpp"

// 상태코드 예외 클래스
class StatusException : public std::exception {
 private:
  EStatusCode _statusCode;
  std::string _message;
  std::string _what;

 public:
  StatusException(EStatusCode statusCode, std::string const& message);
  ~StatusException(void) throw();

  char const* what() const throw();

  EStatusCode getStatusCode(void) const;
};

#endif
