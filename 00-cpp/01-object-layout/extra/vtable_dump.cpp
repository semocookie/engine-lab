// vtable 안을 직접 들여다본다.
//
//   "vtable 은 함수 포인터 배열이다" 를 눈으로 확인한다.
//   객체 맨 앞 8바이트(vptr)를 읽어 그 너머를 배열로 본다.
//
//   ⚠ MSVC x64 구현에 기댄 코드다. 표준이 보장하지 않는다.
//     들여다보기 전용이지 실무에 쓸 것이 아니다.
//   ⚠ Debug(/Od) 로 돌릴 것. 최적화가 켜지면 테이블이 접힐 수 있다.

#include <cstdio>
#include "dump.h"

// ── 02 의 Actor / Pawn 과 같은 모양 ──────────────────────────
struct Actor {
    virtual ~Actor()             {}
    virtual void WhoAmI() const  { std::printf("Actor::WhoAmI\n"); }
};

struct Pawn : Actor {
    void WhoAmI() const override { std::printf("Pawn::WhoAmI\n"); }
    // 소멸자는 적지 않았다
};

// ── 비가상·정적 함수를 더하면 칸이 늘어날까 ──────────────────
struct OnlyVirtual {
    virtual ~OnlyVirtual() {}
    virtual void A() {}
};

struct PlusNonVirtual {
    virtual ~PlusNonVirtual() {}
    virtual void A() {}
    void B() {}             // 비가상
    void D() const {}       // 비가상
    static void C() {}      // 정적
};

// 객체 맨 앞 8바이트 = vptr. 그 너머를 함수 포인터 배열로 본다.
static void** VTableOf(const void* Obj) {
    return *reinterpret_cast<void** const*>(Obj);
}

int main() {
    lab::section("1. vtable 은 정말 클래스당 1개인가");

    Actor a1, a2, a3;
    Pawn  p1, p2;

    std::printf("  Actor a1  vptr = %p\n", (void*)VTableOf(&a1));
    std::printf("  Actor a2  vptr = %p\n", (void*)VTableOf(&a2));
    std::printf("  Actor a3  vptr = %p\n", (void*)VTableOf(&a3));
    std::printf("  Pawn  p1  vptr = %p\n", (void*)VTableOf(&p1));
    std::printf("  Pawn  p2  vptr = %p\n", (void*)VTableOf(&p2));
    std::printf("\n  Actor 끼리 같은가 : %s\n",
                VTableOf(&a1) == VTableOf(&a3) ? "같다" : "다르다");
    std::printf("  Actor 와 Pawn 은  : %s\n",
                VTableOf(&a1) == VTableOf(&p1) ? "같다" : "다르다");

    lab::section("2. 칸마다 무엇이 들어 있나");

    void** va = VTableOf(&a1);
    void** vp = VTableOf(&p1);

    std::printf("  Actor vtable [0] = %p   (소멸자 자리)\n", va[0]);
    std::printf("  Actor vtable [1] = %p   (WhoAmI 자리)\n", va[1]);
    std::printf("  Pawn  vtable [0] = %p   (소멸자 자리)\n", vp[0]);
    std::printf("  Pawn  vtable [1] = %p   (WhoAmI 자리)\n", vp[1]);
    std::printf("\n  [0] 소멸자 칸 : %s\n", va[0] == vp[0] ? "같다" : "다르다");
    std::printf("  [1] WhoAmI 칸 : %s\n", va[1] == vp[1] ? "같다" : "다르다");

    lab::section("3. 정말 함수 포인터인가 - 테이블에서 꺼내 직접 부른다");

    using WhoAmIFn = void(*)(const void*);

    std::printf("  보통 호출        : ");  a1.WhoAmI();
    std::printf("  테이블 [1] 직접  : ");
    reinterpret_cast<WhoAmIFn>(va[1])(&a1);

    std::printf("  보통 호출        : ");  p1.WhoAmI();
    std::printf("  테이블 [1] 직접  : ");
    reinterpret_cast<WhoAmIFn>(vp[1])(&p1);

    // Actor 객체에 Pawn 의 테이블 [1] 을 억지로 적용해 본다
    std::printf("\n  Actor 객체 + Pawn 테이블[1] : ");
    reinterpret_cast<WhoAmIFn>(vp[1])(&a1);

    // ⚠ [0] 은 부르지 않는다. MSVC 의 소멸자 칸은
    //   'vector deleting destructor' 라 delete 까지 해버린다.

    lab::section("4. 비가상 / 정적 함수를 더하면");

    OnlyVirtual    o;
    PlusNonVirtual q;

    LAYOUT(OnlyVirtual);
    LAYOUT(PlusNonVirtual);

    std::printf("\n  OnlyVirtual    [0]=%p [1]=%p\n",
                VTableOf(&o)[0], VTableOf(&o)[1]);
    std::printf("  PlusNonVirtual [0]=%p [1]=%p\n",
                VTableOf(&q)[0], VTableOf(&q)[1]);
    std::printf("\n  (칸 수는 cl /c /d1reportSingleClassLayoutPlusNonVirtual 로 확인)\n");

    std::putchar('\n');
    return 0;
}
