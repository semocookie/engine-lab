# 실측 결과 — 객체 메모리 레이아웃

환경: Windows 11 / MSVC 19.51 (VS 2026) / x64 / Release
날짜: 2026-08-10

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

- `Poly` 앞 8바이트: 88 33 16 F8 F6 7F 00 00 (두 번째도 같음)
- `PolyChild` 앞 8바이트: B0 33 16 F8 F6 7F 00 00 (두 번째도 같음)
- 다시 실행하면 이 값이 바뀌는가: 아니요
- 두 값(앞 8바이트)이 다른 이유:
  앞 8바이트가 vptr이고, 각자 자기 클래스의 vtable 주소를 담고 있어서 다르다.
- `x`가 offset 8 부터 나온 이유:
  vptr이 offset 0에 8바이트를 차지하기 때문에.
- `Poly`가 4 + 8 = 12가 아니라 16인 이유:
  멤버 중 가장 큰 alignof, 가상 함수 포인터의 alignof가 8byte이고 전체의 sizeof는 그 alignof의 배수여야 하기 때문에.

### 슬라이싱 (TODO 3)

| 호출 | 예측 출력 | 실측 출력 |
|---|---|---|
| `Poly sliced = c; sliced.speak();` |   Poly::speak |   Poly::speak |
| `Poly& ref = c; ref.speak();` |   Poly::speak |   PolyChild::speak |

Poly      88 33 6A 0A F7 7F 00 00  11 11 11 11 58 00 00 00
PolyChild B0 33 6A 0A F7 7F 00 00  22 22 22 22 00 00 00 00
sliced    88 33 6A 0A F7 7F 00 00  22 22 22 22 58 00 00 00

`sliced`의 앞 8바이트가 `c`와 다른 이유:
PolyChild로 만든 c를 Poly로 만든 sliced를 복사 생성하면, 멤버 변수들만 복사되고 객체 생성 당시의 Poly의 vptr은 생성자가 Poly의 vtable로 설정한 뒤 그대로이기 때문에 c와는 다르다.

---

## 실험 4 — 빈 구조체

- `sizeof(Empty)` = 1 . **0이면 안 되는 이유:**
  객체를 여러 개 생성 했을 때 객체들은 서로 다른 주소를 가져야 하기 때문에 가장 작은 주소 단위인 1byte로 빈 공간을 갖게된다.
- `sizeof(DerivedFromEmpty)` = 4 . `Empty` 크기와 `int` 크기의 단순 합과 다른 이유(EBO):
  기반 클래스가 비어있는 상태에서 파생클래스 기준으로 주소를 시작하면 되므로.

EBO 없었다면 : 1(Empty) + 3(패딩) + 4(int) = 8
실측          : 4

---

## 실험 5 — `#pragma pack(1)`

| 구조체 | 예측 sizeof | 예측 alignof | 실측 sizeof | 실측 alignof |
|---|---:|---:|---:|---:|
| `BadOrder` | 24byte | 8byte | 24byte | 8byte |
| `Packed` | 14byte | 1byte | 14byte | 1byte |

- 항상 pack(1)을 쓰지 않는 이유:
  pack(1)을 사용하면 패딩 바이트를 이용한 정렬이 깨지기 때문에 캐시 라인 정렬까지 깨져 성능이 낮아지고 일부 플랫폼에선 크래시가 날 수 있다.
- 그럼에도 네트워크 패킷/파일 포맷에서 쓰는 이유:
  데이터 크기도 줄일 수 있지만 패킷을 사용하는 플랫폼의 정렬 규칙이 어떤지 모르기 때문에 발신 쪽, 수신 쪽 패딩을 없애서 같은 규칙을 사용하도록 한다.

---

## 실험 6 — 실전

| 구조체 | 예측 sizeof | 실측 sizeof | 멤버 합 | 패딩 |
|---|---:|---:|---:|---:|
| `MonsterState` | 48byte | 48byte | 28byte | 20byte |
| `MonsterStateTight` | 32byte | 32byte | 28byte | 4byte |

- 개체당 절약: **16 바이트**
- 1,000마리 기준: **약 15.6 KB**
- 이론상 최소값(28을 정렬 배수로 올림)과 내 결과가 같은가:
  같다. 28을 8의 배수로 올린 32가 하한이고 실측이 32.

---

## 한 문장 요약

> 객체 메모리는 멤버 변수 중 제일 큰 alignof 크기 배수를 기준으로 맞춰 메모리에 저장한다. 단 크기 배수에 딱 안맞춰질 경우 중간에 패딩 바이트를 넣어 다음 변수의 alignof 크기 배수를 맞추도록 한다.
