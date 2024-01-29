import cgi
import os
import json
from http import cookies

# CGI 환경 설정
print("Content-type: text/html; charset=UTF-8\r\n\r\n")

# 세션 데이터 파일 경로
SESSION_FILE = 'cgi-bin/sessions.json'

# 세션 데이터를 불러오는 함수
def load_sessions():
    if os.path.exists(SESSION_FILE):
        with open(SESSION_FILE, 'r') as file:
            return json.load(file)
    return {}

# 세션에서 사용자 이름 찾기
def get_username_from_session():
    cookie = cookies.SimpleCookie(os.environ.get('HTTP_COOKIE'))
    session_id = cookie['session_id'].value if 'session_id' in cookie else None
    sessions = load_sessions()
    return sessions.get(session_id, '게스트')

# 사용자 이름 가져오기
username = get_username_from_session()

# 페이지 번호 및 이미지 디렉토리 설정
form = cgi.FieldStorage()
page = int(form.getvalue('page', '1'))
image_dir = os.environ["UPLOAD_PATH"].rstrip('/')
images_per_page = 4

# 웹 서버의 이미지 URL 경로 설정
web_image_path = "/upload"

# 이미지 파일 탐색
image_files = [f for f in os.listdir(image_dir) if f.endswith(('.png', '.jpeg', '.jpg', '.gif'))]
total_pages = (len(image_files) + images_per_page - 1) // images_per_page

# 현재 페이지의 이미지 선택
start = (page - 1) * images_per_page
end = start + images_per_page
current_images = image_files[start:end]

# HTML 응답 생성
print("<html><head>")
print("<style>")
print("  body { font-family: Arial, sans-serif; text-align: center; padding: 20px; }")
print("  .gallery {")
print("    display: grid;")
print("    grid-template-columns: repeat(2, 1fr);")
print("    gap: 10px;")
print("    justify-content: center;")
print("  }")
print("  .gallery img {")
print("    width: 200px;")
print("    height: 200px;")
print("    border: 1px solid #ddd;")
print("    border-radius: 4px;")
print("    padding: 5px;")
print("  }")
print("  .gallery button {")
print("    color: red;")
print("    margin-top: 5px;")
print("  }")
print("  .navigation { margin-top: 20px; }")
print("</style>")
print("<script type='text/javascript'>")
print("function deleteImage(imagePath) {")
print("    if (confirm('이미지를 삭제하시겠습니까?')) {")
print("        var xhr = new XMLHttpRequest();")
print("        xhr.open('DELETE', imagePath, true);")
print("        xhr.onload = function () {")
print("            if (xhr.status === 204) {")
print("                alert('이미지 삭제 성공');")
print("                window.location.reload();")  # 페이지 새로고침
print("            } else {")
print("                alert('이미지 삭제 실패: ' + xhr.status);")
print("            }")
print("        };")
print("        xhr.send();")
print("    }")
print("}")
print("</script>")
print("</head><body>")

print(f"<h2>안녕하세요, {username}님</h2>")

print("<div class='gallery'>")
for image in current_images:
    image_url = f"{web_image_path}/{image}"
    print(f'<div><img src="{image_url}" alt="{image}"/>')
    print(f'<button onclick="deleteImage(\'{image_url}\')">X</button></div>')
print("</div>")

# 페이지네이션과 홈 버튼
print("<div class='navigation'>")
print("<button onclick=\"location.href='/'\">홈으로</button>")
for i in range(1, total_pages + 1):
    print(f'<a href="?page={i}">{i}</a> ')
print("</div>")

print("</body></html>")
