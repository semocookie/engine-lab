# 곁가지 측정

주제 1을 마친 뒤, 파인만 점검에서 vptr 과 패딩을 설명하다가 질문이 생겨 따로 재본 것들이다.
[../README.md](../README.md) 의 `⚠️ 틀렸던 것` 이 여기서 나온 결과를 인용한다.

`main.cpp` 와 달리 예측용 빈칸이 없다. 답이 이미 문서에 실려 있으므로
다시 돌릴 때는 **먼저 예측을 적고** 실행할 것.

## 파일

| 파일 | 무엇을 재는가 | 나온 답 |
|---|---|---|
| [vtable_dump.cpp](vtable_dump.cpp) | vtable 안에 무엇이 있나. 객체 맨 앞 8바이트를 함수 포인터 배열로 읽는다 | 클래스당 1개(Actor 객체 셋이 같은 vptr). 칸은 선언 순서, 비가상·정적 함수는 흔적이 없다. **Actor 객체에 Pawn 의 `[1]` 을 물리면 `Pawn::WhoAmI`** — 호출을 정하는 건 vptr 뿐이다 |
| [typeid_vptr.cpp](typeid_vptr.cpp) | `typeid` 도 vptr 을 따라가는가. vptr 을 손으로 바꿔치기한다 | 같은 객체인데 **이름으로 물으면 Actor, 참조로 물으면 Pawn**. 컴파일러는 선언을, 런타임은 vptr 을 본다 |
| [rtti_dump.cpp](rtti_dump.cpp) | `vt[-1]` 을 따라가 RTTI 구조체를 직접 읽는다 | 이름 + 조상 전체를 평평하게 늘어놓은 배열 + 조상마다 객체 안 위치. 다중 상속에서 `dynamic_cast` 가 주소를 **+8** 옮긴다 |
| [rtti_switch.cpp](rtti_switch.cpp) | 같은 파일을 RTTI 켬(`/GR`) · 끔(`/GR-`)으로 두 번 빌드해 비교 | 끄면 vtable 앞칸이 **통째로 사라진다**. 참조로 부른 `typeid` 는 그래도 그 칸을 읽다가 `Access violation - no RTTI data!` 예외. 가상 호출과 이름으로 부른 `typeid` 는 멀쩡하다 |
| [padding_bytes.cpp](padding_bytes.cpp) | 패딩 3바이트에 무엇이 들어 있나. 만드는 방법(지역 변수 · `{}` · 값을 넣은 `{}` · `new`)과 Debug / Release 에 따라 | **아무도 쓰지 않는다.** 원래 있던 값이 남는다. Debug 는 `CC`(스택)·`CD`(힙) 표시, Release 는 찌꺼기. 0 은 `A a{}` 와 아직 아무도 안 쓴 깨끗한 자리뿐. 실행 중에 바이트만 보고는 패딩을 구분할 수 없다 |

## 빌드

**Visual Studio** — `generate-solution.bat` 을 다시 돌리면 솔루션의 `extra/object_layout` 폴더에 타깃이 생긴다.
시작 프로젝트로 지정하고 `Ctrl+F5`. 구성은 **Debug** 로 둔다. `padding_bytes` 만 **Release** 로도 한 번 더 돌린다.

`rtti_switch.cpp` 만 타깃이 둘이다.

| 타깃 | RTTI |
|---|---|
| `object_layout_rtti_switch` | 켬 (기본값) |
| `object_layout_rtti_switch_off` | 끔 (`CMakeLists.txt` 에서 따로 `/GR-` 지정) |

**명령줄** (이 폴더에서)

```
cl /nologo /EHsc /Od /W4 /std:c++20 /utf-8 /I ..\..\..\tools <파일>.cpp
cl /nologo /EHsc /Od /W4 /std:c++20 /utf-8 /I ..\..\..\tools /GR- rtti_switch.cpp
cl /nologo /EHsc /Od /MDd /RTC1 /W4 /std:c++20 /utf-8 /I ..\..\..\tools padding_bytes.cpp   ← Debug 와 같게
cl /nologo /EHsc /O2 /MD        /W4 /std:c++20 /utf-8 /I ..\..\..\tools padding_bytes.cpp   ← Release 와 같게
```

## 주의

