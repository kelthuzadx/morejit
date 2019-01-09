#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*puts_ptr)(const char*);
typedef void (*myfunc_ptr)(puts_ptr);

/*
void foo(puts_ptr p) {
    p("greetings from generated code!");
}
*/
/* corresponding assembler for generated code is like:
offset | bytes (in hex) | mnemonics
00     | 55             | push EBP
01     | 8BEC           | mov  EBP, ESP
03     | 68 ????????    | push ???????? ; address of string not known until
run-time 08     | FF55 08        | call dword ptr [EBP + 8] 11     | 83C4 04 |
add  ESP, 4 14     | 5D             | pop  EBP 15     | C3             | ret
*/

void* allocateArean(size_t size) {
    return VirtualAlloc(NULL, 50, MEM_COMMIT | MEM_RESERVE,
                        PAGE_EXECUTE_READWRITE);
}

void deallocateArean(void* addr) { VirtualFree(addr, 0, MEM_RELEASE); }

int main() {
    myfunc_ptr pMyfunc; /* pointer to generated code */
    puts_ptr pPuts = &puts;
    char* pCode = (char*)allocateArean(50);

    memset(pCode, '\0', 50);
    char* pGen = pCode; /* address to generate code to */
    const char* pStr = "greetings from generated code!";

    /* do code generation */

    /* function prologue */
    *pGen++ = 0x55; /* push EBP                 */
    *pGen++ = 0x8B;
    *pGen++ = 0xEC; /* mov EBP, ESP             */
    /* function body */
    *pGen++ = 0x68;                  /* push                     */
    *((int*)pGen) = (int)pCode + 16; /* (address of string)      */
    pGen += 4;
    *pGen++ = 0xFF;
    *pGen++ = 0x55;
    *pGen++ = 0x08; /* call dword ptr [EBP + 8] */
    *pGen++ = 0x83;
    *pGen++ = 0xC4;
    *pGen++ = 0x04; /* add ESP, 4               */
    /* function epilogue */
    *pGen++ = 0x5D; /* pop EBP                  */
    *pGen++ = 0xC3; /* ret                      */
    /* make a copy of the string, following the generated code */
    strcpy(pGen, pStr);

    /* end of code generation */

    /* invoke generated code */
    pMyfunc = (myfunc_ptr)pCode;
    pMyfunc(pPuts);
    getchar();
    deallocateArean(pCode);
    return 0;
}
