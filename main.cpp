#include "arch_x86.hpp"

int main() {
    gen_jitcode {
        jitcode c(500);
#if 0
        push(c, eax);
        push(c, ebx);
        push(c, imm8(12));
        push(c, imm16(1234));
        push(c, imm32(0x1234546));
        pop(c, eax);
        pop(c, ebx);
        c.print();
        __asm {
            push eax
            push ebx
            push 12
            push 1234
            push 123456h
            pop eax
            pop ebx
        }
#endif
#if 1
        mov(c, ebx, imm32(14));
        mov(c, addr(eax), imm32(0x435000));
        mov(c, ebx, edi);

        mov(c, ebx, addr(imm32(32)));
        mov(c, ebx, addr(eax));
        mov(c, ebx, addr(ebx, imm8(32)));
        mov(c, ebx, addr(eax, ebx, 8, imm8(45)));
        mov(c, ebx, addr(eax, ecx, 2));
        mov(c, edi, addr(eax, edx, 0));

        mov(c, addr(imm32(32)), ebx);
        mov(c, addr(eax), ebx);
        mov(c, addr(ebx, imm8(32)), ebx);
        mov(c, addr(eax, ebx, 8, imm8(45)), ebx);
        mov(c, addr(eax, ecx, 2), ebx);
        mov(c, addr(eax, edx, 0), edi);

        mov(c, addr(ebx, imm8(32)), imm32(0x535000));
        mov(c, addr(eax, ebx, 8, imm32(0x12345678)), imm32(0x635000));
        mov(c, addr(eax, ecx, 2), imm32(0x735000));
        mov(c, addr(eax, edx, 0), imm32(0x835000));
        c.print();
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
            mov dword ptr[eax+ebx*8+12345678h],635000h
            mov dword ptr[eax+ecx*2],735000h
            mov dword ptr[eax+edx],835000h
        }
#endif
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
