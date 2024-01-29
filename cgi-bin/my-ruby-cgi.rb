#!/usr/bin/env ruby

require 'cgi'
require 'open3'

cgi = CGI.new

def print_response(status_code, status_message)
  puts "Status: #{status_code} #{status_message}\r\n"
  puts "Connection: close" if status_code == 500
  puts "Content-type: text/plain\r\n\r\n"
end

begin
  # PATH_INFO 환경 변수 가져오기
  path_info = ENV['PATH_INFO']

  if path_info.nil? || path_info.empty?
    print_response(500, "CGI Invalid PATH_INFO")
    exit
  end

  # 파일 존재 여부 확인
  unless File.file?(path_info)
    print_response(404, "CGI PATH_INFO Not Found")
    exit
  end

  # 파일 실행
  stdout, stderr, status = Open3.capture3("/usr/bin/ruby", path_info)

  if status.success?
    # 성공적으로 실행된 경우, 결과 출력
    puts stdout
  else
    # 오류 발생 시
    print_response(500, "CGI Execute Error")
    puts stderr
  end
rescue => e
  print_response(500, "CGI Internal Server Error")
  puts e.message
end

