#include "dump.h"   // 콘솔 UTF-8 설정을 겸한다
#include <cstdio>
#include <string>
#include <type_traits>
#include <utility>

struct Payload {
    std::string Data = std::string(64, 'x');
    int Id = 0;
    Payload() = default;
    explicit Payload(int i) : Id(i) {}
};

// 매개변수 타입이 하나씩만 있는 함수 넷
void g_ref  (Payload&)        {}
void g_cref (const Payload&)  {}
void g_rref (Payload&&)       {}
void g_crref(const Payload&&) {}

using Fr   = decltype(&g_ref);
using Fcr  = decltype(&g_cref);
using Frr  = decltype(&g_rref);
using Fcrr = decltype(&g_crref);

// is_invocable_v 는 "컴파일이 되는가" 를 컴파일 타임에 알려준다.
// 인자를 타입으로 적는다:  T&  = 좌측값,  T&&  = 우측값(xvalue),  T = prvalue
#define ROW(label, ARG)                                        \
    std::printf("  %-26s   %s      %s      %s      %s\n", label, \
        std::is_invocable_v<Fr,   ARG> ? "O" : "X",            \
        std::is_invocable_v<Fcr,  ARG> ? "O" : "X",            \
        std::is_invocable_v<Frr,  ARG> ? "O" : "X",            \
        std::is_invocable_v<Fcrr, ARG> ? "O" : "X")

int main() {
    std::printf("\n  인자 \\ 매개변수              T&   const T&   T&&   const T&&\n");
    std::printf("  ---------------------------------------------------------------\n");
    ROW("Payload a;        (a)",        Payload&);
    ROW("const Payload ca; (ca)",       const Payload&);
    ROW("std::move(a)",                 Payload&&);
    ROW("std::move(ca)",                const Payload&&);
    ROW("Payload(9)   prvalue",         Payload);
    std::putchar('\n');
    return 0;
}
