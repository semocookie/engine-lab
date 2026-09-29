# 곁가지 측정

주제 3을 진행하다 질문이 생겨 따로 재본 것들이다.
[../README.md](../README.md) 의 `⚠️ 틀렸던 것` 과 `참고` 가 여기서 나온 숫자를 인용한다.

`main.cpp` 와 달리 예측용 빈칸이 없다. 답이 이미 문서에 실려 있으므로
다시 돌릴 때는 **먼저 예측을 적고** 실행할 것.

## 파일

| 파일 | 무엇을 재는가 | 나온 답 |
|---|---|---|
| [move_return.cpp](move_return.cpp) | `std::move` 의 반환형, 그리고 그 식이 어느 매개변수로 들어가는가 | 반환형은 하나로 고정. 받을 자리가 없으면 `const T&` 로 **밀려 내려간다** |
| [bind_table.cpp](bind_table.cpp) | 인자 5종 × 매개변수 4종 = 20칸 바인딩 표 | `const T&` 만 열 전체가 O. 이 한 칸의 예외가 조용한 복사를 만든다 |
| [lhs_kinds.cpp](lhs_kinds.cpp) | 대입문 왼쪽을 `T` / `T&&` / `const T&` / `T&` 로 바꾸면 | `T&` 만 컴파일 실패(C2440). 나머지 셋은 통과 |
| [named_rref.cpp](named_rref.cpp) | 참조를 만드는 것이 무언가를 옮기는가 | 복사 0 · 이동 0, `&a == &r`. **이름이 붙으면 lvalue** 가 되어 다시 `std::move` 가 필요하다 |
| [lambda_size3.cpp](lambda_size3.cpp) | 값 · 참조 · 이동 캡처의 `sizeof` | 40 / 8 / **40**. 이동 캡처는 참조가 아니라 값 쪽이다 |
| [dangling_capture.cpp](dangling_capture.cpp) | 참조 캡처한 람다가 대상보다 오래 살아남으면 | 소멸자는 이미 돌았고, **같은 람다를 두 번 불렀는데 답이 달랐다**(`0x0000` → `0xF7B7`). 크래시는 안 났다 |

## 빌드

**Visual Studio** — `generate-solution.bat` 을 다시 돌리면 솔루션의 `extra/move_semantics` 폴더에 타깃이 생긴다.
시작 프로젝트로 지정하고 `Ctrl+F5`.

**명령줄** (이 폴더에서)

```
cl /nologo /EHsc /O2 /W4 /std:c++20 /utf-8 /I ..\..\..\tools <파일>.cpp
```

## 주의

- `dangling_capture.cpp` 는 **최적화를 끄고**(`/Od`, 즉 Visual Studio의 Debug) 돌려야 한다.
  이미 죽은 메모리를 읽는 미정의 동작이라, `/O2` 에서는 관찰하려는 것 자체가 바뀐다.
  구성을 바꿀 때마다 답이 달라지는 것 자체가 이 실험의 결론이기도 하다.
- `bind_table.cpp` 는 `is_invocable_v` 로 **컴파일 가능 여부를 컴파일 타임에** 찍는다.
- `lhs_kinds.cpp` 는 실패를 보는 파일이라 해당 줄이 주석으로 막혀 있다. 풀고 빌드하면 오류가 난다.
