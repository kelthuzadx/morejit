#ifndef _X86_MOREJIT
#define _X86_MOREJIT
#include "morejit.hpp"
#include "type_x86.hpp"

struct x86jitcode : public jitcode {
    explicit x86jitcode(int alloc_size) : jitcode(alloc_size) {}

    template <typename ImmType>
    inline void emit_imm(ImmType imm);

    inline void emit_addr(addr mem, int backfill_reg);

    //===----------------------------------------------------------------------===//
    // ADD �C Add
    //===----------------------------------------------------------------------===//
    template <typename DestType, typename SrcType>
    inline void add(DestType dest, SrcType src);

    //===----------------------------------------------------------------------===//
    // AND �C Logical AND
    //===----------------------------------------------------------------------===//
    template <typename DestType, typename SrcType>
    inline void and_(DestType dest, SrcType src);

    //===----------------------------------------------------------------------===//
    // CALL �C Call Procedure (in same segment)
    //===----------------------------------------------------------------------===//
    template <typename OperandType>
    inline void call(OperandType operand);

    
    //===----------------------------------------------------------------------===//
    // CMP �C Compare Two Operands
    //===----------------------------------------------------------------------===//
    template <typename DestType, typename SrcType>
    inline void cmp(DestType dest, SrcType src);

    //===----------------------------------------------------------------------===//
    // DEC �C Decrement by 1
    //===----------------------------------------------------------------------===//
    template <typename OperandType>
    inline void dec(OperandType operand);

    //===----------------------------------------------------------------------===//
    // DIV �C Unsigned Divide
    //===----------------------------------------------------------------------===//
    template <typename OperandType>
    inline void div(OperandType operand);

    //===----------------------------------------------------------------------===//
    // HLT �C Hal
    //===----------------------------------------------------------------------===//
    inline void hlt();

    //===----------------------------------------------------------------------===//
    // IDIV �C Signed Divide
    //===----------------------------------------------------------------------===//
    template <typename OperandType>
    inline void idiv(OperandType operand);

    //===----------------------------------------------------------------------===//
    // IMUL �C Signed Multiply
    //===----------------------------------------------------------------------===//
    template <typename OperandType>
    inline void imul(OperandType operand);
    template <typename DestType, typename SrcType>
    inline void imul(DestType dest, SrcType src);
    //!template <typename OperandType1, typename OperandType2, typename OperandType3>
    //!inline void imul(OperandType1 op1, OperandType2 op2, OperandType3 op3);

    //===----------------------------------------------------------------------===//
    // INC �C Increment by 1
    //===----------------------------------------------------------------------===//
    template <typename OperandType>
    inline void inc(OperandType operand);

    //===----------------------------------------------------------------------===//
    // JMP �C Unconditional Jump (to same segment)
    //===----------------------------------------------------------------------===//
    template <typename OperandType>
    inline void jmp(OperandType operand);

    //===----------------------------------------------------------------------===//
    // MOV �C Move Data
    //===----------------------------------------------------------------------===//
    template <typename DestType, typename SrcType>
    inline void mov(DestType dest, SrcType src);

    //===----------------------------------------------------------------------===//
    // POP �C Pop a Word from the Stack
    //===----------------------------------------------------------------------===//
    template <typename OperandType>
    inline void pop(OperandType operand);

    //===----------------------------------------------------------------------===//
    // PUSH �C Push Operand onto the Stack
    //===----------------------------------------------------------------------===//
    template <typename OperandType>
    inline void push(OperandType operand);

    //===----------------------------------------------------------------------===//
    // RET
    //===----------------------------------------------------------------------===//
    inline void ret();
};

template<typename ImmType> 
inline void x86jitcode::emit_imm(ImmType imm) {
    *(decltype(imm)*)cur_code = imm;
    cur_code += sizeof(imm);
}

inline void x86jitcode::emit_addr(addr mem, int backfill_reg) {
    _backfill_regfield(mem.modrm, backfill_reg);
    *cur_code++ = mem.modrm;
    if (mem.modsib.has_value()) {
        *cur_code++ = mem.modsib.value();
    }
    if (mem.disp8.has_value()) {
        emit_imm(mem.disp8.value());
    } else if (mem.disp32.has_value()) {
        emit_imm(mem.disp32.value());
    }
}
#endif  // !_X86_CODEJIT
