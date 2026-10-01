// unique_ptr 는 포인터보다 큰가
//
//   Tracer* 와 std::unique_ptr<Tracer> 의 크기를 잰다.
//   그리고 안에 무엇이 들어 있는지 바이트를 직접 찍어 본다.

#include <cstdio>
#include <memory>
#include "dump.h"

struct Tracer { const char* Name; };

static void FreeTracer(Tracer* p) { delete p; }
inline auto FreeLambda = [](Tracer* p) { delete p; };

using RawPtr        = Tracer*;
using Unique        = std::unique_ptr<Tracer>;
using UniqueFnPtr   = std::unique_ptr<Tracer, void (*)(Tracer*)>;   // 삭제자가 함수 포인터
using UniqueLambda  = std::unique_ptr<Tracer, decltype(FreeLambda)>; // 삭제자가 캡처 없는 람다
using Shared        = std::shared_ptr<Tracer>;

int main() {
    lab::section("1. 크기");
    std::printf("  %-44s sizeof=%zu\n", "Tracer*",                                sizeof(RawPtr));
    std::printf("  %-44s sizeof=%zu\n", "std::unique_ptr<Tracer>",                sizeof(Unique));

    lab::section("2. 안에 무엇이 들어 있나");
    Unique u = std::make_unique<Tracer>();
    std::printf("  u.get() = %p\n", static_cast<void*>(u.get()));
    lab::hexdump(&u, sizeof(u), "u  :");

    lab::section("3. 크기가 달라지는 경우");
    std::printf("  %-44s sizeof=%zu\n", "unique_ptr<Tracer, 함수 포인터 삭제자>",  sizeof(UniqueFnPtr));
    std::printf("  %-44s sizeof=%zu\n", "unique_ptr<Tracer, 캡처 없는 람다 삭제자>", sizeof(UniqueLambda));
    std::printf("  %-44s sizeof=%zu\n", "std::shared_ptr<Tracer>",                sizeof(Shared));

    UniqueFnPtr uf(new Tracer{}, &FreeTracer);
    std::printf("\n  uf.get() = %p   FreeTracer = %p\n",
                static_cast<void*>(uf.get()), reinterpret_cast<void*>(&FreeTracer));
    lab::hexdump(&uf, sizeof(uf), "uf :");

    std::putchar('\n');
    return 0;
}
