#ifndef _INSTR_H
#define _INSTR_H

#include "morejit.hpp"
#include "morejit_x86.hpp"

namespace instr {
//===----------------------------------------------------------------------===//
// mov
//===----------------------------------------------------------------------===//
template <typename ImmType>
inline void mov(jitcode& c, r dest, ImmType imm) {
    c.emit_opcode(0xb8 + (int)dest);
    c.emit_imm(imm);

    // c.emit_byte('\n');
}

template <typename DispImmType, typename ImmType>
inline void mov(jitcode& c, addr<DispImmType> mem, ImmType imm) {
    c.emit_opcode(0xc7);
    c.emit_addr(mem);
    c.emit_imm(imm);

    // c.emit_byte('\n');
}
inline void mov(jitcode& c, r dest, r src) {
    c.emit_opcode(0x8b);
    c.emit_byte(_modrm(0b11, (int)dest, (int)src));

    // c.emit_byte('\n');
}

template <typename ImmType>
inline void mov(jitcode& c, r dest, addr<ImmType> mem) {
    c.emit_opcode(0x8b);
    c.emit_addr(mem);

    // c.emit_byte('\n');
}
template <typename ImmType>
inline void mov(jitcode& c, addr<ImmType> mem, r src) {
    c.emit_opcode(0x89);
    c.emit_addr(mem, (int)src);

    // c.emit_byte('\n');
}
//===----------------------------------------------------------------------===//
// add
//===----------------------------------------------------------------------===//
template <typename ImmType>
inline void add(jitcode& c, r dest, ImmType imm) {
    c.emit_opcode(is_same<ImmType, imm8>::value ? 0x80 : 0x81);
    c.emit_byte(_modrm(0b11, 0, (int)dest));
    c.emit_imm(imm);

    // c.emit_byte('\n');
}
template <typename ImmType>
inline void add(jitcode& c, r dest, addr<ImmType> mem) {
    c.emit_opcode(is_same<ImmType, imm8>::value ? 0x2 : 0x3);
    c.emit_addr(mem, (int)dest);

    // c.emit_byte('\n');
}
template <typename ImmType>
inline void add(jitcode& c, addr<ImmType> mem, ImmType imm) {
    c.emit_opcode(is_same<ImmType, imm8>::value ? 0x80 : 0x81);
    c.emit_addr(mem);
    c.emit_imm(imm);

    // c.emit_byte('\n');
}

template <typename ImmType>
inline void add(jitcode& c, addr<ImmType> mem, r src) {
    c.emit_opcode(is_same<ImmType, imm8>::value ? 0x0 : 0x1);
    c.emit_addr(mem, (int)src);

    // c.emit_byte('\n');
}

inline void add(jitcode& c, r dest, r src) {
    c.emit_opcode(0x3);
    c.emit_byte(_modrm(0b11, (int)dest, (int)src));
    // c.emit_byte('\n');
}
//===----------------------------------------------------------------------===//
// call
//===----------------------------------------------------------------------===//
template <typename ImmType>
inline void call(jitcode& c, ImmType imm) {
    c.emit_opcode(0xe8);
    c.emit_imm(imm);

    // c.emit_byte('\n');
}

template <typename ImmType>
inline void call(jitcode& c, addr<ImmType> mem) {
    c.emit_opcode(0xff);
    c.emit_addr(mem, 2);

    // c.emit_byte('\n');
}
//===----------------------------------------------------------------------===//
// push
//===----------------------------------------------------------------------===//
template <typename ImmType>
inline void push(jitcode& c, ImmType imm) {
    c.emit_opcode(is_same<ImmType, imm8>::value ? 0x6a : 0x68);
    c.emit_imm(imm);

    // c.emit_byte('\n');
}
template <typename ImmType>
inline void push(jitcode& c, addr<ImmType> mem) {
    c.emit_opcode(0xff);
    c.emit_addr(mem, 6);

    // c.emit_byte('\n');
}
inline void push(jitcode& c, r reg) {
    c.emit_opcode(0x50 + reg);

    // c.emit_byte('\n');
}
//===----------------------------------------------------------------------===//
// pop
//===----------------------------------------------------------------------===//
inline void pop(jitcode& c, r reg) {
    c.emit_opcode(0x58 + reg);

    // c.emit_byte('\n');
}
//===----------------------------------------------------------------------===//
// ret
//===----------------------------------------------------------------------===//
inline void ret(jitcode& c) {
    c.emit_opcode(0xc3);

    // c.emit_byte('\n');
}
};      // namespace instr
#endif  // !_INSTR_H
