#!/usr/bin/env python3

import os
import json
from http import cookies

# 세션 데이터 파일 경로
SESSION_FILE = 'cgi-bin/sessions.json'

# CGI 환경 설정
print("Content-type: application/json\r\n\r\n")

# 세션 데이터를 불러오는 함수
def load_sessions():
    if os.path.exists(SESSION_FILE):
        with open(SESSION_FILE, 'r') as file:
            return json.load(file)
    return {}

# 세션 확인
def check_session():
    cookie = cookies.SimpleCookie(os.environ.get('HTTP_COOKIE'))
    session_id = cookie['session_id'].value if 'session_id' in cookie else None
    sessions = load_sessions()
    return session_id in sessions

# 로그인 상태 확인 및 JSON 응답
logged_in = check_session()
print(json.dumps({"loggedIn": logged_in}))
