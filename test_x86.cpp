#include "instr_x86.hpp"
#include "morejit.hpp"
void test_arch_x86() {
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
#if 0
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

int main() {
    jitcode c{500};
    c.write_text("greeting %x%x!");

    // _i32/_i16/_i8 are user defined literal suffix, they are just
    // analogous to imm32(val) imm16(val) and imm8(val)
    gen_jitcode {
        push(c, ebp);
        mov(c, ebp, esp);
        push(c, addr(ebp, 16_i32));
        push(c, addr(ebp, 12_i32));
        push(c, imm32((int)c.get_text_ptr()));
        call(c, addr(ebp, 8_i32));
        add(c, esp, 12_i32);
        pop(c, ebp);
        ret(c);
    }
    c.dump();

    auto func =
        c.as_function<void (*)(int (*)(const char* const, ...), int, int)>();
    func(&printf, 0xcafe, 0xbabe);
    getchar();
    return 0;
}
