import os
import json
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

# 세션 삭제 함수
def delete_session(session_id):
    sessions = load_sessions()
    if session_id in sessions:
        del sessions[session_id]
        save_sessions(sessions)

def main():
    # 쿠키 정보를 불러와 현재 세션 ID를 확인
    cookie = cookies.SimpleCookie(os.environ.get('HTTP_COOKIE'))
    session_id = cookie['session_id'].value if 'session_id' in cookie else None

    if session_id:
        delete_session(session_id)

    # 사용자 쿠키에서 세션 정보를 삭제
    print("Status: 302 Found")
    print("Set-Cookie: session_id=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; HttpOnly")
    print("Content-type: text/html")
    print("Location: /\n")
    print("<html>")
    print("<head>")
    print("<title>Redirecting...</title>")
    print("</head>")
    print("<body>")
    print("Logging out...")
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()
