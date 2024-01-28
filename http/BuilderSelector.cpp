#include "BuilderSelector.hpp"

// 요청에 알맞는 response builder 선택
// - AResponseBuilder를 상속받은 객체를 동적할당한 포인터 반환
AResponseBuilder* BuilderSelector::getMatchingBuilder(Request const& request) {
  Location const& location = request.getLocation();
  std::string fullPath = request.getFullPath();

  if (location.isRedirectBlock())
    return new RedirectBuilder(request, location.getRedirectUri());

  if (BuilderSelector::isAutoindexBuilderSelected(request))
    return new AutoindexBuilder(request);

  if (fullPath.back() == '/') fullPath = request.generateIndexPath();

  if (BuilderSelector::isCgiBuilderSelected(request, fullPath))
    return new CgiBuilder(request);

  if (BuilderSelector::isDirRedirectSelected(fullPath))
    return new RedirectBuilder(request, request.getPath() + '/');

  if (request.getMethod() == HTTP_DELETE) return new DeleteBuilder(request);

  return new StaticFileBuilder(request);
}

// 디렉토리 경로(/로 끝나는 path)인 경우
// - GET일 때 index 파일이 존재하지 않을 때
// - POST, DELETE의 경우
bool BuilderSelector::isAutoindexBuilderSelected(Request const& request) {
  std::string fullPath = request.getFullPath();

  if (fullPath.back() != '/') return false;

  if (request.getMethod() != HTTP_GET) return true;

  fullPath = request.generateIndexPath();
  if (access(fullPath.c_str(), F_OK) == -1) return true;

  return false;
}

// cgi 확장자 파일 또는 POST인 경우
bool BuilderSelector::isCgiBuilderSelected(Request const& request,
                                           std::string const& fullPath) {
  Location const& location = request.getLocation();

  if (request.getMethod() == HTTP_POST) return true;

  if (location.hasCgiInfo() and
      Config::findFileExtension(fullPath) == location.getCgiExtention())
    return true;

  return false;
}

// 해당 파일이 디렉토리 경로이지만 /가 붙어있지 않은 경우
bool BuilderSelector::isDirRedirectSelected(std::string const& fullPath) {
  if (access(fullPath.c_str(), F_OK) == -1) return false;

  // 파일 정보 확인
  struct stat statbuf;
  if (stat(fullPath.c_str(), &statbuf) == -1) {
    throw std::runtime_error(
        "[4004] BuilderSelector: isDirRedirectSelected - stat failed: " +
        fullPath);
  }

  return S_ISDIR(statbuf.st_mode);
}