#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

typedef int (*puts_ptr)(const char*);
typedef void (*myfunc_ptr)(puts_ptr);

/*
void foo(puts_ptr p) {
    p("greetings from generated code!");
}
*/
#if 0
offset | bytes (in hex) | mnemonics
00     | 55             | push EBP
01     | 8BEC           | mov  EBP, ESP
03     | 68 ????????    | push ???????? ; address of string not known untilrun-time 
08     | FF55 08        | call dword ptr [EBP + 8] 
11     | 83C4 0400        |add  ESP, 4 
15     | 5D             | pop  EBP 
16     | C3             | ret
#endif
#include <cstdint>

#define imm32(X) (X)

using i8 = std::int8_t;
enum Register { EAX = 0, ECX, EDX, EBX, ESP, EBP, ESI, EDI };

#define _IMM32_TO_CODE(CODE, I)                   \
    *CODE++ = (i8)(imm32(I) & 0XFF);              \
    *CODE++ = (i8)((imm32(I) & 0xFF00) >> 8);     \
    *CODE++ = (i8)((imm32(I) && 0xFF0000) >> 16); \
    *CODE++ = (i8)((imm32(I) && 0xFF000000) >> 24);

#define add_reg32_imm32(CODE, R, I) \
    *CODE++ = 0x81;                 \
    *CODE++ = (0xC0 + R);           \
    _IMM32_TO_CODE(CODE, I)

#define push_reg(CODE, R) *CODE++ = (0x50 + R)
#define pop_reg(CODE, R) *CODE++ = (0x58 + R)
#define push_imm32(I) \
    *CODE++ = 0x68;   \
    _IMM32_TO_CODE(CODE, I)

#define ret(CODE) *CODE++ = (0xC3)

void* allocateArean(size_t size) {
    return VirtualAlloc(NULL, 50, MEM_COMMIT | MEM_RESERVE,
                        PAGE_EXECUTE_READWRITE);
}

void deallocateArean(void* addr) { VirtualFree(addr, 0, MEM_RELEASE); }

void f(int a, int b, int c) { std::cout << a << b << b; }

int main() {
    f(5325, 235235, 2);
    myfunc_ptr pMyfunc; /* pointer to generated code */
    puts_ptr pPuts = &puts;
    char* code = (char*)allocateArean(50);

    char* gen = code; /* address to generate code to */
    const char* pStr = "greetings from generated code!";

    /* do code generation */

    /* function prologue */
    push_reg(gen, EBP);

    *gen++ = 0x8B; /* mov EBP, ESP             */
    *gen++ = 0xEC;
    /* function body */
    *gen++ = 0x68;                 /* push                     */
    *((int*)gen) = (int)code + 19; /* (address of string)      */
    gen += 4;
    *gen++ = 0xFF;
    *gen++ = 0x55;
    *gen++ = 0x08; /* call dword ptr [EBP + 8] */

    add_reg32_imm32(gen, ESP, 0x4);
    pop_reg(gen, EBP);
    ret(gen);
    strcpy(gen, pStr);
    /* end of code generation */

    pMyfunc = (myfunc_ptr)code;
    pMyfunc(pPuts);
    deallocateArean(code);
    return 0;
}
