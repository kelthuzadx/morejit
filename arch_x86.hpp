#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>
#include <iostream>
#include <optional>
#include <type_traits>

#if 0
// Demos
offset | bytes (in hex) | mnemonics
00     | 55             | push EBP
01     | 8BEC           | mov  EBP, ESP
03     | 68 ????????    | push ???????? ; address of string not known untilrun-time 
08     | FF55 08        | call dword ptr [EBP + 8] 
11     | 81C4 0400      | add  ESP, 4 
15     | 5D             | pop  EBP 
16     | C3             | ret

   printf("%d%d%d", 5325, 235235, k);
003D50B1 8B 45 EC             mov         eax,dword ptr [k]  
003D50B4 50                   push        eax  
003D50B5 68 E3 96 03 00       push        396E3h  
003D50BA 68 CD 14 00 00       push        14CDh  
003D50BF 68 30 7B 3D 00       push        offset string "%d%d%d" (03D7B30h)  
003D50C4 E8 FF FF       call        _printf (03D13ACh)  
003D50C9 83 C4 10             add         esp,10h
#endif

#define call(CODE, I) \
    *CODE++ = 0xe8;   \
    _imm32(CODE, I);

// call [eax+dispatch8]
#define call_8(CODE, R, DISP)   \
    *CODE++ = 0xff;             \
    *CODE++ = _modrm(01, 2, R); \
    *CODE++ = DISP;

#define add_reg32_imm32(CODE, R, I) \
    *CODE++ = 0x81;                 \
    *CODE++ = _modrm(11, 000, R);   \
    _imm32(CODE, I)

#define pop_reg(CODE, R) *CODE++ = (0x58 + R);

#define ret(CODE) *CODE++ = (0xC3)

#define INVALID (0)
#define MOREJIT_DEBUG

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

enum r : unsigned int { eax, ecx, edx, ebx, esp, ebp, esi, edi };

using namespace std;

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
    explicit addr(ImmType disp32) : modsib(nullopt), disp(disp32) {
        static_assert(is_same<ImmType, imm32>::value,
                      "expects 32 bits immediate number");
        modrm = _modrm(0b00, INVALID, 0b101);
    }
    explicit addr(r reg) { modrm = _modrm(0b00, INVALID, (int)reg); }

    explicit addr(r reg, ImmType disp) : modsib(nullopt), disp(disp) {
        modrm = _modrm((is_same<ImmType, imm32>::value ? 0b10 : 0b01), INVALID,
                       (int)reg);
    }

    explicit addr(r base, r scale, uint8_t index) {
        modrm = _modrm(0b00, INVALID, 0b100);
        _modsib(modsib, base, scale, index);
    }

    explicit addr(r base, r scale, uint8_t index, ImmType disp) : disp(disp) {
        modrm = _modrm((is_same<ImmType, imm32>::value ? 0b10 : 0b01), INVALID,
                       0b100);
        _modsib(modsib, base, scale, index);
    }

    uint8_t modrm;
    optional<uint8_t> modsib;
    optional<ImmType> disp;
};
class jitcode {
public:
    explicit jitcode(size_t code_size) : code_size(code_size) {
        code_start = (char*)VirtualAlloc(
            NULL, code_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        cur_code = (char*)code_start;
    }
    ~jitcode() {
        VirtualFree((char*)code_start, 0, MEM_RELEASE);
        cur_code = nullptr;
    }

public:
    void print() {
        int i = 0;
        while (i != code_size) {
            if (code_start[i] == '\n') {
                printf("\n");
                i++;
                continue;
            }

            printf("%02x ", (unsigned char)code_start[i]);

            i++;
        }
    }

    template <typename FuncPtrType>
    FuncPtrType as_function() {
        static_assert(is_pointer<FuncPtrType>::value,
                      "expects function pointer type");
        return (FuncPtrType)code_start;
    }

public:
    void emit_opcode(uint8_t op) { *cur_code++ = op; }

    template <typename ImmType>
    void emit_imm(ImmType imm) {
        *(decltype(imm.val)*)cur_code = imm.val;
        cur_code += imm.get_bytes();
    }
    template <typename ImmType>
    void emit_addr(addr<ImmType>& mem, int backfill_reg = 0) {
        _backfill_regfield(mem.modrm, backfill_reg);
        *cur_code++ = mem.modrm;
        if (mem.modsib.has_value()) {
            *cur_code++ = mem.modsib.value();
        }
        if (mem.disp.has_value()) {
            emit_imm(mem.disp.value());
        }
    }

    void emit_byte(uint8_t byte) { *cur_code++ = byte; }

private:
    int code_size;
    const char* code_start;
    char* cur_code;

private:
    jitcode& operator=(const jitcode&) = delete;
    jitcode(const jitcode&) = delete;
};

namespace instr {
//===----------------------------------------------------------------------===//
// mov
//===----------------------------------------------------------------------===//
template <typename ImmType>
static void mov(jitcode& c, r dest, ImmType imm) {
    c.emit_opcode(0xb8 + (int)dest);
    c.emit_imm(imm);
#ifdef MOREJIT_DEBUG
    c.emit_byte('\n');
#endif
}

template <typename DispImmType, typename ImmType>
static void mov(jitcode& c, addr<DispImmType> mem, ImmType imm) {
    c.emit_opcode(0xc7);
    c.emit_addr(mem);
    c.emit_imm(imm);
#ifdef MOREJIT_DEBUG
    c.emit_byte('\n');
#endif
}
static void mov(jitcode& c, r dest, r src) {
    c.emit_opcode(0x8b);
    c.emit_byte(_modrm(0b11, (int)dest, (int)src));
#ifdef MOREJIT_DEBUG
    c.emit_byte('\n');
#endif
}

template <typename ImmType>
static void mov(jitcode& c, r dest, addr<ImmType> mem) {
    c.emit_opcode(0x8b);
    c.emit_addr(mem);
#ifdef MOREJIT_DEBUG
    c.emit_byte('\n');
#endif
}
template <typename ImmType>
static void mov(jitcode& c, addr<ImmType> mem, r src) {
    c.emit_opcode(0x89);
    c.emit_addr(mem, (int)src);
#ifdef MOREJIT_DEBUG
    c.emit_byte('\n');
#endif
}
//===----------------------------------------------------------------------===//
// push
//===----------------------------------------------------------------------===//
template <typename ImmType>
static void push(jitcode& c, ImmType imm) {
    c.emit_opcode(is_same<ImmType, imm8>::value ? 0x6a : 0x68);
    c.emit_imm(imm);
#ifdef MOREJIT_DEBUG
    c.emit_byte('\n');
#endif
}

static void push(jitcode& c, r reg) {
    c.emit_opcode(0x50 + reg);
#ifdef MOREJIT_DEBUG
    c.emit_byte('\n');
#endif
}
//===----------------------------------------------------------------------===//
// pop
//===----------------------------------------------------------------------===//
static void pop(jitcode& c, r reg) {
    c.emit_opcode(0x58 + reg);
#ifdef MOREJIT_DEBUG
    c.emit_byte('\n');
#endif
}
};  // namespace instr
