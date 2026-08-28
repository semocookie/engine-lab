// ════════════════════════════════════════════════════════════════
//  00-cpp/03-move-semantics — 이동 시맨틱 : 복사는 어디서 새는가
// ════════════════════════════════════════════════════════════════
//
//  목표 : "복사가 몇 번 일어났는가" 를 숫자로 본다.
//         주제 2에서 소멸을 세었듯이, 여기서는 복사를 센다.
//
//  ⚠ 실행하기 전에 RESULT.md 의 [예측] 칸을 먼저 채운다.
//    이 주제의 예측은 대부분 **횟수**다. 0 인지 1 인지 2 인지 적어두고 실행한다.
//
//  TODO 는 3개다. 하나씩 끝낼 때마다 마지막 줄의 "총 복사"가 줄어든다.
// ════════════════════════════════════════════════════════════════

#include "dump.h"
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// ─────────────────────────────────────────────────────────────
//  도구 : 복사와 이동을 세는 것
// ─────────────────────────────────────────────────────────────
static int g_Copy = 0;
static int g_Move = 0;
static int g_TotalCopy = 0;          // 결산용. 리셋하지 않는다.

static void Reset() { g_Copy = 0; g_Move = 0; }

static void Report(const char* Label) {
    std::printf("      %-38s 복사 %d   이동 %d\n", Label, g_Copy, g_Move);
}

//  일부러 무겁게 만든다. std::string 이 64자면 힙을 쓴다 (SSO 를 넘긴다).
struct Payload {
    std::string Data = std::string(64, 'x');
    int Id = 0;

    Payload() = default;
    explicit Payload(int InId) : Id(InId) {}

    Payload(const Payload& Other) : Data(Other.Data), Id(Other.Id) {
        ++g_Copy; ++g_TotalCopy;
    }
    Payload& operator=(const Payload& Other) {
        Data = Other.Data; Id = Other.Id;
        ++g_Copy; ++g_TotalCopy;
        return *this;
    }

    // TODO 1 ── 이동 생성자와 이동 대입을 추가하라.
    //           매개변수는 Payload&& 로 받고, 멤버는 std::move 로 옮긴다.
    //           ++g_Move 를 세는 것을 잊지 말 것.
    //
    // TODO 2 ── TODO 1 에서 만든 이동 생성자에 noexcept 를 붙여라.
    //           실험 6 의 결과가 바뀐다. 왜 바뀌는지가 이 주제의 핵심이다.

    Payload(Payload&& Other) noexcept
        : Data(std::move(Other.Data)), Id(Other.Id)
    {
        ++g_Move;
    }

    Payload& operator=(Payload&& Other) noexcept {
        Data = std::move(Other.Data);
        Id = Other.Id;
		++g_Move;
        return *this;
    }

    ~Payload() = default;
};

// ─────────────────────────────────────────────────────────────
//  실험 1 : 복사는 어디서 생기나
// ─────────────────────────────────────────────────────────────
static void TakeByValue(Payload p)           { (void)p; }
static void TakeByConstRef(const Payload& p) { (void)p; }

// ─────────────────────────────────────────────────────────────
//  실험 3 : 반환할 때
//
//  식(expression)은 셋 중 하나다. 반환하는 식이 어느 쪽이냐가 결과를 가른다.
//    lvalue  — 가리키는 객체가 있다.        훔쳐가면 안 된다
//    xvalue  — 가리키는 객체가 있다.        곧 사라지니 훔쳐가도 된다
//    prvalue — 가리킬 객체가 아직 없다.     "이런 값을 만들라"는 식일 뿐
// ─────────────────────────────────────────────────────────────
template <class T>
static constexpr const char* ValCat() {
    if constexpr (std::is_lvalue_reference_v<T>)      { return "lvalue   (신원 O, 이동 X)"; }
    else if constexpr (std::is_rvalue_reference_v<T>) { return "xvalue   (신원 O, 이동 O)"; }
    else                                              { return "prvalue  (신원 X, 이동 O)"; }
}
#define VALCAT(e) std::printf("      %-24s %s\n", #e, ValCat<decltype((e))>())

