// ════════════════════════════════════════════════════════════════
//  00-cpp/01-object-layout — 객체 메모리 레이아웃
// ════════════════════════════════════════════════════════════════
//
//  목표 : 구조체 크기를 "예측한 뒤 실측으로 맞힌다"
//
//  ⚠ 실행하기 전에 RESULT.md 의 [예측] 칸을 먼저 채운다.
//    예측 없이 실행하면 이 실습은 아무 의미가 없다.
//    틀린 예측이 이 폴더에서 가장 값어치 있는 산출물이다.
//
//  TODO 는 3개다. 전부 끝내면 프로그램이 "완료"를 찍는다.
// ════════════════════════════════════════════════════════════════

#include "dump.h"
#include <cstdint>

// ─────────────────────────────────────────────────────────────
//  실험 2 : 같은 멤버, 다른 순서
// ─────────────────────────────────────────────────────────────
struct BadOrder {
    char   a;
    double b;
    char   c;
    int    d;
};

// TODO 1 ── 멤버를 지우거나 바꾸지 말고 **순서만** 바꿔서
//           sizeof 를 최소로 만들어라.
//           힌트: 정렬 요구가 큰 것부터.
struct GoodOrder {
    double b;
    int    d;
    char   a;
    char   c;
};

// ─────────────────────────────────────────────────────────────
//  실험 3 : 가상 함수가 붙으면
// ─────────────────────────────────────────────────────────────
struct Plain {
    int x;
};

struct Poly {
    int x;
    virtual void speak() { std::printf("  Poly::speak\n"); }
    virtual ~Poly() = default;
};

struct PolyChild : Poly {
    void speak() override { std::printf("  PolyChild::speak\n"); }
};

// ─────────────────────────────────────────────────────────────
//  실험 4 : 빈 구조체
// ─────────────────────────────────────────────────────────────
struct Empty {};
struct DerivedFromEmpty : Empty { int x; };   // EBO(Empty Base Optimization)

// ─────────────────────────────────────────────────────────────
//  실험 5 : 정렬을 강제로 끄면
// ─────────────────────────────────────────────────────────────
#pragma pack(push, 1)
struct Packed {
    char   a;
    double b;
    char   c;
    int    d;
};
#pragma pack(pop)

// ─────────────────────────────────────────────────────────────
//  실험 6 : 실전 — 언리얼에서 흔한 모양의 구조체
// ─────────────────────────────────────────────────────────────
struct MonsterState {
    bool     bIsAlive;
    double   LastSeenTime;
    uint8_t  Stance;
    float    Health;
    bool     bCrouched;
    void*    Target;
    uint8_t  TeamId;
    float    ViewAngle;
};

// TODO 2 ── 위와 같은 멤버로, 순서만 바꿔 크기를 줄여라.
struct MonsterStateTight {
    bool     bIsAlive;
    double   LastSeenTime;
    uint8_t  Stance;
    float    Health;
    bool     bCrouched;
    void*    Target;
    uint8_t  TeamId;
    float    ViewAngle;
};


