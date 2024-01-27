#!/usr/bin/python3

import cgi
import cgitb
cgitb.enable()

# CGI 스크립트의 HTTP 헤더
print("Content-Type: text/html; charset=UTF-8")
print()

# 폼 데이터를 파싱합니다.
form = cgi.FieldStorage()
name = form.getvalue('name')
email = form.getvalue('email')
password = form.getvalue('password')

# 간단한 응답 메시지
print("<html><body>")
print("<h2>환영합니다, {}!</h2>".format(name))
print("<p>당신의 이메일 주소는 {}입니다.</p>".format(email))
print("<p>당신의 비밀번호는 안전하게 저장되었습니다. (실제로는 저장하지 않음)</p>")
print("</body></html>")
