#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

// 서버가 사용하는 설정들을 정의해 둔 static 클래스
class Config {
 public:
  // 서버가 처리할 수 있는 최소 body 크기
  static const int MIN_LIMIT_BODY_SIZE = 0;
  // 서버가 처리할 수 있는 최대 body 크기
  static const int MAX_LIMIT_BODY_SIZE = 100000000;

 private:
  Config(void);
  Config(Config const& config);
  ~Config(void);
  Config& operator=(Config const& config);
};

#endif