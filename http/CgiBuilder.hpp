#ifndef __CGI_BUILDER_HPP__
#define __CGI_BUILDER_HPP__

#include <signal.h>
#include <unistd.h>

#include <map>
#include <stdexcept>
#include <string>

#include "../config/Location.hpp";
#include "../core/Kqueue.hpp"
#include "AResponseBuilder.hpp"

class CgiBuilder : public AResponseBuilder {
 private:
  pid_t _pid;
  int _readPipeFd;
  int _writePipeFd;

  off_t _writeIndex;
  std::vector<u_int8_t> _storageBuffer;

 public:
  CgiBuilder(CgiBuilder const& builder);
  virtual ~CgiBuilder(void);

  CgiBuilder& operator=(CgiBuilder const& builder);

  virtual std::vector<int> const build(Event::EventType type);
  virtual void close(void);

 private:
  static int const BUFFER_SIZE = 1024;

  std::vector<int> const forkCgi(void);
  void parentProcess(int* const p_to_c, int* const c_to_p);
  void childProcess(int* const p_to_c, int* const c_to_p);
  void makeEnv(void);

  void handleReadEvent(void);
  void handleWriteEvent(void);

  virtual void buildResponseContent(std::string const& body);

  char** makeEnv(void);
  char** createEnvArray(std::map<std::string, std::string> const& env);
  void freeEnvArray(char** envp);

  CgiBuilder(void);
};

#endif