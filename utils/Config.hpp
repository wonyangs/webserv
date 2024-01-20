#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <map>
#include <sstream>
#include <string>

// 서버가 사용하는 설정들을 정의해 둔 static 클래스
class Config {
 public:
  // 서버가 처리할 수 있는 최소 body 크기
  static const int MIN_LIMIT_BODY_SIZE = 0;
  // 서버가 처리할 수 있는 최대 body 크기
  static const int MAX_LIMIT_BODY_SIZE = 100000000;
  // 서버가 처리할 수 있는 최대 URI 크기
  static const int MAX_URI_SIZE = 8000;

  // 상태코드 메시지
  static const std::map<int, std::string> statusMessages;

  static std::string const defaultErrorPageBody(int code,
                                                std::string const& message);

 private:
  static std::map<int, std::string> initializeStatusMessages(void);

  Config(void);
  Config(Config const& config);
  ~Config(void);
  Config& operator=(Config const& config);
};

#endif