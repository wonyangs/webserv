require 'cgi'

cgi = CGI.new

# HTTP 헤더 출력
puts "Content-Type: text/html\r\n\r\n"

# HTML 본문 출력
puts "<html>"
puts "<head>"
puts "<title>Hello World</title>"
puts "</head>"
puts "<body>"
puts "<h1>Hello World</h1>"
puts "</body>"
puts "</html>"

