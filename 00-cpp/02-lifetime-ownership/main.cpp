// ════════════════════════════════════════════════════════════════
//  00-cpp/02-lifetime-ownership — 수명과 소유권
// ════════════════════════════════════════════════════════════════
//
//  목표 : 객체가 "언제" 사라지는지를 출력 순서로 확인한다.
//         주제 1에서 본 vptr 이 소멸 과정에서 어떻게 되돌려지는지까지.
//
//  ⚠ 실행하기 전에 RESULT.md 의 [예측] 칸을 먼저 채운다.
//    이 주제의 예측은 대부분 숫자가 아니라 **출력 순서**다.
//    순서를 종이에 적어놓고 실행해야 의미가 있다.
//
//  TODO 는 3개다. 하나씩 끝낼 때마다 마지막 줄의 "살아남은 개수"가 줄어든다.
// ════════════════════════════════════════════════════════════════

#include "dump.h"
#include <memory>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────
//  도구 : 생성과 소멸을 눈으로 보기 위한 것
//         살아있는 개수를 세어두므로 "누수"가 숫자로 나온다.
// ─────────────────────────────────────────────────────────────
static int g_Alive = 0;

struct Tracer {
    const char* Name;

    explicit Tracer(const char* InName) : Name(InName) {
        ++g_Alive;
        std::printf("      + %s\n", Name);
    }
    ~Tracer() {
        --g_Alive;
        std::printf("      - %s\n", Name);
    }

    Tracer(const Tracer&) = delete;
    Tracer& operator=(const Tracer&) = delete;
};

// ─────────────────────────────────────────────────────────────
//  실험 1 : 생성과 소멸의 순서
// ─────────────────────────────────────────────────────────────
struct Engine {
    Engine()  { std::printf("      + Engine (멤버)\n"); }
    ~Engine() { std::printf("      - Engine (멤버)\n"); }
};

struct Vehicle {
    Vehicle()          { std::printf("      + Vehicle (기반)\n"); }
    virtual ~Vehicle() { std::printf("      - Vehicle (기반)\n"); }
};

struct Car : Vehicle {
    Engine engine;                                    // 멤버
    Car()           { std::printf("      + Car (파생)\n"); }
    ~Car() override { std::printf("      - Car (파생)\n"); }
};

// ─────────────────────────────────────────────────────────────
//  실험 2 : 생성자·소멸자 안에서 가상 함수를 부르면
//           주제 1에서 본 vptr 이 여기서 다시 나온다.
// ─────────────────────────────────────────────────────────────
struct Actor {
    Actor()          { std::printf("      Actor 생성자 안 -> "); WhoAmI(); }
    virtual ~Actor() { std::printf("      Actor 소멸자 안 -> "); WhoAmI(); }
    virtual void WhoAmI() const { std::printf("Actor::WhoAmI\n"); }
};

struct Pawn : Actor {
    void WhoAmI() const override { std::printf("Pawn::WhoAmI\n"); }
};

// ─────────────────────────────────────────────────────────────
//  실험 3 : 가상 소멸자가 없으면
// ─────────────────────────────────────────────────────────────
struct NoVirtualBase {
    ~NoVirtualBase() { std::printf("      - NoVirtualBase\n"); }   // virtual 이 없다
};
struct NoVirtualChild : NoVirtualBase {
    Tracer Held{"NoVirtualChild 가 쥔 자원"};
    ~NoVirtualChild() { std::printf("      - NoVirtualChild\n"); }
};

// TODO 3 ── 아래 소멸자에 virtual 하나만 붙이고 다시 실행하라.
//           위(NoVirtual~)와 출력이 어떻게 달라지는지 비교한다.
struct VirtualBase {
    virtual ~VirtualBase() { std::printf("      - VirtualBase\n"); }
};
struct VirtualChild : VirtualBase {
    Tracer Held{"VirtualChild 가 쥔 자원"};
    ~VirtualChild() override { std::printf("      - VirtualChild\n"); }
};

// ─────────────────────────────────────────────────────────────
//  실험 4 : 중간에 빠져나갈 때
// ─────────────────────────────────────────────────────────────
void ManualDelete(bool bFail) {
    Tracer* p = new Tracer("ManualDelete 가 만든 것");
    if (bFail) {
        throw std::runtime_error("중간에 실패");   // delete 를 지나치지 못한다
    }
    delete p;
}

// TODO 1 ── 위 함수와 같은 일을 하되, 실패해도 새지 않게 고쳐라.
//           힌트: 소멸자가 대신 해주게 만든다. new 와 delete 를 지운다.
void RaiiVersion(bool bFail) {
    std::unique_ptr<Tracer> p = std::make_unique<Tracer>("RaiiVersion 이 만든 것");
    if (bFail) {
        throw std::runtime_error("중간에 실패");
    }
}

// ─────────────────────────────────────────────────────────────
//  실험 6 : 서로를 가리키면
// ─────────────────────────────────────────────────────────────
struct Node {
    Tracer Mark;
    std::shared_ptr<Node> Next;

    // TODO 2 ── 아래 한 줄 때문에 두 노드가 영영 안 사라진다.
    //           둘 중 한쪽만 소유를 포기하게 만들어라.
    std::weak_ptr<Node> Prev;

    explicit Node(const char* InName) : Mark(InName) {}
};


