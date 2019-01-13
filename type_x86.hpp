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
//===----------------------------------------------------------------------===//
// definitions of memory address, it currently only supports 32 bits(dword ptr)
//===----------------------------------------------------------------------===//
template <typename ImmType = std::uint32_t>
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

template <typename ImmType>
template <typename Type>
inline addr<ImmType>::addr(Type disp_or_reg) {
    if constexpr (sizeof(Type)==32) {
        disp = disp_or_reg;
        modsib = std::nullopt;
    } else {
        disp = std::nullopt;
        modrm = _modrm(0b00, INVALID, disp_or_reg.val);
    }
}

template <typename ImmType>
template <typename RegType>
inline addr<ImmType>::addr(RegType reg, ImmType disp) : disp(disp) {
    modrm = _modrm((sizeof(ImmType)==4 ? 0b10 : 0b01), INVALID,
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
    modrm = _modrm((sizeof(ImmType) == 4 ? 0b10 : 0b01), INVALID, 0b100);
    _modsib(modsib, base.val, scale.val, index);
}

//===----------------------------------------------------------------------===//
// type traits
//===----------------------------------------------------------------------===//

template <typename A>
using is_register =
    std::disjunction<is_same<A, reg8>, is_same<A, reg16>, is_same<A, reg32>>;

template <typename A>
using is_reg8 = std::is_same<A, reg8>;

template <typename A>
using is_reg16or32 = std::disjunction<is_same<A, reg16>, is_same<A, reg32>>;

template <typename A>
using is_immediate = std::is_integral<A>;

template <class>
struct is_address : std::false_type {};

template <class Template>
struct is_address<addr<Template>> : std::true_type {};

template <typename SrcType, typename DestType>
using is_reg_2_reg =
    std::conjunction<is_register<SrcType>, is_register<DestType>>;

template <typename SrcType, typename DestType>
using is_reg_2_mem =
    std::conjunction<is_register<SrcType>, is_address<DestType>>;

template <typename SrcType, typename DestType>
using is_mem_2_reg =
    std::conjunction<is_address<SrcType>, is_register<DestType>>;

template <typename SrcType, typename DestType>
using is_imm_2_reg =
    std::conjunction<is_immediate<SrcType>, is_register<DestType>>;

template <typename SrcType, typename DestType>
using is_imm_2_mem =
    std::conjunction<is_immediate<SrcType>, is_address<DestType>>;

template <typename SrcType, typename DestType>
using is_w = std::disjunction<
    is_reg8<SrcType>, 
    is_reg8<DestType>,
    std::conjunction<
    std::is_integral<SrcType>,
    std::is_same<std::integral_constant<int, sizeof(SrcType)>,std::integral_constant<int, 1>>>
                    ,
    std::conjunction<std::is_integral<DestType>,
                     std::is_same<std::integral_constant<int, sizeof(DestType)>,
                                  std::integral_constant<int, 1>>>>;

template <typename OperandType>
using is_s = std::is_same<std::integral_constant<int, sizeof(OperandType)>,
                          std::integral_constant<int, 1>>;

#endif
