// typeid 도 vptr 을 따라가는가
//
//   "이 객체가 실제로 무엇인가" 를 묻는 typeid 가
//   객체 자체를 보는지, 맨 앞 8바이트(vptr)를 보는지 확인한다.
//
//   ⚠ 3번은 vptr 을 손으로 바꿔치기한다. 정의되지 않은 동작이고
//     MSVC x64 구현에 기댄 코드다. 들여다보기 전용.
//   ⚠ Debug(/Od) 로 돌릴 것.

#include <cstdio>
#include <cstring>
#include <typeinfo>
#include "dump.h"

struct Actor {
    virtual ~Actor()             {}
    virtual void WhoAmI() const  { std::printf("Actor::WhoAmI\n"); }
};

struct Pawn : Actor {
    void WhoAmI() const override { std::printf("Pawn::WhoAmI\n"); }
};

// 컴파일러가 "이건 Actor 다" 라고 미리 알아버리지 못하게
// 참조로 받아 함수 경계를 한 번 넘긴다.
__declspec(noinline) static const char* NameVia(const Actor& r) { return typeid(r).name(); }
__declspec(noinline) static void        CallVia(const Actor& r) { r.WhoAmI(); }

int main() {
    Pawn pawn;

    lab::section("1. Actor* p = &pawn;");
    Actor* p = &pawn;
    std::printf("  typeid(*p)          : %s\n", typeid(*p).name());
    std::printf("  p->WhoAmI()         : ");  p->WhoAmI();

    lab::section("2. Actor sliced = pawn;");
    Actor sliced = pawn;
    std::printf("  typeid(sliced) 직접 : %s\n", typeid(sliced).name());
    std::printf("  참조로 넘겨서       : %s\n", NameVia(sliced));
    std::printf("  참조로 WhoAmI       : ");  CallVia(sliced);

    lab::section("3. 진짜 Actor 의 vptr 을 Pawn 것으로 바꿔치기");
    Actor a;
    void* original;
    std::memcpy(&original, static_cast<void*>(&a), sizeof(void*));          // 원래 vptr 보관
    std::memcpy(static_cast<void*>(&a), static_cast<void*>(&pawn), sizeof(void*)); // Pawn 의 vptr 로 덮어씀

    std::printf("  typeid(a) 직접      : %s\n", typeid(a).name());
    std::printf("  참조로 넘겨서       : %s\n", NameVia(a));
    std::printf("  a.WhoAmI() 직접     : ");  a.WhoAmI();
    std::printf("  참조로 WhoAmI       : ");  CallVia(a);

    std::memcpy(static_cast<void*>(&a), &original, sizeof(void*));          // 되돌려 놓는다
    std::putchar('\n');
    return 0;
}