static Payload MakePrvalue() { return Payload(1); }              // 이름 없는 임시를 반환
static Payload MakeNamed()   { Payload p(2); return p; }         // 이름 있는 지역을 반환
static Payload MakeMoved()   { Payload p(3); return std::move(p); }  // 굳이 move 를 붙여 반환

// ─────────────────────────────────────────────────────────────
//  실험 4 : auto 가 만드는 복사
// ─────────────────────────────────────────────────────────────
static const Payload& GetConstRef() { static Payload s(9); return s; }

// ─────────────────────────────────────────────────────────────
//  실험 7 : 덤 — 정적 바인딩과 동적 바인딩
//           주제 1의 vptr, 주제 2의 가상 소멸자와 이어지는 자리다.
// ─────────────────────────────────────────────────────────────
struct Base {
    void         Plain() const { std::printf("Base::Plain\n"); }   // virtual 없음
    virtual void Virt()  const { std::printf("Base::Virt\n"); }
    virtual ~Base() = default;
};
struct Child : Base {
    void Plain() const { std::printf("Child::Plain\n"); }          // 가리기(hiding)
    void Virt()  const override { std::printf("Child::Virt\n"); }
};

//  컴파일러는 여기서 p 가 실제로 무엇인지 모른다. Base* 라는 것만 안다.
static void CallBoth(Base* p) {
    std::printf("      p->Plain() -> "); p->Plain();
    std::printf("      p->Virt()  -> "); p->Virt();
}

