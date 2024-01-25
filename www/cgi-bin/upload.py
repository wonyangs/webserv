#!/usr/bin/env python3

import sys
import cgi
import cgitb
import os
# try:
#     from StringIO import StringIO as IO
# except ImportError:
from io import BytesIO as IO

# os.environ['REQUEST_METHOD'] = 'POST'
# os.environ['CONTENT_TYPE'] = 'multipart/form-data; boundary=spam'



cgitb.enable()

# CGI 스크립트의 HTTP 헤더
print("Content-Type: text/html; charset=UTF-8", end="\r\n\r\n")
# print("upload path: " + os.environ["ROOT_PATH"] + os.environ["UPLOAD_PATH"])

print(os.environ['REQUEST_METHOD'], os.environ['CONTENT_TYPE'], os.environ['CONTENT_LENGTH'])

# # 그대로 출력

# body = """--spam
# Content-Disposition: form-data; name="param1"; filename=blob
# Content-type: binary/octet-stream

# value1
# --spam--
# """

# content_bytes = body.encode('utf-8')
# os.environ['CONTENT_LENGTH'] = str(len(content_bytes))
# print(data)

# form = cgi.FieldStorage(environ=os.environ)
form = cgi.FieldStorage()

# parsed = cgi.FieldStorage(
#     # IO(body.encode('utf-8')),
#     headers={'content-type': 'multipart/form-data; boundary=spam;','content-length': len(body)},
#     environ={'REQUEST_METHOD': 'POST'})

# print(body)
# print(parsed)

# 폼 데이터를 파싱합니다.
# form = cgi.FieldStorage(environ=os.environ)
print(form)
# print(form['name'])

# 모든 필드와 파일을 반복하여 출력
# for key in form.keys():
#     item = form[key]
#     print("필드 이름:", key, end='\r\n')
#     print("값:", item.value, end='\r\n')
        



# "fileUpload" 필드에서 파일 데이터 가져오기
fileitem = form['fileUpload']

# 파일 데이터가 있는지 확인하고 처리
if 'fileUpload' in form and fileitem.filename:
    # 경로를 포함하지 않는 파일 이름 설정
    fn = os.path.basename(fileitem.filename)
    file_path = os.environ["ROOT_PATH"] + os.environ["UPLOAD_PATH"] + fn

    # 파일 저장
    with open(file_path, 'wb') as f:
        f.write(fileitem.file.read())

    message = '파일 "{}" 업로드 성공'.format(fn)
else:
    message = '파일이 업로드되지 않았습니다.'

# print("<html><body>")
# print("<p>{}</p>".format(message))
# print("</body></html>")
