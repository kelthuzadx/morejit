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

/// All instructions are destination operand first. For example, give a
/// mov_reg32_reg32(ESP,EBP), it will generate mov esp<-ebp

#define _imm32(CODE, I)                   \
    *CODE++ = (i8)(I & 0XFF);             \
    *CODE++ = (i8)((I & 0xFF00) >> 8);    \
    *CODE++ = (i8)((I & 0xFF0000) >> 16); \
    *CODE++ = (i8)((I & 0xFF000000) >> 24);

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

#define push_reg(CODE, R) *CODE++ = (0x50 + R);
#define pop_reg(CODE, R) *CODE++ = (0x58 + R);
#define push_imm32(CODE, I) \
    *CODE++ = 0x68;         \
    _imm32(CODE, I)

#define ret(CODE) *CODE++ = (0xC3)

#define mov_reg32_reg32(CODE, R1, R2) \
    *CODE++ = 0x8b;                   \
    *CODE++ = _modrm(11, R1, R2);

#define mov_reg32_imm32(CODE, R, I) \
    *CODE++ = (0xb8 + R);           \
    _imm32(CODE, I);
using namespace std;

#define _modrm(MOD, REG, RM) \
    (uint8_t)((RM & 0x7) | ((REG & 0x7) << 3) | (MOD << 6))
#define _get_mode(MODRM) (MODRM & 0xC0) >> 6
#define _get_reg(MODRM) (MODRM & 0X38) >> 3
#define _get_rm(MODRM) (MODRM & 7)

#define _modsib(SCALE, INDEX, BASE) _modrm(SCALE, INDEX, BASE)

#define INVALID (0)
#define MOREJIT_DEBUG 1

#define ENABLE_DEBUG()        \
    if (MOREJIT_DEBUG) {      \
        *c.cur_code++ = '\n'; \
    }

enum r : unsigned int { eax, ecx, edx, ebx, esp, ebp, esi, edi };

struct instr;

class jitcode {
    friend struct instr;

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

private:
    jitcode& operator=(const jitcode&) = delete;
    jitcode(const jitcode&) = delete;

private:
    int code_size;
    const char* code_start;
    char* cur_code;
};

template <int ImmSize>
struct imm {
    explicit imm() {
        static_assert(true, "only expects 8/16/32 bits immediate number");
    }
};
template <>
struct imm<32> {
    explicit imm(uint32_t val) : val(val) {}
    static constexpr int get_bytes() { return 4; }
    uint32_t val;
};
template <>
struct imm<16> {
    explicit imm(uint16_t val) : val(val) {}
    static constexpr int get_bytes() { return 2; }
    uint16_t val;
};
template <>
struct imm<8> {
    explicit imm(uint8_t val) : val(val) {}
    static constexpr int get_bytes() { return 1; }
    uint8_t val;
};

#define _backfill_regfield(MODRM, REG) MODRM |= ((REG & 7) << 3);

template <typename ImmType = imm<32>, int Size = 32>
struct addr {
    explicit addr(ImmType disp32) : modsib(nullopt), disp(disp32) {
        static_assert(is_same<ImmType, imm<32>>::value,
                      "expects 32 bits immediate number");
        modrm = _modrm(0b00, INVALID, 0b101);
    }
    explicit addr(r reg) { modrm = _modrm(0b00, INVALID, (int)reg); }

    explicit addr(r reg, ImmType disp) : modsib(nullopt), disp(disp) {
        modrm = _modrm((is_same<ImmType, imm<32>>::value ? 0b10 : 0b01),
                       INVALID, (int)reg);
    }
    explicit addr(r base, r scale, uint8_t index) {
        modrm = _modrm(0b00, INVALID, 0b100);
        if (scale == r::esp) {
            static_assert(true, "scale register should not be ESP");
        }
        switch (index) {
            case 0:
                modsib = _modsib(0b00, (int)scale, (int)base);
                break;
            case 2:
                modsib = _modsib(0b01, (int)scale, (int)base);
                break;
            case 4:
                modsib = _modsib(0b10, (int)scale, (int)base);
                break;
            case 8:
                modsib = _modsib(0b11, (int)scale, (int)base);
                break;
        }
    }

    explicit addr(r base, r scale, uint8_t index, ImmType disp) : disp(disp) {
        modrm = _modrm((is_same<ImmType, imm<32>>::value ? 0b10 : 0b01),
                       INVALID, 0b100);
        if (scale == r::esp) {
            static_assert(true, "scale register should not be ESP");
        }

        if (index == 0) {
            modsib = _modsib(0b00, (int)scale, (int)base);
        } else if (index == 2) {
            modsib = _modsib(0b01, (int)scale, (int)base);
        } else if (index == 4) {
            modsib = _modsib(0b10, (int)scale, (int)base);
        } else if (index == 8) {
            modsib = _modsib(0b11, (int)scale, (int)base);
        }
    }

    uint8_t modrm;
    optional<uint8_t> modsib;
    optional<ImmType> disp;
};

struct instr {
    template <int ImmSize>
    static void mov(jitcode& c, r dest, imm<ImmSize> imm) {
        *c.cur_code++ = (0xb8 + (int)dest);
        *(decltype(imm.val)*)c.cur_code = imm.val;
        c.cur_code += imm.get_bytes();
        ENABLE_DEBUG()
    }

