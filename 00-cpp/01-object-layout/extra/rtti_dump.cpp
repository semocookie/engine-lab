// RTTI 안에는 무엇이 들어 있나
//
//   vptr 바로 앞칸(vt[-1])의 주소를 따라가 MSVC 의 RTTI 구조체를 직접 읽는다.
//
//   ⚠ MSVC x64 전용. 구조체 모양은 MSVC 구현이지 표준이 아니다.
//     x64 는 포인터 대신 '실행 파일 시작점 기준 오프셋(RVA)' 을 쓴다.
//   ⚠ Debug(/Od) 로 돌릴 것.

#include <cstdio>
#include <cstdint>
#include <typeinfo>
#include "dump.h"

struct Actor      { virtual ~Actor() {} virtual void WhoAmI() const {} };
struct Pawn       : Actor { void WhoAmI() const override {} };
struct PlayerPawn : Pawn  { void WhoAmI() const override {} };

// 다중 상속 — 부모가 객체 안 '어디' 있는지가 필요해지는 경우
struct Widget  { virtual ~Widget() {} int W = 0; };
struct HudPawn : Pawn, Widget {};

// ── MSVC x64 RTTI 구조체 ─────────────────────────────────────
struct CompleteObjectLocator {        // vt[-1] 이 가리키는 곳
    uint32_t Signature;               // x64 는 1
    uint32_t Offset;                  // 이 vptr 이 완전한 객체 안 몇 바이트째인가
    uint32_t CdOffset;
    int32_t  TypeDescriptor;          // → 이름
    int32_t  ClassDescriptor;         // → 상속 계보
    int32_t  Self;                    // 자기 자신 (시작점 역산용)
};
struct TypeDescriptor {
    const void* TypeInfoVTable;
    void*       Spare;
    char        Name[1];              // 꾸며진 이름. 예) .?AUPawn@@
};
struct ClassHierarchyDescriptor {
    uint32_t Signature;
    uint32_t Attributes;              // 1 = 다중 상속, 2 = 가상 상속
    uint32_t NumBaseClasses;          // 자기 자신 포함
    int32_t  BaseClassArray;          // → BaseClassDescriptor RVA 배열
};
struct PMD { int32_t MDisp, PDisp, VDisp; };
struct BaseClassDescriptor {
    int32_t  TypeDescriptor;
    uint32_t NumContainedBases;
    PMD      Where;                   // MDisp = 이 부모가 객체 안 몇 바이트째에 있나
    uint32_t Attributes;
};

template <class T>
static const T* At(uintptr_t Base, int32_t Rva) {
    return reinterpret_cast<const T*>(Base + Rva);
}

static void DumpRtti(const char* Label, const void* Obj) {
    void** vt = *reinterpret_cast<void** const*>(Obj);
    auto* col = static_cast<const CompleteObjectLocator*>(vt[-1]);
    uintptr_t base = reinterpret_cast<uintptr_t>(col) - col->Self;

    auto* td  = At<TypeDescriptor>(base, col->TypeDescriptor);
    auto* chd = At<ClassHierarchyDescriptor>(base, col->ClassDescriptor);
    auto* arr = At<int32_t>(base, chd->BaseClassArray);

    std::printf("\n  [%s]\n", Label);
    std::printf("    이름      : %s\n", td->Name);
    std::printf("    상속 계보 : %u개 (자기 포함)%s\n", chd->NumBaseClasses,
                (chd->Attributes & 1) ? "  · 다중 상속" : "");
    for (uint32_t i = 0; i < chd->NumBaseClasses; ++i) {
        auto* bcd = At<BaseClassDescriptor>(base, arr[i]);
        auto* btd = At<TypeDescriptor>(base, bcd->TypeDescriptor);
        std::printf("      %u. %-18s 객체 안 +%d 바이트\n", i, btd->Name, bcd->Where.MDisp);
    }
}

int main() {
    lab::section("1. RTTI 안을 직접 읽는다  (vt[-1] 을 따라가서)");

    Actor a; Pawn p; PlayerPawn pp; HudPawn hp;
    DumpRtti("Actor",      &a);
    DumpRtti("Pawn",       &p);
    DumpRtti("PlayerPawn", &pp);
    DumpRtti("HudPawn",    &hp);

    lab::section("2. typeid 는 이 문자열을 읽는가");
    std::printf("  typeid(pp).raw_name() : %s\n", typeid(pp).raw_name());
    std::printf("  typeid(pp).name()     : %s\n", typeid(pp).name());

    lab::section("3. dynamic_cast 가 이 정보를 쓰는 모습");
    Actor* ap = &pp;
    Actor* aa = &a;
    Actor* ah = &hp;
    std::printf("  Actor* → Pawn*   (PlayerPawn 객체) : %s\n", dynamic_cast<Pawn*>(ap) ? "성공" : "실패");
    std::printf("  Actor* → Pawn*   (Actor 객체)      : %s\n", dynamic_cast<Pawn*>(aa) ? "성공" : "실패");
    Widget* w = dynamic_cast<Widget*>(ah);
    std::printf("  Actor* → Widget* (HudPawn 객체)    : %s, 주소가 %+td 바이트 움직임\n",
                w ? "성공" : "실패",
                reinterpret_cast<char*>(w) - reinterpret_cast<char*>(ah));
    std::putchar('\n');
    return 0;
}
