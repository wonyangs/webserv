#ifndef __STATIC_FILE_BUILDER_HPP__
#define __STATIC_FILE_BUILDER_HPP__

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../core/Kqueue.hpp"
#include "../core/Socket.hpp"
#include "../utils/Config.hpp"
#include "../utils/StatusException.hpp"
#include "../utils/Util.hpp"
#include "AResponseBuilder.hpp"

class StaticFileBuilder : public AResponseBuilder {
 private:
  int _fileFd;
  off_t _fileSize;
  off_t _readIndex;

  std::vector<u_int8_t> _storageBuffer;

 public:
  StaticFileBuilder(Request const& request);
  StaticFileBuilder(StaticFileBuilder const& builder);
  virtual ~StaticFileBuilder(void);

  StaticFileBuilder& operator=(StaticFileBuilder const& builder);

  virtual int build(void);
  virtual void close(void);

 private:
  static int const BUFFER_SIZE = 1024;

  void openStaticFile(void);
  void readStaticFile(void);
  std::string const makeFullPath(void);

  virtual void buildResponseContent(std::string const& body);

  StaticFileBuilder(void);
};

#endif