    template <typename DispImmType, int ImmSize>
    static void mov(jitcode& c, addr<DispImmType> mem, imm<ImmSize> imm) {
        *c.cur_code++ = 0xc7;
        _backfill_regfield(mem.modrm, 0);
        *c.cur_code++ = mem.modrm;
        if (mem.modsib.has_value()) {
            *c.cur_code++ = mem.modsib.value();
        }
        if (mem.disp.has_value()) {
            *(decltype(mem.disp.value().val)*)c.cur_code = mem.disp.value().val;
            c.cur_code += mem.disp.value().get_bytes();
        }
        *(decltype(imm.val)*)c.cur_code = imm.val;
        c.cur_code += imm.get_bytes();
        ENABLE_DEBUG()
    }
    static void mov(jitcode& c, r dest, r src) {
        *c.cur_code++ = 0x8b;
        *c.cur_code++ = _modrm(0b11, (int)dest, (int)src);
        ENABLE_DEBUG()
    }

    template <typename ImmType>
    static void mov(jitcode& c, r dest, addr<ImmType> mem) {
        *c.cur_code++ = 0x8b;
        _backfill_regfield(mem.modrm, (int)dest);
        *c.cur_code++ = mem.modrm;
        if (mem.modsib.has_value()) {
            *c.cur_code++ = mem.modsib.value();
        }
        if (mem.disp.has_value()) {
            *(decltype(mem.disp.value().val)*)c.cur_code = mem.disp.value().val;
            c.cur_code += mem.disp.value().get_bytes();
        }
        ENABLE_DEBUG()
    }
    template <typename ImmType>
    static void mov(jitcode& c, addr<ImmType> mem, r src) {
        *c.cur_code++ = 0x89;
        _backfill_regfield(mem.modrm, (int)src);
        *c.cur_code++ = mem.modrm;
        if (mem.modsib.has_value()) {
            *c.cur_code++ = mem.modsib.value();
        }
        if (mem.disp.has_value()) {
            *(decltype(mem.disp.value().val)*)c.cur_code = mem.disp.value().val;
            c.cur_code += mem.disp.value().get_bytes();
        }
        ENABLE_DEBUG()
    }
};

#define __gen_jitcode

int main() {
    __gen_jitcode {
        jitcode c(500);
        instr::mov(c, ebx, imm<32>(14));
        instr::mov(c, addr(eax), imm<32>(0x435000));
        instr::mov(c, ebx, edi);

        instr::mov(c, ebx, addr(imm<32>(32)));
        instr::mov(c, ebx, addr(eax));
        instr::mov(c, ebx, addr(ebx, imm<8>(32)));
        instr::mov(c, ebx, addr(eax, ebx, 8, imm<8>(45)));
        instr::mov(c, ebx, addr(eax, ecx, 2));
        instr::mov(c, edi, addr(eax, edx, 0));

        instr::mov(c, addr(imm<32>(32)), ebx);
        instr::mov(c, addr(eax), ebx);
        instr::mov(c, addr(ebx, imm<8>(32)), ebx);
        instr::mov(c, addr(eax, ebx, 8, imm<8>(45)), ebx);
        instr::mov(c, addr(eax, ecx, 2), ebx);
        instr::mov(c, addr(eax, edx, 0), edi);

        instr::mov(c, addr(ebx, imm<8>(32)), imm<32>(0x535000));
        instr::mov(c, addr(eax, ebx, 8, imm<8>(45)), imm<32>(0x635000));
        instr::mov(c, addr(eax, ecx, 2), imm<32>(0x735000));
        instr::mov(c, addr(eax, edx, 0), imm<32>(0x835000));
        c.print();
#if 0
        typedef void (*func_ptr)(int __cdecl printf(char const* const, ...),
                                 int, int);
        auto f = c.as_function<func_ptr>();
        f(&printf, 0xab, 0xcd);
#endif
    }
    __asm {
        mov ebx,14
        mov dword ptr[eax],435000h
        mov ebx,edi
        mov ebx,[32]
        mov ebx,[eax]
        mov ebx,[ebx+32]
        mov ebx,[eax+ebx*8+45]
        mov ebx,[eax+ecx*2]
        mov edi,[eax+edx]
        mov dword ptr[eax],ebx
        mov dword ptr[ebx+32], ebx
        mov dword ptr[eax+ebx*8+45],ebx
        mov dword ptr[eax+ecx*2],ebx
        mov dword ptr[eax+edx],edi
        mov dword ptr[ebx+32],535000h
        mov dword ptr[eax+ebx*8+45],635000h
        mov dword ptr[eax+ecx*2],735000h
        mov dword ptr[eax+edx],835000h
    }
#if 0
    // Startup
    code_ptr code_start = init_jit_code(500);

    const char* format = "Got %d,%d";
    strcpy((char*)code_start, format);
    code_start[strlen(format)] = '\0';

    code_ptr start = code_start + strlen(format) + 1;
    code_ptr gen = start;
    // prologue
    push_reg(gen, EBP);
    mov_reg32_reg32(gen, EBP, ESP);

    push_imm32(gen, 0XAB);
    push_imm32(gen, 0XCD);
    push_imm32(gen, (int)format);
    mov_reg32_reg32(gen, EBX, EBP);
    call_8(gen, EBX, 8);  // call [esp+8]

    add_reg32_imm32(gen, ESP, 12);

    // epilogue
    pop_reg(gen, EBP);
    ret(gen);

    func_ptr pMyfunc = (func_ptr)(start);
    pMyfunc(&printf, 0XAB, 0XCD);

    free_jit_code(code_start);
#endif
    return 0;
}
