# 곁가지 측정

주제 2를 마친 뒤, 파인만 점검에서 RAII 를 설명하다가 질문이 생겨 따로 재본 것들이다.
[../README.md](../README.md) 의 `⚠️ 틀렸던 것` 이 여기서 나온 결과를 인용한다.

`main.cpp` 와 달리 예측용 빈칸이 없다. 답이 이미 문서에 실려 있으므로
다시 돌릴 때는 **먼저 예측을 적고** 실행할 것.

## 파일

| 파일 | 무엇을 재는가 | 나온 답 |
|---|---|---|
| [unique_ptr_size.cpp](unique_ptr_size.cpp) | `Tracer*` 와 `std::unique_ptr<Tracer>` 의 크기, 그리고 안에 든 바이트 | **둘 다 8**. 안에는 주소 하나뿐이다. 기본 삭제자는 빈 클래스라 0바이트로 접힌다. 16 이 되는 건 삭제자가 값을 가질 때(함수 포인터)와 `shared_ptr` |
| [double_close.cpp](double_close.cpp) | `FileHandle` 의 복사 금지(`= delete`)를 풀고 `FileHandle b = a;` | 같은 `FILE*` 를 둘이 쥔다. 두 번째 `fclose` 는 크래시 없이 `-1`. 그 사이에 새 파일을 열면 **같은 `FILE*` 를 받아서**, a 가 남의 파일을 `0`(성공)으로 닫고 새 파일에 쓴 글이 사라진다 |

## 빌드

**Visual Studio** — `generate-solution.bat` 을 다시 돌리면 솔루션의 `extra/lifetime_ownership` 폴더에 타깃이 생긴다.
시작 프로젝트로 지정하고 `Ctrl+F5`. 구성은 **Debug** 로 둔다.

**명령줄** (이 폴더에서. `/MDd` 는 Visual Studio Debug 와 같은 CRT 를 쓰게 한다)

```
cl /nologo /EHsc /Od /MDd /W4 /std:c++20 /utf-8 /I ..\..\..\tools <파일>.cpp
```

## 주의

- `double_close.cpp` 는 이미 닫은 `FILE*` 를 다시 쓰는 **미정의 동작**이다. 결과는 UCRT 구현에 기댄다 —
  닫힌 스트림은 건너뛰고 `EOF` 를 돌려주는 것, 닫은 `FILE` 칸을 다음 `fopen` 이 다시 쓰는 것.
- `double_close.cpp` 는 실행하는 동안 `%TEMP%` 에 `engine_lab_a.txt`, `engine_lab_c.txt` 를 만들었다가 끝에서 지운다.
- `double_close.cpp` 는 크래시가 나도 찍은 데까지는 보이도록 표준 출력 버퍼를 끄고,
  CRT 오류 창이 뜨지 않게 막아 두었다. 그 장치를 다 켜 두었는데도 아무것도 걸리지 않은 것이 결과다.
- `unique_ptr_size.cpp` 의 바이트는 리틀 엔디언이라 뒤에서부터 읽어야 주소가 된다.

## 참고 — 직접 열어본 소스

MSVC 14.51.36231 (`VC\Tools\MSVC\14.51.36231\`)

- `include\memory:3518` — `unique_ptr` 의 멤버는 `_Compressed_pair<_Dx, pointer> _Mypair` 하나뿐이다
- `include\xmemory:1542` — 첫 타입(삭제자)이 비어 있으면 그걸 상속해서 0바이트로 접는다 (`is_empty_v<_Ty1>`)
- `include\xmemory:1568` — 비어 있지 않으면 둘 다 멤버로 든다. 그래서 함수 포인터 삭제자는 16바이트이고, 삭제자가 앞에 온다

Windows SDK 10.0.26100.0 (`Windows Kits\10\Source\10.0.26100.0\`)

- `ucrt\stdio\fclose.cpp:23` — `result = EOF` 로 시작해서, 사용 중인 스트림일 때만 실제로 닫는다
- `ucrt\stdio\fclose.cpp:41` — `__acrt_stdio_free_stream`. 칸을 비워 두고, 다음 `fopen` 이 그 칸을 다시 쓴다
