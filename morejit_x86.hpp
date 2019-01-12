#ifndef _MOREJIT_X86_H
#define _MOREJIT_X86_H
#include "morejit.hpp"

#define _backfill_regfield(MODRM, REG) MODRM |= ((REG & 7) << 3);

#define _modrm(MOD, REG, RM) \
    (uint8_t)((RM & 0x7) | ((REG & 0x7) << 3) | (MOD << 6))

#define _modsib(modsib, base, scale, index)                      \
    if (scale == reg::esp) {                                     \
        static_assert(true, "scale register should not be ESP"); \
    }                                                            \
    switch (index) {                                             \
        case 0:                                                  \
            modsib = _modrm(0b00, (int)scale, (int)base);        \
            break;                                               \
        case 2:                                                  \
            modsib = _modrm(0b01, (int)scale, (int)base);        \
            break;                                               \
        case 4:                                                  \
            modsib = _modrm(0b10, (int)scale, (int)base);        \
            break;                                               \
        case 8:                                                  \
            modsib = _modrm(0b11, (int)scale, (int)base);        \
            break;                                               \
    }

using std::is_pointer;
using std::is_same;

template <typename FuncPtrType>
inline FuncPtrType jitcode::as_function() {
    static_assert(is_pointer<FuncPtrType>::value,
                  "expects function pointer type");
    return (FuncPtrType)(alloc_start + text_size + 1);
}

template <typename ImmType>
inline void jitcode::emit_imm(ImmType imm) {
    *(decltype(imm.val)*)cur_code = imm.val;
    cur_code += imm.get_bytes();
}

template <typename ImmType>
inline void jitcode::emit_addr(addr<ImmType>& mem, int backfill_reg) {
    _backfill_regfield(mem.modrm, backfill_reg);
    *cur_code++ = mem.modrm;
    if (mem.modsib.has_value()) {
        *cur_code++ = mem.modsib.value();
    }
    if (mem.disp.has_value()) {
        emit_imm(mem.disp.value());
    }
}
template <typename ImmType, int Size>
addr<ImmType, Size>::addr(ImmType disp32) : modsib(std::nullopt), disp(disp32) {
    static_assert(is_same<ImmType, imm32>::value,
                  "expects 32 bits immediate number");
    modrm = _modrm(0b00, INVALID, 0b101);
}

template <typename ImmType, int Size>
addr<ImmType, Size>::addr(reg reg) {
    modrm = _modrm(0b00, INVALID, (int)reg);
}

template <typename ImmType, int Size>
addr<ImmType, Size>::addr(reg reg, ImmType disp)
    : modsib(std::nullopt), disp(disp) {
    modrm = _modrm((is_same<ImmType, imm32>::value ? 0b10 : 0b01), INVALID,
                   (int)reg);
}

template <typename ImmType, int Size>
addr<ImmType, Size>::addr(reg base, reg scale, uint8_t index) {
    modrm = _modrm(0b00, INVALID, 0b100);
    _modsib(modsib, base, scale, index);
}

template <typename ImmType, int Size>
addr<ImmType, Size>::addr(reg base, reg scale, uint8_t index, ImmType disp)
    : disp(disp) {
    modrm =
        _modrm((is_same<ImmType, imm32>::value ? 0b10 : 0b01), INVALID, 0b100);
    _modsib(modsib, base, scale, index);
}
#endif
