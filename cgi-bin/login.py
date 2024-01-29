import cgi
import json
import os
import uuid
from http import cookies

# 세션 데이터 파일 경로
SESSION_FILE = 'cgi-bin/sessions.json'

# 세션 데이터를 불러오는 함수
def load_sessions():
    if os.path.exists(SESSION_FILE):
        with open(SESSION_FILE, 'r') as file:
            return json.load(file)
    return {}

# 세션 데이터를 저장하는 함수
def save_sessions(sessions):
    with open(SESSION_FILE, 'w') as file:
        json.dump(sessions, file)

# 새 세션을 생성하고 저장하는 함수
def create_session(username):
    sessions = load_sessions()
    session_id = str(uuid.uuid4())
    sessions[session_id] = username
    save_sessions(sessions)
    return session_id

# 현재 세션 ID를 확인하고, 필요한 경우 새로운 세션을 생성하는 함수
def get_or_create_session(username):
    # 쿠키 정보를 불러옵니다
    if 'HTTP_COOKIE' in os.environ:
        cookie = cookies.SimpleCookie(os.environ['HTTP_COOKIE'])
        if 'session_id' in cookie:
            session_id = cookie['session_id'].value
            sessions = load_sessions()
            if session_id in sessions:
                return session_id

    # 새로운 세션 생성
    return create_session(username)

def main():
    form = cgi.FieldStorage()
    username = form.getvalue('username')
    password = form.getvalue('password')  # 비밀번호 검증은 별도로 구현 필요

    session_id = get_or_create_session(username)

    print("Status: 302 Found")
    print(f"Set-Cookie: session_id={session_id}; Path=/; HttpOnly")
    print("Content-type: text/html")
    print("Location: /upload.html\n")
    print("<html>")
    print("<head>")
    print("<title>Redirecting...</title>")
    print("</head>")
    print("<body>")
    print("Redirecting to upload page...")
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()