int main() {
    // ── 실험 1 : 기본 타입 ──────────────────────────────────
    lab::section("실험 1 — 기본 타입의 크기와 정렬");
    LAYOUT(char);
    LAYOUT(short);
    LAYOUT(int);
    LAYOUT(long);          // Windows 와 Linux 에서 다르다. 왜?
    LAYOUT(long long);
    LAYOUT(float);
    LAYOUT(double);
    LAYOUT(void*);
    LAYOUT(bool);          // 1비트면 충분한데 왜 1바이트인가?

    // ── 실험 2 : 멤버 순서 ──────────────────────────────────
    lab::section("실험 2 — 같은 멤버, 다른 순서");
    lab::layout_map("BadOrder", sizeof(BadOrder), {
        FIELD(BadOrder, a), FIELD(BadOrder, b),
        FIELD(BadOrder, c), FIELD(BadOrder, d),
    });
    std::putchar('\n');
    lab::layout_map("GoodOrder", sizeof(GoodOrder), {
        FIELD(GoodOrder, a), FIELD(GoodOrder, b),
        FIELD(GoodOrder, c), FIELD(GoodOrder, d),
    });

    // ── 실험 3 : vptr ───────────────────────────────────────
    lab::section("실험 3 — 가상 함수가 붙으면 크기가 변한다");
    LAYOUT(Plain);
    LAYOUT(Poly);
    LAYOUT(PolyChild);
    std::printf("\n  Plain 과 Poly 의 차이 = %zu 바이트.\n",
                sizeof(Poly) - sizeof(Plain));
    std::printf("  주의: 이 %zu 가 전부 포인터는 아니다. 나머지는 무엇인가?\n\n",
                sizeof(Poly) - sizeof(Plain));

    Poly      p;
    PolyChild c;
    p.x = 0x11111111;
    c.x = 0x22222222;

    // 객체의 첫 8바이트를 그대로 들여다본다.
    lab::hexdump(&p, sizeof(Poly),      "Poly");
    lab::hexdump(&c, sizeof(PolyChild), "PolyChild");
    std::printf("\n  → 앞 8바이트가 서로 다르다. 이 값이 무엇을 가리키는가?\n");
    std::printf("  → x 값(11111111 / 22222222)이 몇 번째 바이트부터 나오는가?\n");

    // TODO 3 ── 객체 슬라이싱을 재현하라.
    //   Poly sliced = c;  처럼 값으로 대입한 뒤 sliced.speak() 를 부르면
    //   무엇이 출력되는가? 먼저 예측하고, 아래 주석을 풀어 확인하라.
    //   그리고 참조/포인터로 부른 경우와 비교하라.
    //
    // std::printf("\n  [슬라이싱]\n");
    // Poly sliced = c;
    // sliced.speak();
    // Poly& ref = c;
    // ref.speak();
    // lab::hexdump(&sliced, sizeof(Poly), "sliced");

    // ── 실험 4 : 빈 구조체 ──────────────────────────────────
    lab::section("실험 4 — 빈 구조체");
    LAYOUT(Empty);              // 0이 아니다. 왜 0이면 안 되는가?
    LAYOUT(DerivedFromEmpty);   // Empty + int 인데 왜 이 크기인가?

    // ── 실험 5 : #pragma pack ───────────────────────────────
    lab::section("실험 5 — 정렬을 강제로 끄면");
    LAYOUT(BadOrder);
    LAYOUT(Packed);
    std::printf("\n  작아졌다. 그런데 왜 항상 이렇게 하지 않는가?\n");
    std::printf("  (네트워크 패킷/파일 포맷에서는 왜 오히려 이걸 쓰는가?)\n");

    // ── 실험 6 : 실전 ───────────────────────────────────────
    lab::section("실험 6 — 실전: 몬스터 상태 구조체");
    lab::layout_map("MonsterState", sizeof(MonsterState), {
        FIELD(MonsterState, bIsAlive),  FIELD(MonsterState, LastSeenTime),
        FIELD(MonsterState, Stance),    FIELD(MonsterState, Health),
        FIELD(MonsterState, bCrouched), FIELD(MonsterState, Target),
        FIELD(MonsterState, TeamId),    FIELD(MonsterState, ViewAngle),
    });
    std::putchar('\n');
    lab::layout_map("MonsterStateTight", sizeof(MonsterStateTight), {
        FIELD(MonsterStateTight, bIsAlive),  FIELD(MonsterStateTight, LastSeenTime),
        FIELD(MonsterStateTight, Stance),    FIELD(MonsterStateTight, Health),
        FIELD(MonsterStateTight, bCrouched), FIELD(MonsterStateTight, Target),
        FIELD(MonsterStateTight, TeamId),    FIELD(MonsterStateTight, ViewAngle),
    });

    const std::size_t saved = sizeof(MonsterState) - sizeof(MonsterStateTight);
    std::printf("\n  절약: %zu 바이트/개체\n", saved);
    std::printf("  몬스터 1,000마리 기준: %zu KB\n", saved * 1000 / 1024);
    std::printf("  → 이 숫자가 RESULT.md 에 들어갈 값이다.\n");

    // ── 남은 TODO 확인 ──────────────────────────────────────
    lab::section("남은 TODO");
    int remaining = 0;
    if (sizeof(GoodOrder) >= sizeof(BadOrder)) {
        std::printf("  [ ] TODO 1 — GoodOrder 멤버 순서 재배치 (현재 %zu, 목표 %zu 이하)\n",
                    sizeof(GoodOrder), sizeof(BadOrder) - 8);
        ++remaining;
    } else {
        std::printf("  [x] TODO 1 — %zu → %zu 바이트\n",
                    sizeof(BadOrder), sizeof(GoodOrder));
    }

    if (sizeof(MonsterStateTight) >= sizeof(MonsterState)) {
        std::printf("  [ ] TODO 2 — MonsterStateTight 재배치 (현재 %zu)\n",
                    sizeof(MonsterStateTight));
        ++remaining;
    } else {
        std::printf("  [x] TODO 2 — %zu → %zu 바이트\n",
                    sizeof(MonsterState), sizeof(MonsterStateTight));
    }

    std::printf("  [ ] TODO 3 — 슬라이싱 주석 해제하고 확인 (직접 체크)\n");

    std::putchar('\n');
    if (remaining == 0)
        std::printf("  TODO 1·2 완료. RESULT.md 의 [실측] 칸을 채우고,\n"
                    "  자료를 닫은 채로 README.md 를 써라.\n");
    else
        std::printf("  남은 TODO: %d개\n", remaining);

    std::putchar('\n');
    return 0;
}
