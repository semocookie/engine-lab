// 참조 캡처가 대상보다 오래 살면 무슨 일이 일어나는가
//
//  Magic 값을 표식으로 쓴다.
//    생성자가 0xABCD 를 적고
//    소멸자가 0xDEAD 로 덮는다
//  람다가 읽어오는 값이 셋 중 무엇인지가 답이다.

#include "dump.h"   // 콘솔 UTF-8 설정을 겸한다
#include <cstdio>
#include <functional>

static int g_Ctor = 0, g_Dtor = 0;

struct Tracer {
    int Id;
    int Magic;
    explicit Tracer(int i) : Id(i), Magic(0xABCD) { ++g_Ctor; }
    Tracer(const Tracer& o) : Id(o.Id), Magic(o.Magic) { ++g_Ctor; }
    ~Tracer() { Magic = 0xDEAD; ++g_Dtor; }
};

//  참조 캡처 — t 는 이 함수를 벗어나는 순간 죽는다
static std::function<int()> MakeRef(const void** OutAddr) {
    Tracer t(1);
    *OutAddr = &t;
    return [&t] { return t.Magic; };
}

//  값 캡처 — 사본을 들고 나간다 (대조군)
static std::function<int()> MakeVal() {
    Tracer t(2);
    return [t] { return t.Magic; };
}

//  스택 프레임을 한 번 헤집는다. 죽은 자리를 다른 것이 덮어쓰게 만든다.
static int Disturb(int depth) {
    volatile int pad[64];
    for (int i = 0; i < 64; ++i) { pad[i] = depth * 7919 + i; }
    if (depth > 0) { return Disturb(depth - 1) + pad[3]; }
    return pad[0];
}

int main() {
    std::printf("\n[1] 참조 캡처 — 만든 함수를 빠져나온 뒤\n\n");
    const void* addr = nullptr;
    g_Ctor = g_Dtor = 0;
    std::function<int()> f = MakeRef(&addr);

    std::printf("      MakeRef 를 빠져나온 직후   생성 %d   소멸 %d\n", g_Ctor, g_Dtor);
    std::printf("      람다가 보고 있는 주소      %p\n\n", addr);

    std::printf("      (1-a) 바로 호출          Magic = 0x%04X\n", f() & 0xFFFF);

    Disturb(8);
    std::printf("      (1-b) 스택을 헤집은 뒤    Magic = 0x%04X\n", f() & 0xFFFF);

    std::printf("\n[2] 값 캡처 — 대조군\n\n");
    {
        std::function<int()> g = MakeVal();
        std::printf("      (2-a) 바로 호출          Magic = 0x%04X\n", g() & 0xFFFF);
        Disturb(8);
        std::printf("      (2-b) 스택을 헤집은 뒤    Magic = 0x%04X\n", g() & 0xFFFF);
    }

    std::printf("\n[3] 표식\n\n");
    std::printf("      0xABCD  생성자가 적은 값 — 살아 있는 객체\n");
    std::printf("      0xDEAD  소멸자가 덮은 값 — 이미 죽었다\n");
    std::printf("      그 외    남의 것이 덮어썼다\n\n");
    return 0;
}
