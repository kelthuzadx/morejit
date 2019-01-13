#ifndef _INSTR_X86_H
#define _INSTR_X86_H

#include "morejit_x86.hpp"

#define BIN(a, b) (0b##a##b)

//===----------------------------------------------------------------------===//
// ADD �C Add
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void x86jitcode::add(DestType dest, SrcType src) {
    if constexpr (is_reg_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0000,0000):BIN(0000,0001));
        emit_u8(_modrm(0b11, src.val, dest.val));
    } else if constexpr (is_mem_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0000,0010):BIN(0000,0011));
        emit_addr(src, dest.val);
    } else if constexpr (is_reg_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0000,0000) : BIN(0000,0001));
        emit_addr(dest, src.val);
    } else if constexpr (is_imm_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000,0000) : BIN(1000,0001));
        emit_u8(_modrm(0b11, 000, dest.val));
        emit_imm(src);
    } else if constexpr (is_imm_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000,0000) : BIN(1000,0001));
        emit_addr(dest, 0);
        emit_imm(src);
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// AND �C Logical AND
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void x86jitcode::and_(DestType dest, SrcType src) {
    if constexpr (is_reg_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0010, 0000)
                                               : BIN(0010, 0001));
        emit_u8(_modrm(0b11, src.val, dest.val));
    } else if constexpr (is_mem_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0010, 0010)
                                               : BIN(0010, 0011));
        emit_addr(src, dest.val);
    } else if constexpr (is_reg_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0010, 0000)
                                               : BIN(0010, 0001));
        emit_addr(dest, src.val);
    } else if constexpr (is_imm_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0000)
                                               : BIN(1000, 0001));
        emit_u8(_modrm(0b11, 0b100, dest.val));
        emit_imm(src);
    } else if constexpr (is_imm_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0000)
                                               : BIN(1000, 0001));
        emit_addr(dest, 0b100);
        emit_imm(src);
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// CALL �C Call Procedure (in same segment)
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::call(OperandType operand) {
    if constexpr (is_immediate<OperandType>::value) {
        emit_u8(BIN(1110,1000));
        emit_imm(operand);
    } else if constexpr (is_address<OperandType>::value) {
        emit_u8(BIN(1111,1111));
        emit_addr(operand, 0b010);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(BIN(1111,1111));
        emit_u8(_modrm(0b11, 0b010, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// CMP �C Compare Two Operands
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void x86jitcode::cmp(DestType dest, SrcType src) {
    if constexpr (is_reg_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0011, 1000)
                                               : BIN(0011, 1001));
        emit_u8(_modrm(0b11, src.val, dest.val));
    } else if constexpr (is_mem_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0011, 1000)
                                               : BIN(0011, 1001));
        emit_addr(src, dest.val);
    } else if constexpr (is_reg_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0011, 1010)
                                               : BIN(0011, 1011));
        emit_addr(dest, src.val);
    } else if constexpr (is_imm_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0000)
                                               : BIN(1000, 0001));
        emit_u8(_modrm(0b11, 0b111, dest.val));
        emit_imm(src);
    } else if constexpr (is_imm_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0000)
                                               : BIN(1000, 0001));
        emit_addr(dest, 0b111);
        emit_imm(src);
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// DEC �C Decrement by 1
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::dec(OperandType operand) {
    if constexpr (is_address<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 1110) : BIN(1111, 1111));
        emit_addr(operand, 0b001);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 1110) : BIN(1111, 1111));
        emit_u8(_modrm(0b11, 0b001, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// DIV �C Unsigned Divide
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::div(OperandType operand) {
    if constexpr (is_address<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_addr(operand, 0b110);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_u8(_modrm(0b11, 0b110, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// HLT �C Hal
//===----------------------------------------------------------------------===//
inline void x86jitcode::hlt() { emit_u8(BIN(1111,0100)); }

//===----------------------------------------------------------------------===//
// MOV �C Move Data
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void x86jitcode::mov(DestType dest, SrcType src) {
    if constexpr (is_reg_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000,1000) : BIN(1000,1001));
        emit_u8(_modrm(0b11, src.val, dest.val));
    } else if constexpr (is_mem_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000,1010) : BIN(1000,1011));
        emit_addr(src, dest.val);
    } else if constexpr (is_reg_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000,1000) : BIN(1000,1001));
        emit_addr(dest, src.val);
    } else if constexpr (is_imm_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1100,0110) : BIN(1100,0111));
        emit_u8(_modrm(0b11, 000, dest.val));
        emit_imm(src);
    } else if constexpr (is_imm_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1100,0110) : BIN(1100,0111));
        emit_addr(dest);
        emit_imm(src);
    } else {
        static_assert(true, "unexpects types");
    }
}

//===----------------------------------------------------------------------===//
// POP �C Pop a Word from the Stack
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::pop(OperandType operand) {
    if constexpr (is_address<OperandType>::value) {
        emit_u8(BIN(1000,1111));
        emit_addr(operand, 0b000);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(BIN(1000,1111));
        emit_u8(_modrm(0b11, 0b000, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}

//===----------------------------------------------------------------------===//
// PUSH �C Push Operand onto the Stack
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::push(OperandType operand) {
    if constexpr (is_immediate<OperandType>::value) {
        emit_u8(is_s<OperandType>::value ? BIN(0110,1010) : BIN(0110,1000));
        emit_imm(operand);
    } else if constexpr (is_address<OperandType>::value) {
        emit_u8(BIN(1111,1111));
        emit_addr(operand, 0b110);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(BIN(1111,1111));
        emit_u8(_modrm(0b11, 0b110, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}

//===----------------------------------------------------------------------===//
// RET
//===----------------------------------------------------------------------===//
inline void x86jitcode::ret() { emit_u8(0xc3); }

#endif  // !_INSTR_X86_H
