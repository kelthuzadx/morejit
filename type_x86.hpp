#ifndef _MOREJIT_X86_H
#define _MOREJIT_X86_H

#include <any>
#include "morejit.hpp"

using std::optional;
using std::uint32_t;
using std::uint8_t;

using std::is_pointer;
using std::is_same;

//===----------------------------------------------------------------------===//
// internal macros
//===----------------------------------------------------------------------===//
#define _backfill_regfield(MODRM, REG) MODRM |= ((REG & 7) << 3);

#define _modrm(MOD, REG, RM) \
    (uint8_t)((RM & 0x7) | ((REG & 0x7) << 3) | (MOD << 6))

#define _modsib(modsib, base, scale, index)                      \
    if (scale == reg::esp) {                                     \
        static_assert(true, "scale register should not be ESP"); \
    }                                                            \
    switch (index) {                                             \
        case 0:                                                  \
            modsib = _modrm(0b00, scale.val, base.val);          \
            break;                                               \
        case 2:                                                  \
            modsib = _modrm(0b01, scale.val, base.val);          \
            break;                                               \
        case 4:                                                  \
            modsib = _modrm(0b10, scale.val, base.val);          \
            break;                                               \
        case 8:                                                  \
            modsib = _modrm(0b11, scale.val, base.val);          \
            break;                                               \
    }
//===----------------------------------------------------------------------===//
// definitions of 8/16/32bits general-purpose registers
//===----------------------------------------------------------------------===//
#define eax \
    reg32 { 0 }
#define ecx \
    reg32 { 1 }
#define edx \
    reg32 { 2 }
#define ebx \
    reg32 { 3 }
#define esp \
    reg32 { 4 }
#define ebp \
    reg32 { 5 }
#define esi \
    reg32 { 6 }
#define edi \
    reg32 { 7 }

#define ax \
    reg16 { 0 }
#define cx \
    reg16 { 1 }
#define dx \
    reg16 { 2 }
#define bx \
    reg16 { 3 }
#define sp \
    reg16 { 4 }
#define bp \
    reg16 { 5 }
#define si \
    reg16 { 6 }
#define di \
    reg16 { 7 }

#define al \
    reg8 { 0 }
#define cl \
    reg8 { 1 }
#define dl \
    reg8 { 2 }
#define bl \
    reg8 { 3 }
#define ah \
    reg8 { 4 }
#define ch \
    reg8 { 5 }
#define dh \
    reg8 { 6 }
#define bh \
    reg8 { 7 }

struct reg8 {
    explicit reg8(uint8_t val) : val(val) {}
    const uint8_t val;
};
struct reg16 {
    explicit reg16(uint8_t val) : val(val) {}
    const uint8_t val;
};
struct reg32 {
    explicit reg32(uint8_t val) : val(val) {}
    const uint8_t val;
};

template <typename A>
using is_register =
    std::disjunction<is_same<A, reg8>, is_same<A, reg16>, is_same<A, reg32>>;
template <typename A>
using is_reg8 = std::is_same<A, reg8>;

template <typename A>
using is_reg16or32 = std::disjunction<is_same<A, reg16>, is_same<A, reg32>>;
//===----------------------------------------------------------------------===//
// definitions of 8/16/32bits immediate numbers
//===----------------------------------------------------------------------===//
struct imm32 {
    explicit imm32(uint32_t val) : val(val) {}
    static constexpr int get_bytes() { return 4; }
    uint32_t val;
};

struct imm16 {
    explicit imm16(uint16_t val) : val(val) {}
    static constexpr int get_bytes() { return 2; }
    uint16_t val;
};

struct imm8 {
    explicit imm8(uint8_t val) : val(val) {}
    static constexpr int get_bytes() { return 1; }
    uint8_t val;
};

inline imm32 operator"" _i32(unsigned long long val) { return imm32(val); }

inline imm16 operator"" _i16(unsigned long long val) { return imm16(val); }

inline imm8 operator"" _i8(unsigned long long val) { return imm8(val); }

template <typename A>
using is_immediate =
    std::disjunction<is_same<A, imm8>, is_same<A, imm16>, is_same<A, imm32>>;

//===----------------------------------------------------------------------===//
// definitions of memory address, it currently only supports 32 bits(dword ptr)
//===----------------------------------------------------------------------===//
template <typename ImmType = imm32>
struct addr {
    template <typename Type>
    explicit addr(Type disp_or_reg);

    template <typename RegType>
    explicit addr(RegType reg, ImmType disp);

    template <typename RegType>
    explicit addr(RegType base, RegType scale, uint8_t index);

    template <typename RegType>
    explicit addr(RegType base, RegType scale, uint8_t index, ImmType disp);

    uint8_t modrm;
    optional<uint8_t> modsib;
    optional<ImmType> disp;

    static const int INVALID = 0;
};
template <class>
struct is_address : std::false_type {};
template <class Template>
struct is_address<addr<Template>> : std::true_type {};

template <typename ImmType>
template <typename Type>
inline addr<ImmType>::addr(Type disp_or_reg) {
    if constexpr (is_immediate<Type>::value) {
        disp = disp_or_reg;
        modsib = std::nullopt;
    } else if constexpr (is_register<Type>::value) {
        disp = std::nullopt;
        modrm = _modrm(0b00, INVALID, (int)disp_or_reg);
    }
}

template <typename ImmType>
template <typename RegType>
inline addr<ImmType>::addr(RegType reg, ImmType disp) : disp(disp) {
    modrm = _modrm((is_same<ImmType, imm32>::value ? 0b10 : 0b01), INVALID,
                   reg.val);
}

template <typename ImmType>
template <typename RegType>
inline addr<ImmType>::addr(RegType base, RegType scale, uint8_t index)
    : disp(std::nullopt) {
    modrm = _modrm(0b00, INVALID, 0b100);
    _modsib(modsib, base.val, scale.val, index);
}

template <typename ImmType>
template <typename RegType>
inline addr<ImmType>::addr(RegType base, RegType scale, uint8_t index,
                           ImmType disp)
    : disp(disp) {
    modrm =
        _modrm((is_same<ImmType, imm32>::value ? 0b10 : 0b01), INVALID, 0b100);
    _modsib(modsib, base.val, scale.val, index);
}

#endif