int main() {
    // ── 실험 1 ──────────────────────────────────────────────
    lab::section("실험 1 — 복사는 어디서 생기나");
    {
        Payload a(1);

        Reset(); TakeByValue(a);          Report("함수에 값으로 넘기기");
        Reset(); TakeByConstRef(a);       Report("함수에 const& 로 넘기기");

        Reset(); Payload b = a;           Report("다른 객체로 초기화");
        Reset(); b = a;                   Report("이미 있는 객체에 대입");

        std::vector<Payload> v;
        v.reserve(4);
        Reset(); v.push_back(a);          Report("vector 에 push_back(a)");
        Reset(); v.push_back(Payload(3)); Report("vector 에 push_back(임시)");
        Reset(); v.emplace_back(4);       Report("vector 에 emplace_back(4)");
    }

    // ── 실험 2 ──────────────────────────────────────────────
    lab::section("실험 2 — std::move 는 무엇을 옮기는가");
    std::printf("  std::move 를 썼는데 복사가 세어지는가?\n\n");
    {
        Payload a(1);
        Reset();
        Payload b = std::move(a);
        Report("Payload b = std::move(a)");
        std::printf("\n      옮긴 뒤   a.Data 길이 = %zu   b.Data 길이 = %zu\n",
                    a.Data.size(), b.Data.size());
        std::printf("\n  -> std::move 자체는 무엇을 하는 함수인가? 이름에 속지 말 것.\n");
    }

    // ── 실험 3 ──────────────────────────────────────────────
    lab::section("실험 3 — 반환할 때는 몇 번 복사되는가");
    std::printf("  먼저 반환하는 식이 어느 값 범주인지 본다.\n\n");
    {
        Payload p(0);
        VALCAT(Payload(1));
        VALCAT(p);
        VALCAT(std::move(p));
    }
    std::printf("\n  이제 각각을 반환해본다.\n\n");
    {
        Reset(); Payload x = MakePrvalue(); (void)x; Report("return Payload(1);       prvalue");
        Reset(); Payload y = MakeNamed();   (void)y; Report("Payload p; return p;     lvalue");
        Reset(); Payload z = MakeMoved();   (void)z; Report("return std::move(p);     xvalue");
        std::printf("\n  -> 값으로 반환하면 무조건 복사가 난다고 알고 있었다면 여기서 갈린다.\n");
        std::printf("  -> 셋 중 하나만 결과가 다르다. 값 범주 표와 대조해볼 것.\n");
    }

    // ── 실험 4 ──────────────────────────────────────────────
    lab::section("실험 4 — auto 가 만드는 복사");
    {
        Reset(); auto        p1 = GetConstRef(); (void)p1; Report("auto        p = GetConstRef()");
        Reset(); const auto& p2 = GetConstRef(); (void)p2; Report("const auto& p = GetConstRef()");

        std::vector<Payload> v;
        v.reserve(3);
        v.emplace_back(1); v.emplace_back(2); v.emplace_back(3);

        Reset(); for (auto        e : v) { (void)e; } Report("for (auto        e : v)");
        Reset(); for (const auto& e : v) { (void)e; } Report("for (const auto& e : v)");
        std::printf("\n  -> auto 는 참조와 const 를 떼어낸다. 그 결과가 이 숫자다.\n");
    }

    // ── 실험 5 ──────────────────────────────────────────────
    lab::section("실험 5 — 람다 캡처");
    {
        Payload a(1);

        Reset(); auto L1 = [a]  { return a.Id; };  Report("[a]  값 캡처");
        Reset(); auto L2 = [&a] { return a.Id; };  Report("[&a] 참조 캡처");

        // TODO 3 ── 아래 캡처를 이동 캡처로 바꿔라.   [b = std::move(b)]
        //           복사가 이동으로 바뀌는 것을 확인한다.
        Payload b(2);
        Reset(); auto L3 = [b = std::move(b)] { return b.Id; };   Report("[b]  (TODO 3 대상)");

        std::putchar('\n');
        LAYOUT(decltype(L1));
        LAYOUT(decltype(L2));
        std::printf("\n  -> 람다의 크기는 무엇으로 정해지는가?\n");
        (void)L1; (void)L2; (void)L3;
    }

    // ── 실험 6 ──────────────────────────────────────────────
    lab::section("실험 6 — vector 가 자리를 늘릴 때");
    std::printf("  reserve(2) 로 꽉 채운 뒤 하나 더 넣는다.\n");
    std::printf("  기존 원소 2개를 옮길까, 복사할까?\n\n");
    {
        std::vector<Payload> v;
        v.reserve(2);
        v.emplace_back(1);
        v.emplace_back(2);

        Reset();
        v.emplace_back(3);              // 여기서 재할당이 일어난다
        Report("재할당 — 기존 원소 2개");

        std::printf("\n  -> TODO 1 을 끝낸 뒤에도 이 숫자가 그대로일 수 있다.\n");
        std::printf("     그때 TODO 2 를 하면 바뀐다. 무엇 때문인지가 이 실험의 전부다.\n");
    }

    // ── 실험 7 ──────────────────────────────────────────────
    lab::section("실험 7 (덤) — 정적 바인딩과 동적 바인딩");
    std::printf("  Child 를 만들어 Base* 로 넘긴다. 두 호출이 어디로 가는가.\n\n");
    {
        Child c;
        CallBoth(&c);
    }
    std::putchar('\n');
    LAYOUT(Base);
    std::printf("\n  -> 한쪽은 컴파일 시점에, 다른 쪽은 실행 시점에 목적지가 정해진다.\n");
    std::printf("  -> 어셈블리로 확인하려면:\n");
    std::printf("       cl /nologo /EHsc /O2 /std:c++20 /FAsc /c main.cpp\n");
    std::printf("     생성된 main.cod 에서 CallBoth 를 찾아 call 두 개를 비교한다.\n");

    // ── 결산 ────────────────────────────────────────────────
    lab::section("결산");
    std::printf("  이번 실행에서 일어난 총 복사 : %d 회\n\n", g_TotalCopy);
    std::printf("  이 중 일부는 의도된 복사다. 고치지 않고 대조군으로 남긴다.\n");
    std::printf("    - 값으로 넘기기 · 값으로 초기화 · 대입      (실험 1)\n");
    std::printf("    - auto 로 받기 · range-for 값 순회          (실험 4)\n");
    std::printf("    - 람다 [a] 값 캡처                          (실험 5)\n\n");
    std::printf("  나머지는 TODO 1 · 2 · 3 을 끝내면 사라진다.\n");
    std::printf("  몇 회가 되어야 맞는지는 RESULT.md 에 먼저 예측해두고 확인한다.\n");
    std::putchar('\n');
    return 0;
}
