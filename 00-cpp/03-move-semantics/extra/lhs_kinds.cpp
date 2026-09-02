#include <string>
#include <utility>

struct Payload {
    std::string Data = std::string(64, 'x');
    int Id = 0;
    Payload() = default;
    explicit Payload(int i) : Id(i) {}
    Payload(const Payload&) {}
    Payload(Payload&&) noexcept {}
};

int main() {
    Payload a(1);

    Payload        b = std::move(a);   // (1)
    Payload&&      r = std::move(a);   // (2)
    const Payload& d = std::move(a);   // (3)
    // Payload&    c = std::move(a);   // (4) ← 이 줄의 주석을 풀면 컴파일이 실패한다
    //                                 //     error C2440 : 비const 참조는 lvalue에만 바인딩할 수 있습니다
    Payload&       e = a;              // (5)

    (void)b; (void)r; (void)d; (void)e;
    return 0;
}
