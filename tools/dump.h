// tools/dump.h — 메모리 레이아웃을 눈으로 보기 위한 헬퍼
//
// 이 헤더는 "보조 도구"다. 학습의 본체는 각 주제의 main.cpp에 있다.
#pragma once

#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <string>
#include <initializer_list>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <windows.h>
#endif

namespace lab {

// Windows 콘솔이 cp949라 UTF-8 한글이 깨지는 것을 막는다.
struct ConsoleInit {
    ConsoleInit() {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
#endif
    }
};
inline ConsoleInit g_console_init{};

inline void rule(char c = '-', int n = 66) {
    for (int i = 0; i < n; ++i) std::putchar(c);
    std::putchar('\n');
}

inline void section(const char* s) {
    std::putchar('\n');
    rule('=');
    std::printf("  %s\n", s);
    rule('=');
}

// 바이트를 16진수로 찍는다. vptr 확인용.
inline void hexdump(const void* p, std::size_t n, const char* label = "") {
    const unsigned char* b = static_cast<const unsigned char*>(p);
    std::printf("  %-16s", label);
    for (std::size_t i = 0; i < n; ++i) {
        std::printf("%02X ", b[i]);
        if ((i + 1) % 8 == 0 && i + 1 < n) std::printf(" ");
    }
    std::putchar('\n');
}

struct FieldInfo {
    const char* name;
    std::size_t off;
    std::size_t size;
};

// 구조체의 바이트 지도를 그린다.
//   [aaaaaaaa..bbbb....]   a/b = 멤버,  . = 패딩
inline void layout_map(const char* type_name,
                       std::size_t total,
                       std::initializer_list<FieldInfo> fields)
{
    std::string marks(total, '.');
    char mark = 'a';
    std::size_t used = 0;

    for (const FieldInfo& f : fields) {
        for (std::size_t i = 0; i < f.size && f.off + i < total; ++i)
            marks[f.off + i] = mark;
        used += f.size;
        ++mark;
    }

    std::printf("  %-20s sizeof=%-4zu 멤버합=%-4zu 패딩=%zu\n",
                type_name, total, used, total - used);
    std::printf("    [%s]\n", marks.c_str());

    mark = 'a';
    for (const FieldInfo& f : fields) {
        std::printf("     %c  %-16s offset=%-4zu size=%zu\n",
                    mark, f.name, f.off, f.size);
        ++mark;
    }
    std::printf("     .  (padding)\n");
}

} // namespace lab

// sizeof / alignof 한 줄 출력
#define LAYOUT(T) \
    std::printf("  %-24s sizeof=%-4zu alignof=%zu\n", #T, sizeof(T), alignof(T))

// layout_map 에 넘길 FieldInfo 하나를 만든다.
//   lab::layout_map("Foo", sizeof(Foo), { FIELD(Foo, a), FIELD(Foo, b) });
// 주의: offsetof 는 standard-layout 타입에서만 안전하다.
//       (가상 함수가 있으면 표준상 보장되지 않는다 — 그래서 그 경우엔 hexdump 를 쓴다)
#define FIELD(T, m) lab::FieldInfo{ #m, offsetof(T, m), sizeof(T::m) }
