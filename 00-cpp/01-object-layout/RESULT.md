# 실측 결과 — 객체 메모리 레이아웃

환경: Windows 11 / MSVC 19.51 (VS 2026) / x64 / Release
날짜: 🔵

> ## 🔵 = 내가 채울 곳
> **이 파일에 🔵 이 하나도 남지 않으면 완료.**
>
> **먼저 [예측] 칸을 채우고, 그다음에 실행한다.**
> 틀린 예측이 이 문서에서 가장 값어치 있는 부분이다. **지우지 말고 남긴다.**

---

## 실험 1 — 기본 타입

| 타입 | 예측 sizeof | 실측 sizeof | 실측 alignof | 비고 |
|---|---:|---:|---:|---|
| `char` | 1byte | 1byte | 1 | - |
| `short` | 1byte | 2byte | 2 | 기본 자료형을 잘못 기억하고 있었음. |
| `int` | 4byte | 4byte | 4 | - |
| `long` | 4byte | 4byte | 4 | - |
| `long long` | 8byte | 8byte | 8 | - |
| `float` | 4byte | 4byte | 4 | - |
| `double` | 8byte | 8byte | 8 | - |
| `void*` | 4byte(32bit), 8byte(64bit) | 4byte(32bit), 8byte(64bit) | 4byte(32bit), 8byte(64bit) | 32bit 릴리즈 빌드로 따로 실행 후 측정. |
| `bool` | 1byte | 1byte | 1 | - |

- `long`이 Windows에서 4바이트인 이유:
  - C++ 표준은 long의 하한만 정하고 실제 크기는 플랫폼 ABI가 정함. windows x64에서는 32bit 코드 호환을 위해 포인터만 8byte로 늘렸고 long은 그대로 남았다.
- `bool`이 1비트가 아니라 1바이트인 이유:
  - 주소를 붙일 수 있는 최소 단위가 1byte.

---

## 실험 2 — 같은 멤버, 다른 순서

| 구조체 | 예측 | 실측 | 패딩 |
|---|---:|---:|---:|
| `BadOrder` | 24 | 24 | 10 |
| `GoodOrder` (재배치 후) | 16 | 16 | 2 |

내가 찾은 최적 순서:

```cpp
struct GoodOrder {
    double b;
    int    d;
    char   a;
    char   c;
};
```

**재배치 규칙 한 줄:**
멤버를 alignof 내림차순으로 배치.

---

## 실험 3 — vptr

| 타입 | 예측 sizeof | 실측 sizeof | 비고 |
|---|---:|---:|---:|
| `Plain` | 4byte | 4byte | - |
| `Poly` | 12byte | 16byte | int 4byte와 가상 함수 포인터 8byte가 더한 12byte라 생각 |
| `PolyChild` | 20byte | 16byte | Poly의 크기와 가상 함수 포인터 8byte가 더해질 것이라 생각 |

hexdump에서 관찰한 것:

- `Poly` 앞 8바이트: 🔵
- `PolyChild` 앞 8바이트: 🔵
- 다시 실행하면 이 값이 바뀌는가: 🔵
- 두 값이 다른 이유:
  🔵
- `x`가 offset 🔵 부터 나온 이유:
  🔵
- `Poly`가 4 + 8 = 12가 아니라 16인 이유:
  🔵

### 슬라이싱 (TODO 3)

| 호출 | 예측 출력 | 실측 출력 |
|---|---|---|
| `Poly sliced = c; sliced.speak();` | 🔵 | 🔵 |
| `Poly& ref = c; ref.speak();` | 🔵 | 🔵 |

`sliced`의 앞 8바이트가 `c`와 다른 이유:
🔵

---

## 실험 4 — 빈 구조체

- `sizeof(Empty)` = 🔵 . **0이면 안 되는 이유:**
  🔵
- `sizeof(DerivedFromEmpty)` = 🔵 . `Empty` 크기와 `int` 크기의 단순 합과 다른 이유(EBO):
  🔵

---

## 실험 5 — `#pragma pack(1)`

| 구조체 | 예측 sizeof | 실측 sizeof | 실측 alignof |
|---|---:|---:|---:|
| `BadOrder` | 🔵 | 🔵 | 🔵 |
| `Packed` | 🔵 | 🔵 | 🔵 |

- 항상 pack(1)을 쓰지 않는 이유:
  🔵
- 그럼에도 네트워크 패킷/파일 포맷에서 쓰는 이유:
  🔵

---

## 실험 6 — 실전

| 구조체 | 예측 sizeof | 실측 sizeof | 멤버 합 | 패딩 |
|---|---:|---:|---:|---:|
| `MonsterState` | 🔵 | 🔵 | 🔵 | 🔵 |
| `MonsterStateTight` | 🔵 | 🔵 | 🔵 | 🔵 |

- 개체당 절약: **🔵 바이트**
- 1,000마리 기준: **🔵 KB**
- 이론상 최소값(28을 정렬 배수로 올림)과 내 결과가 같은가:
  🔵

---

## 한 문장 요약

> 🔵
