import cgi
import cgitb
import os
import sys

cgitb.enable()

# "fileUpload" 필드에서 파일 데이터 가져오기
form = cgi.FieldStorage()

if 'fileUpload' in form and fileitem.filename:
    fileitem = form['fileUpload']
    fn = os.path.basename(fileitem.filename) # 디렉토리 부분 제거, 파일 이름만 추출
    file_path = os.environ["UPLOAD_PATH"].rstrip('/') + "/" + fn
else:
    print("Status: 400 Bad Request")  # 잘못된 형식
    print("Connection: close")
    print("Content-Type: text/plain", end="\r\n\r\n")
    print("Invalid field value.")
    return


# HTTP 요청 메서드 가져오기
request_method = os.environ.get("REQUEST_METHOD", "GET")

# 파일 생성 (POST 요청)
if request_method == "POST":
    if os.path.exists(file_path):
        print("Status: 409 Conflict")  # 파일이 이미 존재하는 경우
        print("Connection: keep-alive")
        print("Content-Type: text/plain", end="\r\n\r\n")
        print("File already exists.")
    else:
        try:
            with open(file_path, 'wb') as f:
                f.write(fileitem.file.read())
            print("Status: 201 Created")  # 파일 업로드 성공
            print("Connection: keep-alive")
            print("Content-Type: text/plain", end="\r\n\r\n")
            print("File created.")
        except OSError as e:
            print("Status: 500 Internal Server Error")  # 파일 생성 실패
            print("Connection: close")
            print("Content-Type: text/plain", end="\r\n\r\n")
            print(f"Error creating file: {e}")

else:
    print("Status: 400 Bad Request")  # 지원하지 않는 메서드
    print("Connection: close")
    print("Content-Type: text/plain", end="\r\n\r\n")
    print("Unsupported method.")
