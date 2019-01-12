#ifndef _INSTR_H
#define _INSTR_H

#include "morejit.hpp"
#include "morejit_x86.hpp"

namespace instr {
template <typename T>
using is_register = std::is_enum<T>;
//===----------------------------------------------------------------------===//
// mov
//===----------------------------------------------------------------------===//
template <typename DestType, typename ImmType>
inline void mov(jitcode& c, DestType dest, ImmType src) {
    // If destination operand is register
    if constexpr (is_register<DestType>::value) {
        c.emit_opcode(0xb8 + (int)dest);
        c.emit_imm(src);
    }
    // Otherwise, it represents a memory address
    else {
        c.emit_opcode(0xc7);
        c.emit_addr(dest);
        c.emit_imm(src);
    }
}

template <typename DestType>
inline void mov(jitcode& c, DestType dest, reg src) {
    if constexpr (is_register<DestType>::value) {
        c.emit_opcode(0x8b);
        c.emit_byte(_modrm(0b11, (int)dest, (int)src));
    } else {
        c.emit_opcode(0x89);
        c.emit_addr(dest, (int)src);
    }
}
template <typename DestType, typename ImmType>
inline void mov(jitcode& c, DestType dest, addr<ImmType> src) {
    if constexpr (is_register<DestType>::value) {
        c.emit_opcode(0x8b);
        c.emit_addr(src);
    }
}
//===----------------------------------------------------------------------===//
// add
//===----------------------------------------------------------------------===//
template <typename DestType, typename ImmType>
inline void add(jitcode& c, DestType dest, ImmType src) {
    if constexpr (is_register<DestType>::value) {
        c.emit_opcode(is_same<ImmType, imm8>::value ? 0x80 : 0x81);
        c.emit_byte(_modrm(0b11, 0, (int)dest));
        c.emit_imm(src);
    } else {
        c.emit_opcode(is_same<ImmType, imm8>::value ? 0x80 : 0x81);
        c.emit_addr(dest);
        c.emit_imm(src);
    }
}

template <typename DestType>
inline void add(jitcode& c, DestType dest, reg src) {
    if constexpr (is_register<DestType>::value) {
        c.emit_opcode(0x3);
        c.emit_byte(_modrm(0b11, (int)dest, (int)src));
    } else {
        c.emit_opcode(is_same<DestType, addr<imm8>>::value ? 0x0 : 0x1);
        c.emit_addr(dest, (int)src);
    }
}

template <typename ImmType>
inline void add(jitcode& c, reg dest, addr<ImmType> src) {
    c.emit_opcode(is_same<ImmType, imm8>::value ? 0x2 : 0x3);
    c.emit_addr(src, (int)dest);
}
//===----------------------------------------------------------------------===//
// sub
//===----------------------------------------------------------------------===//
template <typename DestType, typename ImmType>
inline void sub(jitcode& c, DestType dest, ImmType src) {
    if constexpr (is_register<DestType>::value) {
        c.emit_opcode(is_same<ImmType, imm8>::value ? 0x80 : 0x81);
        c.emit_byte(_modrm(0b11, 5, (int)dest));
        c.emit_imm(src);
    } else {
        c.emit_opcode(is_same<ImmType, imm8>::value ? 0x80 : 0x81);
        c.emit_addr(dest, 5);
        c.emit_imm(src);
    }
}

template <typename DestType>
inline void sub(jitcode& c, DestType dest, reg src) {
    if constexpr (is_register<DestType>::value) {
        c.emit_opcode(0x2b);
        c.emit_byte(_modrm(0b11, (int)dest, (int)src));
    } else {
        c.emit_opcode(is_same<DestType, addr<imm8>>::value ? 0x28 : 0x29);
        c.emit_addr(dest, (int)src);
    }
}

template <typename ImmType>
inline void sub(jitcode& c, reg dest, addr<ImmType> src) {
    c.emit_opcode(is_same<ImmType, imm8>::value ? 0x2a : 0xab);
    c.emit_addr(src, (int)dest);
}

//===----------------------------------------------------------------------===//
// call
//===----------------------------------------------------------------------===//
template <typename ImmType>
inline void call(jitcode& c, ImmType operand) {
    c.emit_opcode(0xe8);
    c.emit_imm(operand);
}

template <typename ImmType>
inline void call(jitcode& c, addr<ImmType> operand) {
    c.emit_opcode(0xff);
    c.emit_addr(operand, 2);
}
//===----------------------------------------------------------------------===//
// push
//===----------------------------------------------------------------------===//
template <typename ImmType>
inline void push(jitcode& c, ImmType operand) {
    c.emit_opcode(is_same<ImmType, imm8>::value ? 0x6a : 0x68);
    c.emit_imm(operand);
}
template <typename ImmType>
inline void push(jitcode& c, addr<ImmType> operand) {
    c.emit_opcode(0xff);
    c.emit_addr(operand, 6);
}
inline void push(jitcode& c, reg operand) { c.emit_opcode(0x50 + operand); }
//===----------------------------------------------------------------------===//
// pop
//===----------------------------------------------------------------------===//
inline void pop(jitcode& c, reg operand) { c.emit_opcode(0x58 + operand); }
//===----------------------------------------------------------------------===//
// ret
//===----------------------------------------------------------------------===//
inline void ret(jitcode& c) { c.emit_opcode(0xc3); }
};      // namespace instr
#endif  // !_INSTR_H
