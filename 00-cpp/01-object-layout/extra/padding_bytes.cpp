// 패딩에는 무엇이 들어 있나
//
//   struct A { float b; bool c; int d; } 의 5~7번(패딩 3바이트)을
//   만드는 방법에 따라 찍어 본다.
//
//   ⚠ Debug 와 Release 에서 답이 다르다. 둘 다 돌려 볼 것.

#include <cstdio>
#include "dump.h"

struct A { float b; bool c; int d; };

static void Show(const char* Label, const A* p) {
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(p);
    std::printf("  %-26s ", Label);
    for (int i = 0; i < static_cast<int>(sizeof(A)); ++i) {
        if (i == 5) std::printf("[");
        std::printf("%02X", bytes[i]);
        if (i == 7) std::printf("]");
        std::printf(" ");
    }
    std::printf("\n");
}

// ── 자리를 먼저 더럽혀 둔다. 0 이 "원래 0" 인지 "아직 아무도 안 써서 0" 인지 가르기 위해 ──
__declspec(noinline) static void DirtyStack() {
    volatile unsigned char Junk[512];
    for (int i = 0; i < 512; ++i) { Junk[i] = 0xAB; }
}
__declspec(noinline) static void MakeOnStack() {
    A a;
    a.b = 1.0f; a.c = true; a.d = 7;
    Show("가'. 스택을 더럽힌 뒤", &a);
}
__declspec(noinline) static void MakeOnHeapAfterDirty() {
    void* Junk = ::operator new(sizeof(A));
    volatile unsigned char* j = static_cast<volatile unsigned char*>(Junk);
    for (int i = 0; i < static_cast<int>(sizeof(A)); ++i) { j[i] = 0xAB; }
    ::operator delete(Junk);                      // 방금 더럽힌 칸을 돌려준다

    A* h = new A;                                 // 같은 칸을 다시 받을 가능성이 높다
    h->b = 1.0f; h->c = true; h->d = 7;
    std::printf("  (같은 칸을 받았나: %s)\n", static_cast<void*>(h) == Junk ? "예" : "아니오");
    Show("다'. 힙을 더럽힌 뒤", h);
    delete h;
}

int main() {
#ifdef _DEBUG
    lab::section("패딩(5~7번, [ ] 안)에 무엇이 들어 있나 — Debug");
#else
    lab::section("패딩(5~7번, [ ] 안)에 무엇이 들어 있나 — Release");
#endif
    std::printf("  %-26s b(0~3)      c  패딩       d(8~11)\n\n", "");

    A a;
    a.b = 1.0f; a.c = true; a.d = 7;
    Show("가. A a; 후 대입", &a);

    A z{};
    Show("나. A a{};", &z);

    A v = { 1.0f, true, 7 };
    Show("(덤) A a = {1, true, 7};", &v);

    A* h = new A;
    h->b = 1.0f; h->c = true; h->d = 7;
    Show("다. new A 후 대입", h);
    delete h;

    std::printf("\n  -- 자리를 0xAB 로 먼저 더럽힌 뒤 --\n");
    DirtyStack();
    MakeOnStack();
    MakeOnHeapAfterDirty();

    std::putchar('\n');
    return 0;
}
