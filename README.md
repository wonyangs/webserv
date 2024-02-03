<div align="center">
   <h1> webserv </h1>
   <h3> Our own NGINX </h3>

<p>
  <a href="https://github.com/wonyangs/webserv/wiki">위키</a>
</p>

  <p>
  <a href="https://github.com/wonyangs/webserv/wiki/config-%ED%8C%8C%EC%9D%BC-%EB%AA%85%EC%84%B8">config 파일 명세</a>
    &nbsp; | &nbsp; 
  <a href="https://github.com/wonyangs/webserv/wiki/%ED%95%99%EC%8A%B5-%EB%B0%8F-%EA%B0%9C%EB%B0%9C-%EC%9D%BC%EC%A7%80">학습 및 개발 일지</a>
  &nbsp; | &nbsp; 
  <a href="https://github.com/wonyangs/webserv/wiki#%EF%B8%8F-%EA%B8%B0%EB%A1%9D">데일리 스크럼 및 회의록 </a>
&nbsp; | &nbsp; 
    <a href="https://github.com/wonyangs/webserv/wiki/%ED%9A%8C%EA%B3%A0">회고</a>
 
  
  
</p>
</div>
<br>

## ⚙️ 프로젝트 소개

NGINX와 같이 나만의 HTTP 서버를 구현하는 프로젝트입니다.

https://github.com/wonyangs/webserv/assets/44529556/a5a62a04-5875-4b4c-a429-26012b6e0588

### 사용법

```
make
./webserv [configuration file]
```

### 주요 기능

- HTTP/1.1 요청 처리: GET, POST, DELETE
- 클라이언트 요청에 따른 적절한 HTTP Response 및 상태 코드 반환
- kqueue를 활용한 I/O 멀티플렉싱
- CGI 실행
- 서버 설정 파일을 통해 가상 서버 커스텀 설정

### 부하 테스트

| siege 테스트 실행 | 실행 결과 |
| --- | --- |
| ![siegetest_final](https://github.com/wonyangs/webserv/assets/44529556/f7e170ec-09ae-4363-95d3-d28e0b1153bb) | ![image](https://github.com/wonyangs/webserv/assets/44529556/09de6233-f2df-42ff-b8a6-5d078cbca1a0) |

## 👥 팀원

|<img src="https://github.com/wonyangs.png" width=150>|<img src="https://github.com/mingxoxo.png" width="150">|
|:--:|:--:|
|🦊|🐶|
|[wonyang](https://github.com/wonyangs)|[jeongmin](https://github.com/mingxoxo)|
