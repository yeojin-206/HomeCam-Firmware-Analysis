# cin의 getline() 함수

`*cin.getline(char str, streamsize n);*`

`*cin.getline(char str, streamsize n, char dlim);*`

→ 입력 가능한 최대 문자 수를 지정x

→ 지정한 delimiter를 만나기 전까지 모든 문자 읽어서 string 객체에 저장 (디폴트 : 개행 문자)

주의사항 

- **char*형의 문자열**을 받을 경우 사용할 수 있다.

cin은 개행 문자를 버퍼에 그대로 남겨두기 때문에 연이어서 바로 입력을 받으면 버퍼에 있던 개행 문자가 그대 입력된다.

![image.png](cin%E1%84%8B%E1%85%B4%20getline()%20%E1%84%92%E1%85%A1%E1%86%B7%E1%84%89%E1%85%AE%202339f28a0fcf801cb3b9e2e89cd1cdee/image.png)

cin.ignore() 해주면 입력 버퍼를 지우기 때문에 b:이게 안나옴

참고자료

https://velog.io/@jxlhe46/C-getline-%ED%95%A8%EC%88%98