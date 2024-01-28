#ifndef __DELETE_BUILDER_HPP__
#define __DELETE_BUILDER_HPP__

#include <unistd.h>

#include <cstdio>
#include <string>

#include "../utils/StatusException.hpp"
#include "AResponseBuilder.hpp"

class DeleteBuilder : public AResponseBuilder {
 public:
  DeleteBuilder(Request const& request);
  DeleteBuilder(DeleteBuilder const& builder);
  ~DeleteBuilder(void);

  DeleteBuilder& operator=(DeleteBuilder const& builder);

  virtual std::vector<int> const build(Event::EventType type);
  virtual void close(void);

 private:
  virtual void buildResponseContent(std::string const& body);

  DeleteBuilder(void);
};

#endif