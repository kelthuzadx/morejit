#ifndef _X86_MOREJIT
#define _X86_MOREJIT
#include "morejit.hpp"
#include "type_x86.hpp"

struct x86jitcode : public jitcode {
    explicit x86jitcode(int alloc_size) : jitcode(alloc_size) {}

    template <typename ImmType>
    inline void emit_imm(ImmType imm);

    template <typename ImmType>
    inline void emit_addr(addr<ImmType>& mem, int backfill_reg);

    //===----------------------------------------------------------------------===//
    // ADD 每 Add
    //===----------------------------------------------------------------------===//
    template <typename DestType, typename SrcType>
    inline void add(DestType dest, SrcType src);

    //===----------------------------------------------------------------------===//
    // CALL 每 Call Procedure (in same segment)
    //===----------------------------------------------------------------------===//
    template <typename OperandType>
    inline void call(OperandType operand);

    //===----------------------------------------------------------------------===//
    // MOV 每 Move Data
    //===----------------------------------------------------------------------===//
    template <typename DestType, typename SrcType>
    inline void mov(DestType dest, SrcType src);

    //===----------------------------------------------------------------------===//
    // POP 每 Pop a Word from the Stack
    //===----------------------------------------------------------------------===//
    template <typename OperandType>
    inline void pop(OperandType operand);

    //===----------------------------------------------------------------------===//
    // PUSH 每 Push Operand onto the Stack
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

template <typename ImmType>
inline void x86jitcode::emit_addr(addr<ImmType>& mem, int backfill_reg) {
    _backfill_regfield(mem.modrm, backfill_reg);
    *cur_code++ = mem.modrm;
    if (mem.modsib.has_value()) {
        *cur_code++ = mem.modsib.value();
    }
    if (mem.disp.has_value()) {
        emit_imm(mem.disp.value());
    }
}
#endif  // !_X86_CODEJIT
