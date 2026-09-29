// RTTI 를 끄면 무엇이 달라지나
//
//   같은 파일을 두 번 빌드한다.
//     object_layout_rtti_switch       RTTI 켬  (/GR,  기본값)
//     object_layout_rtti_switch_off   RTTI 끔  (/GR-, CMakeLists 에서 따로 지정)
//   두 출력을 나란히 놓고 본다.
//
//   ⚠ [3] 은 vptr 을 손으로 바꿔치기한다. 정의되지 않은 동작이고
//     MSVC x64 구현에 기댄 코드다. 들여다보기 전용.
//   ⚠ RTTI 끔 빌드에서는 경고 C4541 이 뜬다. 의도한 것이다.
//     (C++20 에서는 참조로 묻는 자리에만, C++17 로 직접 컴파일하면 직접 묻는 자리에도 뜬다)
//   ⚠ Debug(/Od) 로 돌릴 것.

#include <cstdio>
#include <cstring>
#include <typeinfo>
#include <exception>
#include "dump.h"

struct Actor { virtual ~Actor() {} virtual void WhoAmI() const { std::printf("Actor::WhoAmI\n"); } };
struct Pawn : Actor { void WhoAmI() const override { std::printf("Pawn::WhoAmI\n"); } };

// 참조로 받아 함수 경계를 넘긴다 — 컴파일러가 선언을 못 보게.
// RTTI 를 끄면 typeid 가 예외를 던지므로 잡아서 메시지를 찍는다.
__declspec(noinline) static void NameVia(const char* Label, const Actor& r) {
    std::printf("  %-22s: ", Label);
    try                               { std::printf("%s\n", typeid(r).name()); }
    catch (const std::exception& e)   { std::printf("예외 — %s\n", e.what()); }
    catch (...)                       { std::printf("예외 — (알 수 없음)\n"); }
}
__declspec(noinline) static void CallVia(const char* Label, const Actor& r) {
    std::printf("  %-22s: ", Label);  r.WhoAmI();
}

int main() {
#ifdef _CPPRTTI
    lab::section("RTTI 켬 (/GR)");
#else
    lab::section("RTTI 끔 (/GR-)");
#endif
    Pawn pawn;

    std::printf("\n [1] Actor* p = &pawn;\n");
    Actor* p = &pawn;
    NameVia("참조로 typeid", *p);
    CallVia("참조로 WhoAmI", *p);

    std::printf("\n [2] Actor sliced = pawn;\n");
    Actor sliced = pawn;
    std::printf("  %-22s: %s\n", "typeid 직접", typeid(sliced).name());
    NameVia("참조로 typeid", sliced);

    std::printf("\n [3] 진짜 Actor 의 vptr 을 Pawn 것으로\n");
    Actor a;
    void* original;
    std::memcpy(&original, static_cast<void*>(&a), sizeof(void*));
    std::memcpy(static_cast<void*>(&a), static_cast<void*>(&pawn), sizeof(void*));
    std::printf("  %-22s: %s\n", "typeid 직접", typeid(a).name());
    NameVia("참조로 typeid", a);
    CallVia("참조로 WhoAmI", a);
    std::memcpy(static_cast<void*>(&a), &original, sizeof(void*));

    std::putchar('\n');
    return 0;
}
