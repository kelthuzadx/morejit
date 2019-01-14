#ifndef _INSTR_X86_H
#define _INSTR_X86_H

#include "morejit_x86.hpp"

#define BIN(a, b) (0b##a##b)

//===----------------------------------------------------------------------===//
// ADD - Add
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void x86jitcode::add(DestType dest, SrcType src) {
    if constexpr (is_reg_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0000, 0000) : BIN(0000, 0001));
        emit_u8(_modrm(0b11, src.val, dest.val));
    } else if constexpr (is_mem_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0000, 0010) : BIN(0000, 0011));
        emit_addr(src, dest.val);
    } else if constexpr (is_reg_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0000, 0000) : BIN(0000, 0001));
        emit_addr(dest, src.val);
    } else if constexpr (is_imm_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0000) : BIN(1000, 0001));
        emit_u8(_modrm(0b11, 000, dest.val));
        emit_imm(src);
    } else if constexpr (is_imm_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0000) : BIN(1000, 0001));
        emit_addr(dest, 0);
        emit_imm(src);
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// AND - Logical AND
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void x86jitcode::and(DestType dest, SrcType src) {
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
// CALL - Call Procedure (in same segment)
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::call(OperandType operand) {
    if constexpr (is_immediate<OperandType>::value) {
        emit_u8(BIN(1110, 1000));
        emit_imm(operand);
    } else if constexpr (is_address<OperandType>::value) {
        emit_u8(BIN(1111, 1111));
        emit_addr(operand, 0b010);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(BIN(1111, 1111));
        emit_u8(_modrm(0b11, 0b010, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// CMP - Compare Two Operands
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
// DEC - Decrement by 1
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
// DIV - Unsigned Divide
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
// HLT - Hal
//===----------------------------------------------------------------------===//
inline void x86jitcode::hlt() { emit_u8(BIN(1111, 0100)); }

//===----------------------------------------------------------------------===//
// IDIV 每 Signed Divide
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::idiv(OperandType operand) {
    if constexpr (is_address<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_addr(operand, 0b111);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_u8(_modrm(0b11, 0b111, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// IMUL 每 Signed Multiply
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::imul(OperandType operand) {
    if constexpr (is_address<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_addr(operand, 0b101);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_u8(_modrm(0b11, 0b101, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}
template <typename DestType, typename SrcType>
inline void x86jitcode::imul(DestType dest, SrcType src) {
    if constexpr (is_reg_2_reg<SrcType, DestType>::value) {
        emit_u8(BIN(0000, 1111));
        emit_u8(BIN(1010, 1111));
        emit_u8(_modrm(0b11, dest.val, src.val));
    } else if constexpr (is_mem_2_reg<SrcType, DestType>::value) {
        emit_u8(BIN(0000, 1111));
        emit_u8(BIN(1010, 1111));
        emit_addr(src,dest.val) ;
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// INC 每 Increment by 1
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::inc(OperandType operand) {
    if constexpr (is_address<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 1110) : BIN(1111, 1111));
        emit_addr(operand, 0b000);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 1110) : BIN(1111, 1111));
        emit_u8(_modrm(0b11, 0b000, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// JMP 每 Unconditional Jump (to same segment)
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::jmp(OperandType operand) {
    if constexpr (is_address<OperandType>::value) {
        emit_u8(BIN(1111, 1111));
        emit_addr(operand, 0b100);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(BIN(1111, 1111));
        emit_u8(_modrm(0b11, 0b100, operand.val));
    } else if constexpr (is_immediate<OperandType>::value) {
        if constexpr (sizeof(OperandType) == 1) {
            emit_u8(BIN(1110, 1011));
        } else if constexpr (sizeof(OperandType) == 8) {
            emit_u8(BIN(1110, 1001));
        }
        emit_imm(operand);
    }
    else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// MOV - Move Data
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void x86jitcode::mov(DestType dest, SrcType src) {
    if constexpr (is_reg_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 1000) : BIN(1000, 1001));
        emit_u8(_modrm(0b11, src.val, dest.val));
    } else if constexpr (is_mem_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 1010) : BIN(1000, 1011));
        emit_addr(src, dest.val);
    } else if constexpr (is_reg_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 1000) : BIN(1000, 1001));
        emit_addr(dest, src.val);
    } else if constexpr (is_imm_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1100, 0110) : BIN(1100, 0111));
        emit_u8(_modrm(0b11, 000, dest.val));
        emit_imm(src);
    } else if constexpr (is_imm_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1100, 0110) : BIN(1100, 0111));
        emit_addr(dest);
        emit_imm(src);
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// MUL 每 Unsigned Multiply
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::mul(OperandType operand) {
    if constexpr (is_address<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_addr(operand, 0b100);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_u8(_modrm(0b11, 0b100, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// NEG 每 Two's Complement Negation
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::neg(OperandType operand) {
    if constexpr (is_address<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_addr(operand, 0b011);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_u8(_modrm(0b11, 0b011, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}

//===----------------------------------------------------------------------===//
// NOP 每 No Operatio
//===----------------------------------------------------------------------===//
inline void x86jitcode::nop() { emit_u8(BIN(1001,0000)); }
//===----------------------------------------------------------------------===//
// NOT 每 One's Complement Negation
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::not(OperandType operand) {
    if constexpr (is_address<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_addr(operand, 0b010);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(is_w1<OperandType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_u8(_modrm(0b11, 0b010, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// OR 每 Logical Inclusive OR
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void x86jitcode::or(DestType dest, SrcType src) {
    if constexpr (is_reg_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0000, 1000)
            : BIN(0000, 1001));
        emit_u8(_modrm(0b11, src.val, dest.val));
    } else if constexpr (is_mem_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0000, 1010)
            : BIN(0000, 1011));
        emit_addr(src, dest.val);
    } else if constexpr (is_reg_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0000, 1000)
            : BIN(0000, 1001));
        emit_addr(dest, src.val);
    } else if constexpr (is_imm_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0000)
            : BIN(1000, 0001));
        emit_u8(_modrm(0b11, 0b001, dest.val));
        emit_imm(src);
    } else if constexpr (is_imm_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0000)
            : BIN(1000, 0001));
        emit_addr(dest, 0b001);
        emit_imm(src);
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// POP - Pop a Word from the Stack
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::pop(OperandType operand) {
    if constexpr (is_address<OperandType>::value) {
        emit_u8(BIN(1000, 1111));
        emit_addr(operand, 0b000);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(BIN(1000, 1111));
        emit_u8(_modrm(0b11, 0b000, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}

//===----------------------------------------------------------------------===//
// PUSH - Push Operand onto the Stack
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void x86jitcode::push(OperandType operand) {
    if constexpr (is_immediate<OperandType>::value) {
        emit_u8(is_s<OperandType>::value ? BIN(0110, 1010) : BIN(0110, 1000));
        emit_imm(operand);
    } else if constexpr (is_address<OperandType>::value) {
        emit_u8(BIN(1111, 1111));
        emit_addr(operand, 0b110);
    } else if constexpr (is_register<OperandType>::value) {
        emit_u8(BIN(1111, 1111));
        emit_u8(_modrm(0b11, 0b110, operand.val));
    } else {
        static_assert(true, "unexpects types");
    }
}

//===----------------------------------------------------------------------===//
// RET 每 Return from Procedure (to same segment)
//===----------------------------------------------------------------------===//
inline void x86jitcode::ret() { emit_u8(0xc3); }
//===----------------------------------------------------------------------===//
// SAR 每 Shift Arithmetic Right
//===----------------------------------------------------------------------===//
template <typename OperandType1,typename OperandType2>
inline void x86jitcode::sar(OperandType1 op1,OperandType2 op2) {
    if constexpr (is_address<OperandType1>::value
        &&is_immediate<OperandType2>::value
        && sizeof(OperandType2) == 1) {
        emit_u8(is_w1<OperandType1>::value ? BIN(1100, 0000): BIN(1100, 0001));
        emit_addr(operand, 0b111);
        emit_imm(op2);
    } else if constexpr (is_register<OperandType1>::value
        &&is_immediate<OperandType2>::value
        &&sizeof(OperandType2)==1) {
        emit_u8(is_w1<OperandType1>::value ? BIN(1100, 0000): BIN(1100, 0001));
        emit_u8(_modsib(0b11, 0b111, op1.val));
        emit_imm(op2);
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// SHL 每 Shift Left
//===----------------------------------------------------------------------===//
template <typename OperandType1, typename OperandType2>
inline void x86jitcode::shl(OperandType1 op1, OperandType2 op2) {
    if constexpr (is_address<OperandType1>::value
        &&is_immediate<OperandType2>::value
        && sizeof(OperandType2) == 1) {
        emit_u8(is_w1<OperandType1>::value ? BIN(1100, 0000) : BIN(1100, 0001));
        emit_addr(operand, 0b100);
        emit_imm(op2);
    } else if constexpr (is_register<OperandType1>::value
        &&is_immediate<OperandType2>::value
        && sizeof(OperandType2) == 1) {
        emit_u8(is_w1<OperandType1>::value ? BIN(1100, 0000) : BIN(1100, 0001));
        emit_u8(_modsib(0b11, 0b100, op1.val));
        emit_imm(op2);
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// SHR 每 Shift Right
//===----------------------------------------------------------------------===//
template <typename OperandType1, typename OperandType2>
inline void x86jitcode::shr(OperandType1 op1, OperandType2 op2) {
    if constexpr (is_address<OperandType1>::value
        &&is_immediate<OperandType2>::value
        && sizeof(OperandType2) == 1) {
        emit_u8(is_w1<OperandType1>::value ? BIN(1100, 0000) : BIN(1100, 0001));
        emit_addr(operand, 0b101);
        emit_imm(op2);
    } else if constexpr (is_register<OperandType1>::value
        &&is_immediate<OperandType2>::value
        && sizeof(OperandType2) == 1) {
        emit_u8(is_w1<OperandType1>::value ? BIN(1100, 0000) : BIN(1100, 0001));
        emit_u8(_modsib(0b11, 0b101, op1.val));
        emit_imm(op2);
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// SUB 每 Integer Subtraction
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void x86jitcode::sub(DestType dest, SrcType src) {
    if constexpr (is_reg_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0010, 1000) : BIN(0010, 1001));
        emit_u8(_modrm(0b11, src.val, dest.val));
    } else if constexpr (is_mem_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0010, 1010) : BIN(0010, 1011));
        emit_addr(src, dest.val);
    } else if constexpr (is_reg_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0010, 1000) : BIN(0010, 1001));
        emit_addr(dest, src.val);
    } else if constexpr (is_imm_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0000) : BIN(1000, 0001));
        emit_u8(_modrm(0b11, 0b101, dest.val));
        emit_imm(src);
    } else if constexpr (is_imm_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0000) : BIN(1000, 0001));
        emit_addr(dest, 0b101);
        emit_imm(src);
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// TEST 每 Logical Compare
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void x86jitcode::test(DestType dest, SrcType src) {
    if constexpr (is_reg_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0100) : BIN(1000, 0101));
        emit_u8(_modrm(0b11, src.val, dest.val));
    }
    // test reg,mem is analogous to test mem,reg. Intel instruction manual supports
    // only test mem,reg, so this is a syntaic surgar of supporting test reg,mem
    else if constexpr (is_mem_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0100) : BIN(1000, 0101));
        emit_addr(src, dest.val);
    } else if constexpr (is_reg_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0100) : BIN(1000, 0101));
        emit_addr(dest, src.val);
    } else if constexpr (is_imm_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_u8(_modrm(0b11, 0b000, dest.val));
        emit_imm(src);
    } else if constexpr (is_imm_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1111, 0110) : BIN(1111, 0111));
        emit_addr(dest, 0b000);
        emit_imm(src);
    } else {
        static_assert(true, "unexpects types");
    }
}
//===----------------------------------------------------------------------===//
// SUB 每 Integer Subtraction
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void x86jitcode::xor(DestType dest, SrcType src) {
    if constexpr (is_reg_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0011, 0000) : BIN(0011, 0001));
        emit_u8(_modrm(0b11, src.val, dest.val));
    } else if constexpr (is_mem_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0011, 0010) : BIN(0011, 0011));
        emit_addr(src, dest.val);
    } else if constexpr (is_reg_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(0011, 0000) : BIN(0011, 0001));
        emit_addr(dest, src.val);
    } else if constexpr (is_imm_2_reg<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0000) : BIN(1000, 0001));
        emit_u8(_modrm(0b11, 0b110, dest.val));
        emit_imm(src);
    } else if constexpr (is_imm_2_mem<SrcType, DestType>::value) {
        emit_u8(is_w<SrcType, DestType>::value ? BIN(1000, 0000) : BIN(1000, 0001));
        emit_addr(dest, 0b110);
        emit_imm(src);
    } else {
        static_assert(true, "unexpects types");
    }
}
#endif  // !_INSTR_X86_H
