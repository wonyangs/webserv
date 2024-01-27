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

  std::vector<octet_t> _storageBuffer;

 public:
  StaticFileBuilder(Request const& request);
  StaticFileBuilder(StaticFileBuilder const& builder);
  virtual ~StaticFileBuilder(void);

  StaticFileBuilder& operator=(StaticFileBuilder const& builder);

  virtual std::vector<int> const build(Event::EventType type);
  virtual void close(void);

 private:
  static int const BUFFER_SIZE = 10000;

  void openStaticFile(void);
  void readStaticFile(void);

  virtual void buildResponseContent(std::string const& body);

  StaticFileBuilder(void);
};

#endif