#include "dump.h"   // 콘솔 UTF-8 설정을 겸한다
#include <cstdio>
#include <string>
#include <utility>

struct Payload {
    std::string Data = std::string(64, 'x');
    int Id = 0;
    Payload() = default;
    explicit Payload(int i) : Id(i) {}
    Payload(const Payload& o) : Data(o.Data), Id(o.Id) {}
    Payload(Payload&& o) noexcept : Data(std::move(o.Data)), Id(o.Id) {}
};

int main() {
    Payload a(1), b(2);
    auto L1 = [a]                { return a.Id; };   // 값
    auto L2 = [&a]               { return a.Id; };   // 참조
    auto L3 = [b = std::move(b)] { return b.Id; };   // 이동

    std::printf("\n  sizeof(Payload)      = %zu\n\n", sizeof(Payload));
    std::printf("  [a]                    값 캡처   sizeof = %zu\n", sizeof(L1));
    std::printf("  [&a]                   참조 캡처 sizeof = %zu\n", sizeof(L2));
    std::printf("  [b = std::move(b)]     이동 캡처 sizeof = %zu\n\n", sizeof(L3));
    (void)L1; (void)L2; (void)L3;
    return 0;
}