- `padding_bytes.cpp` 를 뺀 나머지는 **최적화를 끄고**(`/Od`, 즉 Debug) 돌린다. `typeid_vptr.cpp` 와 `rtti_switch.cpp` 는 vptr 을 손으로 덮어쓰는
  미정의 동작이라, 최적화가 켜지면 관찰하려는 것 자체가 바뀐다.
- `padding_bytes.cpp` 는 **Debug 와 Release 를 둘 다** 돌린다. 두 구성의 답이 다른 것 자체가 결과다.
  명령줄로 돌릴 때 `/Od` 만 주면 `/RTC` 와 디버그 CRT 가 꺼져 있어서 Debug 와 다른 값(스택 찌꺼기)이 나온다. 위 명령줄처럼 `/MDd /RTC1` 까지 준다.
- vtable·RTTI 를 다루는 네 파일은 **MSVC x64 구현**에 기댄다. vptr 이 맨 앞에 있다는 것, `vt[-1]` 에 RTTI 주소가 있다는 것,
  RTTI 구조체의 모양은 표준이 정한 게 아니다.
- `vtable_dump.cpp` 는 테이블 `[0]` 을 부르지 않는다. MSVC 의 소멸자 칸은 `delete` 까지 하는 변형(`__vecDelDtor`)이라
  스택 객체에 부르면 스택 주소를 해제하러 간다.
- vtable 의 **칸 수는 코드로 셀 수 없다.** 배열 끝 표시가 없어서다. `cl /c /d1reportSingleClassLayout<클래스이름> <파일>.cpp`
  로 컴파일러에게 물어본다.
  단 이 보고서는 `/GR-` 에서도 RTTI 칸(`&Actor_meta`)을 똑같이 찍는다. RTTI 를 껐을 때 실제로 무엇이 만들어지는지는
  어셈블리(`/FA`)를 열어 봐야 한다 — 켜면 vftable 이 3줄, 끄면 2줄이다.
- `object_layout_rtti_switch_off` 빌드에서는 경고 C4541 이 뜬다. 의도한 것이다.
- `padding_bytes.cpp` 의 `CC`·`CD` 는 MSVC 디버그 CRT 가 칠하는 값이다. `CC` 는 `/RTC` 가 초기화 안 한 스택에,
  `CD` 는 디버그 힙이 새로 내준 칸에 칠한다. 다른 컴파일러나 Release 에서는 다른 값이 나온다.
- 패딩 자리는 **실행 중에 바이트로는 구분할 수 없다.** 컴파일할 때 물어본다 — 경고 C4820(`/w14820` 로 켬),
  `/d1reportSingleClassLayout<이름>` 의 `<alignment member>`, `std::has_unique_object_representations_v<T>`(패딩이 있으면 `false`).

## 참고 — 직접 열어본 소스

MSVC 14.51.36231 (`VC\Tools\MSVC\14.51.36231\`)

- `include\rttidata.h:111` — CompleteObjectLocator. `vt[-1]` 이 가리키는 곳
- `include\rttidata.h:86` — ClassHierarchyDescriptor. 계보 배열을 쥔다
- `include\rttidata.h:37` — BaseClassDescriptor. 조상 한 칸 (이름 · 객체 안 위치 · 상속 방식)
- `crt\src\vcruntime\rtti.cpp:102` — `[0][-1]`. vptr 을 꺼내 한 칸 앞을 읽는다
- `crt\src\vcruntime\rtti.cpp:141` — `__RTtypeid`. RTTI 가 꺼졌는지 모르고 여전히 `vt[-1]` 을 읽는다
- `crt\src\vcruntime\rtti.cpp:163` — 접근 위반을 `no RTTI data!` 예외로 바꾸는 `__except`
- `crt\src\vcruntime\rtti.cpp:365` — `FindSITargetTypeInstance`. 계보 배열을 앞에서부터 훑는다
- `crt\src\vcruntime\rtti.cpp:425` — 주소 비교가 실패하면 `strcmp` 로 이름을 비교한다 (다른 DLL 에서 온 객체)

Unreal Engine 5.7

- `Runtime\CoreUObject\Public\UObject\Class.h:430` — `IsChildOfUsingStructArray`. 부모의 깊이를 칸 번호로 한 번만 비교한다
- `Runtime\CoreUObject\Public\UObject\ObjectMacros.h:39` — 에디터 빌드에서는 이 방식을 끄고 부모를 한 칸씩 거슬러 올라간다
