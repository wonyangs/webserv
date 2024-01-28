import cgi
import cgitb
import os
import sys

def print_response(status_code, status_message, body):
    print(f"Status: {status_code} {status_message}", end='\r\n')
    if status_code == 400 or status_code == 500:
        print("Connection: close", end='\r\n')
    else:
        print("Connection: keep-alive", end='\r\n')
    print("Content-Type: text/plain", end="\r\n\r\n")
    print(body, end='\r\n')
    sys.exit()

cgitb.enable()

# "fileUpload" 필드에서 파일 데이터 가져오기
form = cgi.FieldStorage()
if 'fileUpload' in form:
    fileitem = form['fileUpload']
    if fileitem.filename:
        fn = os.path.basename(fileitem.filename)  # 디렉토리 부분 제거, 파일 이름만 추출
        file_path = os.environ["UPLOAD_PATH"].rstrip('/') + "/" + fn
    else:
        print_response(400, "Bad Request", "No file uploaded.")
else:
    print_response(400, "Bad Request", "Invalid field value.")

# HTTP 요청 메서드 가져오기
request_method = os.environ.get("REQUEST_METHOD", "GET")

# 파일 생성 (POST 요청)
if request_method == "POST":
    if os.path.exists(file_path):
        print_response(409, "Conflict", "File already exists.")
    else:
        try:
            with open(file_path, 'wb') as f:
                f.write(fileitem.file.read())
            print_response(201, "Created", "File created.")
        except OSError as e:
            print_response(500, "Internal Server Error", "Error creating file")

else:
    print_response(400, "Bad Request", "Unsupported method.")
