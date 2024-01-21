#ifndef __ERROR_BUILDER_HPP__
#define __ERROR_BUILDER_HPP__

#include "../utils/Util.hpp"
#include "AResponseBuilder.hpp"

class ErrorBuilder : public AResponseBuilder {
 private:
  int _statusCode;

 public:
  ErrorBuilder(void);
  ErrorBuilder(Request const& request, int statusCode);
  ErrorBuilder(ErrorBuilder const& builder);
  virtual ~ErrorBuilder(void);

  ErrorBuilder& operator=(ErrorBuilder const& builder);

  virtual void build(void);
  virtual void close(void);

 private:
  void readStatusCodeFile(void);
  void generateDefaultPage(void);
};

#endif