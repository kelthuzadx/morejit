#ifndef _INSTR_H
#define _INSTR_H

#include "morejit.hpp"
#include "type_x86.hpp"

//===----------------------------------------------------------------------===//
// emit native x86 machine code to jitcode
//===----------------------------------------------------------------------===//
struct x86emitter {
    static void emit_opcode(jitcode& c, uint8_t op) { *c.cur_code++ = op; }

    static void emit_byte(jitcode& c, uint8_t byte) { *c.cur_code++ = byte; }

    template <typename ImmType>
    static void emit_imm(jitcode& c, ImmType imm);

    template <typename ImmType>
    static void emit_addr(jitcode& c, addr<ImmType>& mem, int backfill_reg = 0);

    template <typename SrcType, typename DestType>
    static void emit_reg_2_rm(jitcode& c, SrcType reg, DestType rm,
                              uint8_t opcode_8bits, uint8_t opcode_16_32_bits,
                              uint8_t backfill_reg) {
        if constexpr (is_reg8<DestType>::value) {
            x86emitter::emit_opcode(c, opcode_8bits);
            x86emitter::emit_byte(_modrm(0b11, reg.val, rm.val));
        } else if constexpr (is_reg16or32<DestType>::value) {
            x86emitter::emit_opcode(c, opcode_16_32_bits);
            x86emitter::emit_byte(c, _modrm(0b11, reg.val, rm.val));
        } else if constexpr (is_address<DestType>::value) {
            if constexpr (is_reg8<SrcType>::value) {
                x86emitter::emit_opcode(c, opcode_8bits);
                x86emitter::emit_addr(c, rm, backfill_reg);
            } else if constexpr (is_reg16or32<SrcType>::value) {
                x86emitter::emit_opcode(c, opcode_16_32_bits);
                x86emitter::emit_addr(c, rm, backfill_reg);
            } else {
                static_assert(true,
                              "expects 8/16/32bits register/memory address");
            }
        } else {
            static_assert(true, "expects 8/16/32bits register/memory address");
        }
    }

    template <typename SrcType, typename DestType>
    static void emit_imm_2_rm(jitcode& c, SrcType imm, DestType rm,
                              uint8_t opcode_8bits, uint8_t opcode_16_32_bits,
                              uint8_t backfill_reg) {
        if constexpr (is_reg8<DestType>::value) {
            x86emitter::emit_opcode(c, opcode_8bits);
            x86emitter::emit_byte(c, _modrm(0b11, 0, rm.val));
            x86emitter::emit_imm(c, imm);
        } else if constexpr (is_reg16or32<DestType>::value) {
            x86emitter::emit_opcode(c, opcode_16_32_bits);
            x86emitter::emit_byte(c, _modrm(0b11, 0, rm.val));
            x86emitter::emit_imm(c, imm);
        } else if constexpr (is_address<DestType>::value) {
            if constexpr (is_reg8<SrcType>::value) {
                x86emitter::emit_opcode(c, opcode_8bits);
                x86emitter::emit_addr(c, rm, backfill_reg);
                x86emitter::emit_imm(c, imm);
            } else if constexpr (is_reg16or32<SrcType>::value) {
                x86emitter::emit_opcode(c, opcode_16_32_bits);
                x86emitter::emit_addr(c, rm, backfill_reg);
                x86emitter::emit_imm(c, imm);
            } else {
                static_assert(true,
                              "expects 8/16/32bits register/memory address");
            }
        } else {
            static_assert(true, "expects 8/16/32bits register/memory address");
        }
    }

    template <typename SrcType, typename DestType>
    static void emit_addr_2_reg(jitcode& c, addr<SrcType> addr, DestType reg,
                                uint8_t opcode_8bits, uint8_t opcode_16_32_bits,
                                uint8_t backfill_reg) {
        if constexpr (is_reg8<DestType>::value) {
            x86emitter::emit_opcode(c, opcode_8bits);
            x86emitter::emit_addr(c, addr, backfill_reg);
        } else if constexpr (is_reg16or32<DestType>::value) {
            x86emitter::emit_opcode(c, opcode_16_32_bits);
            x86emitter::emit_addr(c, addr, backfill_reg);
        } else {
            static_assert(true, "expects 8/16/32bits register");
        }
    }
};

template <typename ImmType>
inline void x86emitter::emit_imm(jitcode& c, ImmType imm) {
    *(decltype(imm.val)*)c.cur_code = imm.val;
    c.cur_code += imm.get_bytes();
}

