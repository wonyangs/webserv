#include "AutoindexBuilder.hpp"

/**
 * Constructor & Destructor
 */

AutoindexBuilder::AutoindexBuilder(Request const& request)
    : AResponseBuilder(AUTOINDEX, request) {}

AutoindexBuilder::AutoindexBuilder(AutoindexBuilder const& builder)
    : AResponseBuilder(builder) {}

AutoindexBuilder::~AutoindexBuilder(void) {}

/**
 * Operator Overloading
 */

AutoindexBuilder& AutoindexBuilder::operator=(AutoindexBuilder const& builder) {
  if (this != &builder) {
    _response = builder._response;
    _isDone = builder._isDone;
    setRequest(builder.getRequest());
    setType(builder.getType());
  }
  return *this;
}

/**
 * Public method
 */

// autoindex page 제작이 가능한지 확인 후 가능하다면 제작
// - 가능한 경우가 아니라면 예외 발생
int AutoindexBuilder::build(void) {
  Request const& request = getRequest();
  EHttpMethod const& method = request.getMethod();

  if (method != HTTP_GET and method != HTTP_POST) {
    throw StatusException(
        HTTP_NOT_ALLOWED,
        "[5200] AutoindexBuilder: build - http method not allowed");
  }

  Location const& location = request.getLocation();
  std::string const& path = request.getPath();
  std::string const& fullPath = request.getFullPath();

  if (access(fullPath.c_str(), F_OK) == -1) {
    throw StatusException(
        HTTP_NOT_FOUND,
        "[5201] AutoindexBuilder: build - file not exist: " + fullPath);
  }

  if (access(fullPath.c_str(), R_OK) == -1) {
    throw StatusException(
        HTTP_INTERNAL_SERVER_ERROR,
        "[5202] AutoindexBuilder: build - file permissions denied: " +
            fullPath);
  }

  if (method == HTTP_POST or location.isAutoIndex() == false) {
    throw StatusException(
        HTTP_NOT_FOUND,
        "[5203] AutoindexBuilder: build - autoindex is not found");
  }

  DIR* directory;
  if ((directory = opendir(fullPath.c_str())) == NULL)
    throw std::runtime_error("[5204] AutoindexBuilder: build - opendir failed");

  generateAutoindexPage(fullPath, path, directory);

  if (closedir(directory) == -1) {
    throw std::runtime_error(
        "[5205] AutoindexBuilder: build - closedir failed");
  }
  return -1;
}

void AutoindexBuilder::close(void) {}

/**
 * Private method
 */

// autoindex page 제작
void AutoindexBuilder::generateAutoindexPage(std::string const& fullPath,
                                             std::string const& dirPath,
                                             DIR* directory) {
  struct dirent* entry;

  std::stringstream ss;

  ss << "<html> <head> <title>Index of " << dirPath
     << "</title> <style> table { border - collapse : collapse; }"
        "th, td { border: 1px solid black; padding: 10px; } </style> </head>";
  ss << "<body> <h1>Index of " << dirPath << "</h1>\n";
  ss << "<table> <tr> <th>filename</th> <th>last modified time</th> "
        "<th>size</th> </tr> ";

  struct stat fileStat;
  while ((entry = readdir(directory)) != NULL) {
    std::string fileName(entry->d_name);
    std::string const& filePath = fullPath + fileName;
    bool isDir = (entry->d_type == DT_DIR);

    if (fileName == ".") continue;

    if (stat(filePath.c_str(), &fileStat) == -1) {
      throw std::runtime_error(
          "[5206] AutoindexBuilder: generateAutoindexPage - stat failed: " +
          filePath);
    }

    if (isDir) fileName += "/";

    ss << "<tr> ";
    ss << "<td> <a href=\"" << fileName << "\">" << fileName << "</a> </td>";
    ss << "<td> " << std::ctime(&fileStat.st_mtime) << "</td>";
    if (!isDir)
      ss << "<td> " << fileStat.st_size << "</td>";
    else
      ss << "<td> - </td>";
    ss << "</tr>";
  }

  ss << "</table> </body> <html>";

  buildResponseContent(ss.str());
}

// Connection은 request Header 정보 보고 변경되어야 함
void AutoindexBuilder::buildResponseContent(std::string const& body) {
  // HTTP 응답 생성
  _response.setHttpVersion("HTTP/1.1");
  _response.setStatusCode(200);

  _response.addHeader("Content-Type", "text/html");
  _response.addHeader("Content-Length", Util::itos(body.size()));
  _response.addHeader("Connection", "keep-alive");

  _response.appendBody(body);

  _response.makeResponseContent();

  _isDone = true;
}
