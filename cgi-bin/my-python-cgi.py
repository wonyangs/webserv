#!/usr/bin/env python3

import os
import subprocess
import sys


def print_response(status_code, status_message):
    print(f"Status: {status_code} {status_message}", end='\r\n')
    print("Content-type: text/plain", end='\r\n\r\n')


def main():
    # PATH_INFO 환경 변수 가져오기
    path_info = os.environ.get("PATH_INFO", "")

    if path_info:
        try:
            # 파일 실행
            result = subprocess.check_output(["/usr/bin/python3", path_info], text=True)

            # 결과 출력
            print(result)
        except Exception as e:
            print_response(500, "CGI Execute Error")
    else:
        print_response(500, "CGI Invalid PATH_INFO")


if __name__ == "__main__":
    main()
