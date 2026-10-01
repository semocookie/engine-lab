// 복사 금지를 풀면 — 같은 FILE* 를 두 객체가 쥔다
//
//   FileHandle 에서 = delete 두 줄을 지우면, 컴파일러가 만든 복사 생성자가
//   포인터 값만 베낀다. 파일은 하나인데 닫으려는 객체는 둘이 된다.
//
//   ⚠ 이미 닫은 FILE* 를 다시 쓰는 것은 미정의 동작이다. 들여다보기 전용.
//   ⚠ Debug(/Od) 로 돌릴 것.

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <string>
#include <crtdbg.h>
#include "dump.h"

struct FileHandle {
    FILE* f;

    explicit FileHandle(const char* Path) : f(std::fopen(Path, "w")) {
        std::printf("    + 객체 %p  fopen  → f = %p\n", static_cast<void*>(this), static_cast<void*>(f));
    }
    ~FileHandle() {
        if (f) {
            int r = std::fclose(f);
            std::printf("    - 객체 %p  fclose(%p) → %d\n", static_cast<void*>(this), static_cast<void*>(f), r);
        }
    }
    // FileHandle(const FileHandle&) = delete;             ← 지웠다
    // FileHandle& operator=(const FileHandle&) = delete;  ← 지웠다
};

// CRT 가 잘못된 인자를 잡으면 창을 띄우거나 프로그램을 끝내는 대신 여기로 온다
static void OnInvalidParam(const wchar_t*, const wchar_t* Func, const wchar_t*, unsigned, uintptr_t) {
    std::printf("      ! CRT 가 잘못된 인자를 잡음 : %ls\n", Func ? Func : L"(이름 없음)");
}

static std::string TempPath(const char* Name) {
    char Dir[MAX_PATH];
    GetTempPathA(MAX_PATH, Dir);
    return std::string(Dir) + Name;
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);                       // 죽더라도 찍은 데까지는 보이게
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    _set_invalid_parameter_handler(OnInvalidParam);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

    const std::string PathA = TempPath("engine_lab_a.txt");
    const std::string PathC = TempPath("engine_lab_c.txt");

    lab::section("1. FileHandle b = a;");
    {
        FileHandle a(PathA.c_str());
        FileHandle b = a;
        std::printf("    a.f = %p   b.f = %p\n", static_cast<void*>(a.f), static_cast<void*>(b.f));
        std::printf("    -- 스코프 끝. 지역 변수는 만든 반대 순서로 소멸 --\n");
    }
    std::printf("    (여기까지 왔다)\n");

    lab::section("2. b 가 닫은 뒤, a 가 닫기 전에 다른 파일을 하나 연다면");
    std::optional<FileHandle> c;
    {
        FileHandle a(PathA.c_str());
        { FileHandle b = a; }                                  // b 소멸 → 닫힘
        std::printf("    -- c 가 새 파일을 연다 --\n");
        c.emplace(PathC.c_str());
        std::printf("    a.f == c->f ? %s\n", a.f == c->f ? "같다" : "다르다");
        std::printf("    -- a 소멸 --\n");
    }
    std::printf("    -- c 가 자기 파일에 쓴다 --\n");
    int w = std::fputs("hello\n", c->f);
    std::printf("    fputs → %d\n", w);
    std::printf("    -- c 소멸 --\n");
    c.reset();

    std::remove(PathA.c_str());
    std::remove(PathC.c_str());
    std::printf("\n    (끝까지 왔다)\n\n");
    return 0;
}