template <typename ImmType>
inline void x86emitter::emit_addr(jitcode& c, addr<ImmType>& mem,
                                  int backfill_reg) {
    _backfill_regfield(mem.modrm, backfill_reg);
    *c.cur_code++ = mem.modrm;
    if (mem.modsib.has_value()) {
        *c.cur_code++ = mem.modsib.value();
    }
    if (mem.disp.has_value()) {
        emit_imm(c, mem.disp.value());
    }
}

namespace instr {
//===----------------------------------------------------------------------===//
// mov
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void mov(jitcode& c, DestType dest, SrcType src) {
    if constexpr (is_immediate<SrcType>::value) {
        x86emitter::emit_imm_2_rm(c, src, dest, 0xb0 + dest.val,
                                  0xb8 + dest.val, 0);
    } else if constexpr (is_register<SrcType>::value) {
        x86emitter::emit_reg_2_rm(c, src, dest, 0x88, 0x89, src.val);
    } else if constexpr (is_address<SrcType>::value) {
        x86emitter::emit_addr_2_reg(c, src, dest, 0x8a, 0x8b, dest.val);
    } else {
        static_assert(true, "only expects type of address/register/immediate");
    }
}
//===----------------------------------------------------------------------===//
// add
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void add(jitcode& c, DestType dest, SrcType src) {
    if constexpr (is_immediate<SrcType>::value) {
        x86emitter::emit_imm_2_rm(c, src, dest, 0x80, 0x81, 0);
    } else if constexpr (is_register<SrcType>::value) {
        x86emitter::emit_reg_2_rm(c, src, dest, 0x0, 0x1, src.val);
    } else if constexpr (is_address<SrcType>::value) {
        x86emitter::emit_addr_2_reg(c, src, dest, 0x2, 0x3, dest.val);
    } else {
        static_assert(true, "only expects type of address/register/immediate");
    }
}
//===----------------------------------------------------------------------===//
// sub
//===----------------------------------------------------------------------===//
template <typename DestType, typename SrcType>
inline void sub(jitcode& c, DestType dest, SrcType src) {
    if constexpr (is_immediate<SrcType>::value) {
        x86emitter::emit_imm_2_rm(c, src, dest, 0x80, 0x81, 5);
    } else if constexpr (is_register<SrcType>::value) {
        x86emitter::emit_reg_2_rm(c, src, dest, 0x28, 0x29, src.val);
    } else if constexpr (is_address<SrcType>::value) {
        x86emitter::emit_addr_2_reg(c, src, dest, 0x2a, 0x2b, dest.val);
    } else {
        static_assert(true, "only expects type of address/register/immediate");
        ;
    }
}
//===----------------------------------------------------------------------===//
// call
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void call(jitcode& c, OperandType operand) {
    if constexpr (is_immediate<OperandType>::value) {
        x86emitter::emit_opcode(c, 0xe8);
        x86emitter::emit_imm(c, operand);
    } else if constexpr (is_address<OperandType>::value) {
        x86emitter::emit_opcode(c, 0xff);
        x86emitter::emit_addr(c, operand, 2);
    } else {
        static_assert(true, "only expects type of address/immediate");
    }
}
//===----------------------------------------------------------------------===//
// push
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void push(jitcode& c, OperandType operand) {
    if constexpr (is_immediate<OperandType>::value) {
        x86emitter::emit_opcode(
            c, is_same<OperandType, imm8>::value ? 0x6a : 0x68);
        x86emitter::emit_imm(c, operand);
    } else if constexpr (is_address<OperandType>::value) {
        x86emitter::emit_opcode(c, 0xff);
        x86emitter::emit_addr(c, operand, 6);
    } else if constexpr (is_register<OperandType>::value) {
        x86emitter::emit_opcode(c, 0x50 + operand.val);
    } else {
        static_assert(true, "only expects type of address/register/immediate");
    }
}
//===----------------------------------------------------------------------===//
// pop
//===----------------------------------------------------------------------===//
template <typename OperandType>
inline void pop(jitcode& c, OperandType operand) {
    if constexpr (is_register<OperandType>::value) {
        x86emitter::emit_opcode(c, 0x58 + operand.val);
    } else {
        static_assert(true, "only expects type of register");
    }
}
//===----------------------------------------------------------------------===//
// ret
//===----------------------------------------------------------------------===//
inline void ret(jitcode& c) { x86emitter::emit_opcode(c, 0xc3); }
};      // namespace instr
#endif  // !_INSTR_H