int main() {
    // ── 실험 1 ──────────────────────────────────────────────
    lab::section("실험 1 — 생성과 소멸의 순서");
    std::printf("  Car 를 스코프 안에서 만들었다가 버린다.\n\n");
    {
        Car c;
        std::printf("      ... 스코프 안 ...\n");
    }

    // ── 실험 2 ──────────────────────────────────────────────
    lab::section("실험 2 — 생성자·소멸자 안에서 가상 함수를 부르면");
    std::printf("  Pawn 을 만든다. Pawn 은 WhoAmI 를 재정의했다.\n\n");
    {
        Pawn p;
        std::printf("      다 만들어진 뒤 호출 -> ");
        p.WhoAmI();
    }
    std::printf("\n  -> 세 번의 호출 중 몇 번이 Pawn::WhoAmI 였는가?\n");

    // ── 실험 3 ──────────────────────────────────────────────
    lab::section("실험 3 — 가상 소멸자가 없으면");
    std::printf("  기반 클래스 포인터로 받아서 delete 한다.\n\n");
    {
        std::printf("    [virtual 없음]\n");
        NoVirtualBase* p = new NoVirtualChild();
        delete p;      // 표준상 정의되지 않은 동작이다. 관찰용으로만 둔다.

        std::printf("\n    [TODO 3 대상]\n");
        VirtualBase* q = new VirtualChild();
        delete q;
    }

    //  virtual 소멸자의 대가를 크기로 확인한다.
    //  두 클래스 모두 멤버 변수가 하나도 없다. 차이는 virtual 하나뿐이다.
    std::putchar('\n');
    LAYOUT(NoVirtualBase);
    LAYOUT(VirtualBase);
    std::printf("\n  -> 멤버가 없는데 왜 다른가? 주제 1에서 Plain 과 Poly 를 비교한 그것이다.\n");

    // ── 실험 4 ──────────────────────────────────────────────
    lab::section("실험 4 — 중간에 빠져나갈 때");
    std::printf("  두 함수 모두 실패시킨다. 소멸(-) 줄이 찍히는지 본다.\n\n");
    {
        std::printf("    [ManualDelete]\n");
        try { ManualDelete(true); } catch (const std::exception&) { std::printf("      (예외 잡음)\n"); }

        std::printf("\n    [RaiiVersion]\n");
        try { RaiiVersion(true); }  catch (const std::exception&) { std::printf("      (예외 잡음)\n"); }
    }

    // ── 실험 5 ──────────────────────────────────────────────
    lab::section("실험 5 — 스마트 포인터의 크기와 소유권 이전");
    LAYOUT(int*);
    LAYOUT(std::unique_ptr<int>);
    LAYOUT(std::shared_ptr<int>);
    std::printf("\n  -> unique_ptr 이 생포인터와 같은 크기인 이유는? (주제 1의 EBO)\n");
    std::printf("  -> shared_ptr 이 더 큰 이유는 무엇을 더 들고 있어서인가?\n\n");
    {
        auto a = std::make_unique<Tracer>("unique_ptr 이 소유한 것");
        std::printf("      이동 전 : a=%s\n", a ? "가리킴" : "비었음");
        auto b = std::move(a);
        std::printf("      이동 후 : a=%s  b=%s\n",
                    a ? "가리킴" : "비었음", b ? "가리킴" : "비었음");
         //auto c = b;   // 주석을 풀면 컴파일 에러. 에러 메시지를 RESULT.md 에 적어둔다.
    }

    std::putchar('\n');
    {
        auto s1 = std::make_shared<Tracer>("shared_ptr 이 소유한 것");
        std::printf("      s1 만든 직후      use_count = %ld\n", s1.use_count());
        {
            auto s2 = s1;
            std::printf("      s2 로 복사한 뒤   use_count = %ld\n", s1.use_count());
        }
        std::printf("      s2 가 사라진 뒤   use_count = %ld\n", s1.use_count());
    }

    // ── 실험 6 ──────────────────────────────────────────────
    lab::section("실험 6 — 서로를 가리키면");
    std::printf("  A 와 B 가 서로를 가리키게 한다. 소멸(-) 줄이 찍히는가?\n\n");
    {
        auto A = std::make_shared<Node>("Node A");
        auto B = std::make_shared<Node>("Node B");
        A->Next = B;
        B->Prev = A;
        std::printf("      연결 후  A.use_count = %ld   B.use_count = %ld\n",
                    A.use_count(), B.use_count());
        std::printf("      ... 스코프를 벗어난다 ...\n");
    }
    std::printf("\n  -> 위에서 '- Node A' 가 찍혔는가?\n");

    // ── 결산 ────────────────────────────────────────────────
    lab::section("결산");
    std::printf("  살아남은 Tracer : %d 개\n\n", g_Alive);
    std::printf("  이 중 둘은 의도된 누수다. 고치지 않고 대조군으로 남긴다.\n");
    std::printf("    - NoVirtualChild 가 쥔 자원   (실험 3 대조군)\n");
    std::printf("    - ManualDelete 가 만든 것     (실험 4 대조군)\n\n");
    std::printf("  나머지는 TODO 1 · 2 · 3 을 끝내면 사라진다.\n");
    std::printf("  몇 개가 남아야 맞는지는 RESULT.md 에 먼저 예측해두고 확인한다.\n");

    std::putchar('\n');
    return 0;
}
