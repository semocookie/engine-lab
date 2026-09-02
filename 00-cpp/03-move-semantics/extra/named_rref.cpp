#include <cstdio>
#include <string>
#include <type_traits>
#include <utility>

static int g_Copy = 0, g_Move = 0;
static void Reset() { g_Copy = 0; g_Move = 0; }
static void Report(const char* s) { std::printf("  %-34s 복사 %d   이동 %d\n", s, g_Copy, g_Move); }

struct Payload {
    std::string Data = std::string(64, 'x');
    int Id = 0;
    Payload() = default;
    explicit Payload(int i) : Id(i) {}
    Payload(const Payload& o) : Data(o.Data), Id(o.Id)            { ++g_Copy; }
    Payload(Payload&& o) noexcept : Data(std::move(o.Data)), Id(o.Id) { ++g_Move; }
};

template <class T> static constexpr const char* Cat() {
    if constexpr (std::is_lvalue_reference_v<T>)      return "lvalue";
    else if constexpr (std::is_rvalue_reference_v<T>) return "xvalue";
    else                                              return "prvalue";
}
#define VALCAT(e) std::printf("  %-24s %s\n", #e, Cat<decltype((e))>())

// 우측값 참조로 받은 매개변수를, 그대로 / std::move 로 넘겼을 때
static void TakeAndBind (Payload&& p) { Reset(); Payload b = p;            (void)b; Report("f(Payload&& p) { b = p; }"); }
static void TakeAndMove (Payload&& p) { Reset(); Payload b = std::move(p); (void)b; Report("f(Payload&& p) { b = move(p); }"); }

int main() {
    // [1] 바인딩 자체가 무언가를 옮기는가
    std::printf("\n[1] Payload&& r = std::move(a);  — 이 줄만 실행한 직후\n\n");
    {
        Payload a(1);
        Reset();
        Payload&& r = std::move(a);
        Report("Payload&& r = std::move(a)");
        std::printf("  %-34s a.Data 길이 %zu   r.Data 길이 %zu\n", "", a.Data.size(), r.Data.size());
        std::printf("  %-34s &a = %p\n", "", (void*)&a);
        std::printf("  %-34s &r = %p\n", "", (void*)&r);
    }

    // [2] 이름이 붙은 우측값 참조의 값 범주
    std::printf("\n[2] 이름이 붙은 뒤의 값 범주\n\n");
    {
        Payload a(1);
        Payload&& r = std::move(a);
        VALCAT(a);
        VALCAT(std::move(a));
        VALCAT(r);              // 이름이 붙은 우측값 참조
    }

    // [3] 그래서 그 이름을 다시 넘기면
    std::printf("\n[3] 그 이름을 다른 객체 초기화에 쓰면\n\n");
    {
        Payload a(1);
        Payload&& r = std::move(a);
        Reset(); Payload b = r;            (void)b; Report("Payload b = r;");
    }
    {
        Payload a(1);
        Payload&& r = std::move(a);
        Reset(); Payload b = std::move(r); (void)b; Report("Payload b = std::move(r);");
    }

    // [4] 함수 매개변수도 같은가
    std::printf("\n[4] 매개변수로 받은 경우\n\n");
    TakeAndBind(Payload(1));
    TakeAndMove(Payload(1));
    std::putchar('\n');
    return 0;
}
