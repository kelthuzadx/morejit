#ifndef _TYPE_X86_H
#define _TYPE_X86_H

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

inline reg32 eax{0}, ecx{1}, edx{2}, ebx{3}, esp{4}, ebp{5}, esi{6}, edi{7};
inline reg16 ax{0}, cx{1}, dx{2}, bx{3}, sp{4}, bp{5}, si{6}, di{7};
inline reg8 al{0}, cl{1}, dl{2}, bl{3}, ah{4}, ch{5}, dh{6}, bh{7};
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
template <typename SrcType>
using is_w1 = std::disjunction<
    is_reg8<SrcType>,
    std::conjunction<std::is_integral<SrcType>,
                     std::is_same<std::integral_constant<int, sizeof(SrcType)>,
                                  std::integral_constant<int, 1>>>>;

template <typename OperandType>
using is_s = std::is_same<std::integral_constant<int, sizeof(OperandType)>,
                          std::integral_constant<int, 1>>;

#endif
