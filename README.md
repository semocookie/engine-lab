# engine-lab

게임 클라이언트 프로그래머로서 **"쓸 줄 아는 것"을 "설명할 수 있는 것"으로 바꾸기 위한** 실험 저장소.

언리얼로 3년간 다뤄 온 기능들의 **아래층**을 직접 구현하고 측정한 기록입니다.
GC를 발표만 하지 않고 만들어 보고, NavMesh를 쓰기만 하지 않고 안을 열어 봅니다.

---

## 규칙

모든 주제는 아래 셋이 **모두** 있어야 완료로 표시합니다. 하나라도 없으면 미완입니다.

| 파일 | 내용 | 없으면 |
|---|---|---|
| `main.cpp` | 돌아가는 최소 구현 | "안다"에서 끝난다 |
| `README.md` | **자료를 닫고** 다시 쓴 설명 + `⚠️ 틀렸던 것` | 두 달 뒤 같은 자리에서 다시 헤맨다 |
| `SPEAK.md` | 60초 구술 스크립트 + 꼬리질문 3개 | 아는데 말을 못 한다 |

추가 규칙

- **백지 노트는 자료를 닫고 쓴다.** 막히면 `?`만 남기고 넘어간다. 그 `?`가 진짜 모르는 부분이다.
- **측정 가능하면 숫자를 남긴다.** ns · MB · 드로우콜 · 프레임 시간.
- **커밋 하나 = 학습 단위 하나.** 형식: `[cpp/01] 객체 레이아웃 — 멤버 순서만 바꿔 48B → 32B`

---

## 진행 현황

| # | 주제 | 구현 | 노트 | 구술 | 측정 결과 |
|---|---|:--:|:--:|:--:|---|
| [00-cpp/01](00-cpp/01-object-layout) | 객체 메모리 레이아웃 | ✅ | ✅ | 🚧 | 멤버 재배치 48B → 32B |
| [00-cpp/02](00-cpp/02-lifetime-ownership) | 수명과 소유권 | ✅ | ✅ | 🚧 | 살아남은 Tracer 6개 → 2개 |
| [00-cpp/03](00-cpp/03-move-semantics) | 이동 시맨틱 | ✅ | ✅ | 🚧 | 총 복사 15회 → 9회 |
| 00-cpp/04 | 스택 오버플로 | — | — | — | — |
| 01-ds/01 | vector vs list — 캐시 | — | — | — | — |
| 01-ds/02 | 해시맵 직접 구현 | — | — | — | — |
| 02-cs/01 | 데이터 레이스 | — | — | — | — |
| 03-unreal/01 | Mark & Sweep GC | — | — | — | — |
| 04-ai/01 | A* | — | — | — | — |

`✅ 완료` · `🚧 진행` · `— 미착수`

---

## 빌드 — Visual Studio

**`generate-solution.bat` 더블클릭** → `build/engine_lab.slnx` 가 생성되고 VS 로 열립니다.
언리얼의 *Generate Visual Studio project files* 와 같은 역할입니다.

열린 뒤 두 가지만:

1. `object_layout` 우클릭 → **시작 프로젝트로 설정**
   (CMake 가 만든 솔루션은 기본이 `ALL_BUILD` 라 실행이 안 됩니다)
2. **`Ctrl+F5`** 로 실행 — `F5` 는 콘솔이 결과를 찍고 바로 닫힙니다

이후로는 `main.cpp` 고치고 `Ctrl+F5` 반복.

### 솔루션을 다시 만들어야 할 때

- 처음 clone 했을 때
- `build/` 를 지웠을 때
- **새 주제 폴더(`main.cpp`)를 추가했을 때**

→ `generate-solution.bat` 를 다시 실행하면 됩니다.

`CMakeLists.txt` 가 소스이고 `build/` 안의 `.slnx` · `.vcxproj` 는 **생성물**입니다.
그래서 `build/` 는 `.gitignore` 에 있습니다.

### 새 주제 시작

```bash
cp -r _template 00-cpp/02-lifetime-ownership
```

그다음 `generate-solution.bat` 실행. `main.cpp` 가 있으면 CMake 가 타깃을 자동으로 잡습니다.

### 명령줄로 빌드하려면

```bash
cmake -B build && cmake --build build --config Release && ./build/bin/Release/object_layout.exe
```

`cmake` 가 PATH 에 없다면 VS 에 번들된 것을 쓰면 됩니다:
`<VS설치경로>\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`

---

## 구조

```
engine-lab/
├─ _template/           새 주제 시작할 때 복사해 쓰는 뼈대
├─ tools/               공통 헤더 (타이머 · 레이아웃 출력 · hexdump)
├─ 00-cpp/              C++ 언어와 메모리 모델
├─ 01-datastructure/    자료구조 = 캐시
├─ 02-cs/               동시성 · 메모리 · 네트워크
├─ 03-unreal-internals/ GC · 리플렉션 · Replication
├─ 04-ai-pathfinding/   A* · Funnel · RVO
├─ 05-graphics/         렌더링 파이프라인 · 래스터라이저
└─ 06-agentic-ai/       MCP · 멀티 에이전트 워크플로우
```

---

## 관련 저장소

- **TDProject** — UE 5.7 MOBA 개인 프로젝트. 여기서 배운 것을 실제 게임에 적용하는 곳
  *(engine-lab = 지식 / TDProject = 제품)*
