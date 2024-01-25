#ifndef __ERROR_BUILDER_HPP__
#define __ERROR_BUILDER_HPP__

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>

#include "../core/Kqueue.hpp"
#include "../core/Socket.hpp"
#include "../utils/StatusException.hpp"
#include "../utils/Util.hpp"
#include "AResponseBuilder.hpp"

class ErrorBuilder : public AResponseBuilder {
 private:
  int _statusCode;
  bool _recursiveFlag;

  int _fileFd;
  off_t _fileSize;
  off_t _readIndex;

  std::vector<u_int8_t> _storageBuffer;

 public:
  ErrorBuilder(void);
  ErrorBuilder(Request const& request, int statusCode);
  ErrorBuilder(ErrorBuilder const& builder);
  virtual ~ErrorBuilder(void);

  ErrorBuilder& operator=(ErrorBuilder const& builder);

  virtual bool isConnectionClose(void) const;

  virtual std::vector<int> const build(Event::EventType type);
  virtual void close(void);

 private:
  static int const BUFFER_SIZE = 1024;

  int readStatusCodeFile(Location const& location);
  void openStatusCodeFile(Location const& location);

  void generateDefaultPage(void);

  virtual void buildResponseContent(std::string const& body);
  std::string makeAllowHeaderValue(void);
};

#endif