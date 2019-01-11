#ifndef _MOREJIT_H
#define _MOREJIT_H
#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <type_traits>

#define gen_jitcode using namespace instr;

#define _backfill_regfield(MODRM, REG) MODRM |= ((REG & 7) << 3);

#define _modrm(MOD, REG, RM) \
    (uint8_t)((RM & 0x7) | ((REG & 0x7) << 3) | (MOD << 6))

#define _modsib(modsib, base, scale, index)                      \
    if (scale == r::esp) {                                       \
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
using namespace std;
enum r : unsigned int { eax, ecx, edx, ebx, esp, ebp, esi, edi };

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
    explicit addr(r reg);

    explicit addr(r reg, ImmType disp);

    explicit addr(r base, r scale, uint8_t index);

    explicit addr(r base, r scale, uint8_t index, ImmType disp);

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

    void print();

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

#endif
