#ifndef _MOREJIT_H
#define _MOREJIT_H
#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <type_traits>

#define gen_jitcode using namespace instr;

using std::optional;
using std::uint32_t;
using std::uint8_t;

enum reg : unsigned int { eax, ecx, edx, ebx, esp, ebp, esi, edi };

struct imm32 {
    explicit imm32(uint32_t val) : val(val) {}
    static constexpr int get_bytes() { return 4; }
    uint32_t val;
};

struct imm16 {
    explicit imm16(uint16_t val) : val(val) {}
    static constexpr int get_bytes() { return 2; }
    uint16_t val;
};

struct imm8 {
    explicit imm8(uint8_t val) : val(val) {}
    static constexpr int get_bytes() { return 1; }
    uint8_t val;
};

template <typename ImmType = imm32, int Size = 32>
struct addr {
    explicit addr(ImmType disp32);
    explicit addr(reg reg);

    explicit addr(reg reg, ImmType disp);

    explicit addr(reg base, reg scale, uint8_t index);

    explicit addr(reg base, reg scale, uint8_t index, ImmType disp);

    uint8_t modrm;
    optional<uint8_t> modsib;
    optional<ImmType> disp;

    static const int INVALID = 0;
};

class jitcode {
public:
    explicit jitcode(size_t alloc_size);
    ~jitcode();

public:
    int get_alloc_size() const;

    int get_code_size() const;

    int get_text_size() const;

    char* get_text_ptr(int offset = 0);

    void dump();

    template <typename FuncPtrType>
    FuncPtrType as_function();

    void write_text(const char* str);

public:
    void emit_opcode(uint8_t op) { *cur_code++ = op; }

    void emit_byte(uint8_t byte) { *cur_code++ = byte; }

    template <typename ImmType>
    void emit_imm(ImmType imm);

    template <typename ImmType>
    void emit_addr(addr<ImmType>& mem, int backfill_reg = 0);

private:
    jitcode& operator=(const jitcode&) = delete;
    jitcode(const jitcode&) = delete;

private:
    const int alloc_size;
    const char* alloc_start;
    int text_size;
    char* cur_code;
};

imm32 operator"" _i32(unsigned long long val);

imm16 operator"" _i16(unsigned long long val);

imm8 operator"" _i8(unsigned long long val);

#endif
