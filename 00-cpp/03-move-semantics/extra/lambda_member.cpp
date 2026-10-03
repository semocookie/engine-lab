// 멤버 함수 안의 람다는 멤버 변수를 어떻게 잡는가
//
//   [Hp] 처럼 멤버를 이름으로 잡을 수 있는가 (TRY_NAME 을 켜고 컴파일해 본다)
//   [=] 로 잡으면 멤버가 복사되는가, 아니면 다른 것을 잡는가
//
//   ⚠ 3번은 이미 지운 객체를 람다로 읽는 미정의 동작이다. Debug 로 돌릴 것.

#include <cstdio>
#include "dump.h"

static int GCount = 7;                    // 전역 변수 (지역 변수가 아니다)

struct Actor {
    int    Hp = 100;
    int    Mp = 30;
    double Stats[2] = {};

    void Make();
    auto MakeEq()       { return [=]       { return Hp; }; }
    auto MakeThisCopy() { return [*this]   { return Hp; }; }
    auto MakeInit()     { return [Hp = Hp] { return Hp; }; }
};

void Actor::Make() {
#ifdef TRY_NAME
    auto f1 = [Hp] { return Hp; };       // 가.
#endif
    auto f2 = [=] { return Hp; };        // 나.
    std::printf("  나. sizeof([=] 람다)             = %zu\n", sizeof(f2));
    Hp = 50;
    std::printf("  다. Hp 를 50 으로 바꾼 뒤 f2()   = %d\n", f2());
    Hp = 100;
}

int main() {
    lab::section("1. 멤버 함수 안에서 [=] 로 잡으면");
    Actor a;
    std::printf("  (참고) sizeof(Actor)             = %zu\n", sizeof(Actor));
    a.Make();

    lab::section("2. 정말 복사하고 싶다면");
    auto fc = a.MakeThisCopy();
    auto fi = a.MakeInit();
    a.Hp = 50;
    std::printf("  [*this]   sizeof = %-3zu  Hp 를 50 으로 바꾼 뒤 = %d\n", sizeof(fc), fc());
    std::printf("  [Hp = Hp] sizeof = %-3zu  Hp 를 50 으로 바꾼 뒤 = %d\n", sizeof(fi), fi());
    a.Hp = 100;

    lab::section("3. 객체가 먼저 사라지면");
    Actor* h = new Actor;
    auto fe = h->MakeEq();
    std::printf("  지우기 전 fe() = %d\n", fe());
    delete h;
    std::printf("  지운 뒤   fe() = %d  (0x%08X)\n", fe(), static_cast<unsigned>(fe()));

    lab::section("4. 전역 변수는");
    auto g = [] { return GCount; };      // 캡처 목록이 비어 있다
    std::printf("  sizeof = %zu   g() = %d\n", sizeof(g), g());
    GCount = 8;
    std::printf("  GCount 를 8 로 바꾼 뒤 g() = %d\n", g());

    std::putchar('\n');
    return 0;
}
