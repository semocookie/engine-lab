// tools/bench.h — 측정용 헬퍼
//
// 규칙: "빠르다/느리다"로 끝내지 말고 항상 숫자를 남긴다.
//       그 숫자가 그대로 RESULT.md 로 간다.
#pragma once

#include <chrono>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <string>

namespace lab {

class Timer {
public:
    Timer() : start_(clock::now()) {}
    void reset() { start_ = clock::now(); }

    double ms() const {
        return std::chrono::duration<double, std::milli>(clock::now() - start_).count();
    }
    double ns() const {
        return std::chrono::duration<double, std::nano>(clock::now() - start_).count();
    }

private:
    using clock = std::chrono::steady_clock;
    clock::time_point start_;
};

// 같은 작업을 여러 번 재고 중앙값을 쓴다.
// 평균은 튀는 한 번에 오염되지만 중앙값은 버틴다.
template <typename Fn>
double median_ms(Fn&& fn, int runs = 7) {
    std::vector<double> samples;
    samples.reserve(static_cast<std::size_t>(runs));
    for (int i = 0; i < runs; ++i) {
        Timer t;
        fn();
        samples.push_back(t.ms());
    }
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

// 최적화로 코드가 통째로 사라지는 것을 막는다.
// (측정하려던 루프가 통으로 제거돼서 0ms 가 나오는 사고는 흔하다)
template <typename T>
inline void keep(T const& value) {
    volatile auto sink = value;
    (void)sink;
}

inline void report(const std::string& label, double ms, const std::string& note = "") {
    std::printf("  %-32s %10.3f ms   %s\n", label.c_str(), ms, note.c_str());
}

// 기준값 대비 몇 배인지
inline void report_ratio(const std::string& label, double ms, double baseline_ms) {
    std::printf("  %-32s %10.3f ms   (기준 대비 %.2fx)\n",
                label.c_str(), ms, ms / baseline_ms);
}

} // namespace lab
