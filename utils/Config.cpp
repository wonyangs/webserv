#include "Config.hpp"

const std::map<int, std::string> Config::statusMessages =
    Config::initializeStatusMessages();

const std::map<std::string, std::string> Config::mimeTypes =
    Config::initializeMimeTypes();

std::map<int, std::string> Config::initializeStatusMessages(void) {
  std::map<int, std::string> messages;
  messages[200] = "OK";
  messages[201] = "Created";
  messages[204] = "No Content";
  messages[206] = "Partial Content";
  messages[300] = "Multiple Choices";
  messages[301] = "Moved Permanently";
  messages[302] = "Found";
  messages[304] = "Not Modified";
  messages[307] = "Temporary Redirect";
  messages[400] = "Bad Request";
  messages[401] = "Unauthorized";
  messages[403] = "Forbidden";
  messages[404] = "Not Found";
  messages[405] = "Method Not Allowed";
  messages[406] = "Not Acceptable";
  messages[408] = "Request Timeout";
  messages[409] = "Conflict";
  messages[410] = "Gone";
  messages[413] = "Payload Too Large";
  messages[414] = "URI Too Long";
  messages[415] = "Unsupported Media Type";
  messages[429] = "Too Many Requests";
  messages[500] = "Internal Server Error";
  messages[501] = "Not Implemented";
  messages[502] = "Bad Gateway";
  messages[503] = "Service Unavailable";
  messages[504] = "Gateway Timeout";
  return messages;
}

std::map<std::string, std::string> Config::initializeMimeTypes() {
  std::map<std::string, std::string> m;
  m[".html"] = "text/html";
  m[".htm"] = "text/html";
  m[".css"] = "text/css";
  m[".js"] = "text/javascript";
  m[".json"] = "application/json";
  m[".xml"] = "application/xml";
  m[".txt"] = "text/plain";
  m[".md"] = "text/markdown";
  m[".csv"] = "text/csv";
  m[".jpg"] = "image/jpeg";
  m[".jpeg"] = "image/jpeg";
  m[".png"] = "image/png";
  m[".gif"] = "image/gif";
  m[".bmp"] = "image/bmp";
  m[".svg"] = "image/svg+xml";
  m[".ico"] = "image/x-icon";
  m[".webp"] = "image/webp";
  m[".pdf"] = "application/pdf";
  m[".zip"] = "application/zip";
  m[".rar"] = "application/vnd.rar";
  m[".7z"] = "application/x-7z-compressed";
  m[".tar"] = "application/x-tar";
  m[".gz"] = "application/gzip";
  m[".mp3"] = "audio/mpeg";
  m[".wav"] = "audio/wav";
  m[".mp4"] = "video/mp4";
  m[".avi"] = "video/x-msvideo";
  m[".mov"] = "video/quicktime";
  m[".wmv"] = "video/x-ms-wmv";
  m[".flv"] = "video/x-flv";
  m[".webm"] = "video/webm";
  m[".m4a"] = "audio/mp4";
  m[".mpg"] = "video/mpeg";
  m[".mpeg"] = "video/mpeg";
  m[".ogg"] = "audio/ogg";
  m[".ogv"] = "video/ogg";
  return m;
}

// 상태 코드에 해당하는 메시지 찾기
// - 정의되지 않은 코드일 경우 예외 발생
std::string const& Config::findStatusMessage(int code) {
  std::map<int, std::string>::const_iterator it =
      Config::statusMessages.find(code);

  if (it == Config::statusMessages.end()) {
    throw std::runtime_error(
        "[6000] Config: findStatusMessage - status message does not exist: " +
        Util::itos(code));
  }

  return it->second;
}

std::string const Config::defaultErrorPageBody(int code) {
  std::stringstream ss;
  std::string const& message = Config::findStatusMessage(code);

  ss << "<!DOCTYPE html> <html> <head> <title>ERROR</title> <style> "
        "@font-face { font-family: 'WarhavenB'; src: "
        "url('https://cdn.jsdelivr.net/gh/projectnoonnu/noonfonts_2312-1@1.1/"
        "WarhavenB.woff2') format('woff2'); font-weight: 700; font-style: "
        "normal; } body {font-family: 'WarhavenB', Arial, sans-serif; "
        "background-color: #d7ddcd; margin: 0; padding: 0; display: flex; "
        "align-items: center; text-align: center; } .container { margin: "
        "50px auto; animation: shake 0.5s cubic-bezier(.36,.07,.19,.97) both; "
        "transform: translate3d(0, 0, 0); backface-visibility: hidden; "
        "perspective: 1000px; } .error { font-size: 100px; color: #313438bd; "
        "} "
        "@keyframes shake { 10%, 90% { transform: translate3d(-1px, 0, 0);} "
        "20%, 80% { transform: translate3d(2px, 0, 0); } 30%, 50%, 70% {  "
        "transform: translate3d(-4px, 0, 0); } 40%, 60% {  transform: "
        "translate3d(4px, 0, 0);}} </style> </head> <body> <div class= "
        "\"container \"> <h1 class=\"error\">";

  ss << code << " " << message << "</h1>";
  ss << " <img src=\" https://http.cat/" << code
     << "\"alt=\"Error Image\" class=\"error-image\"> </div> </body>\n\n";

  return ss.str();
}

// path에 포함된 확장자에 맞는 MIME 타입 반환
// - 알 수 없는 확장자의 경우 기본 설정된 MIME 타입 반환
std::string const Config::findMimeType(const std::string& path) {
  // 마지막 '.' 위치를 찾음
  size_t dotPos = path.find_last_of('.');
  std::string extension;

  // 확장자가 없는 경우 기본 MIME 타입 반환
  if (dotPos == std::string::npos) return "application/octet-stream";

  // 확장자 추출 (마지막 '.' 이후의 모든 문자)
  extension = path.substr(dotPos);

  // 확장자에 맞는 MIME 타입 반환
  std::map<std::string, std::string>::const_iterator it =
      mimeTypes.find(extension);
  if (it != mimeTypes.end()) {
    return it->second;
  }
  return "application/octet-stream";
}
