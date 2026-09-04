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

// ── A. 반환형이 무엇인가 ──────────────────────────────────
#define SHOW(expr)                                                            \
    std::printf("  %-22s   Payload&&: %d    const Payload&&: %d\n", #expr,    \
        (int)std::is_same_v<decltype(expr), Payload&&>,                       \
        (int)std::is_same_v<decltype(expr), const Payload&&>)

// ── B. 그 타입이 어느 매개변수에 들어가는가 ────────────────
namespace Full {          // 후보 4개 전부
    void f(Payload&)        { std::printf("Payload&\n"); }
    void f(const Payload&)  { std::printf("const Payload&\n"); }
    void f(Payload&&)       { std::printf("Payload&&\n"); }
    void f(const Payload&&) { std::printf("const Payload&&\n"); }
}
namespace NoMove {        // 이동 생성자에 해당하는 Payload&& 를 뺀 경우
    void f(Payload&)        { std::printf("Payload&\n"); }
    void f(const Payload&)  { std::printf("const Payload&\n"); }
    void f(const Payload&&) { std::printf("const Payload&&\n"); }
}
namespace CopyOnly {      // 어제의 Payload 상태 : 복사 생성자 하나뿐
    void f(const Payload&)  { std::printf("const Payload&\n"); }
}

int main() {
    Payload a(1);
    const Payload ca(2);

    std::printf("\n[A] std::move 가 돌려주는 타입\n\n");
    SHOW(std::move(a));
    SHOW(std::move(ca));

    std::printf("\n[B] 그 표현식이 어느 매개변수로 들어가는가\n\n");
    std::printf("  후보 4개  f(std::move(a))   -> "); Full::f(std::move(a));
    std::printf("  후보 4개  f(std::move(ca))  -> "); Full::f(std::move(ca));
    std::printf("  &&제외    f(std::move(a))   -> "); NoMove::f(std::move(a));
    std::printf("  복사만    f(std::move(a))   -> "); CopyOnly::f(std::move(a));

    std::printf("\n[C] 좌측값을 Payload&& 에 넘길 수 있는가\n");
    std::printf("  아래 줄의 주석을 풀면 컴파일이 되는가?\n");
    // Full::f 는 Payload& 후보가 있어 되므로, 우측값 참조만 있는 함수로 따로 본다.
    std::putchar('\n');
    return 0;
}
