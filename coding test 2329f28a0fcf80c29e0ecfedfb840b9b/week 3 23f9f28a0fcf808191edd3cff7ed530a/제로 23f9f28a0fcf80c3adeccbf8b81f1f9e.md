# 제로

https://www.acmicpc.net/problem/10773

0 입력 → 최근에 쓴 숫자 지움.

⇒ push 0 → pop()

모든 수 합하기

```cpp
    for(int i = 0; i < s.size();i++)
    {
        res += s.top();
        s.pop();
    }
```

pop 할 때 empty인지 확인하기