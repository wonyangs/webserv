#!/usr/bin/env python3

import cgi
import cgitb
import os
cgitb.enable()

# CGI 스크립트의 HTTP 헤더
print("Content-Type: text/html; charset=UTF-8", end="\r\n\r\n")
# print(os.environ['REQUEST_METHOD'], os.environ['CONTENT_TYPE'], os.environ['CONTENT_LENGTH'])

form = cgi.FieldStorage()
# print(form)

# "fileUpload" 필드에서 파일 데이터 가져오기
fileitem = form['fileUpload']

# 파일 데이터가 있는지 확인하고 처리
if 'fileUpload' in form and fileitem.filename:
    # 경로를 포함하지 않는 파일 이름 설정
    fn = os.path.basename(fileitem.filename)
    file_path = os.environ["ROOT_PATH"] + os.environ["UPLOAD_PATH"] + "/" + fn
    print(file_path)

    # 파일 저장
    with open(file_path, 'wb') as f:
        f.write(fileitem.file.read())

    message = '파일 "{}" 업로드 성공'.format(fn)
else:
    message = '파일이 업로드되지 않았습니다.'

print("<html><body>")
print("<p>{}</p>".format(message))
print("</body></html>")
