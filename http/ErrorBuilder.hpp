#ifndef __ERROR_BUILDER_HPP__
#define __ERROR_BUILDER_HPP__

#include "AResponseBuilder.hpp"

class ErrorBuilder : public AResponseBuilder {
 private:
  int _statusCode;

 public:
  ErrorBuilder(Request const& request, int statusCode);
  ErrorBuilder(ErrorBuilder const& builder);
  ~ErrorBuilder(void);

  ErrorBuilder& operator=(ErrorBuilder const& builder);

  virtual void build(void);
  virtual void close(void);

 private:
  ErrorBuilder(void);

  void readStatusCodeFile(void);
  void generateDefaultPage(void);
};

#endif