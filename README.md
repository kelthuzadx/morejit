# morejit
morejit can facilitate generating architecture-dependent machine code at runtime. In general, it was used to build a **JIT** compiler, you can also use it to write some shellcode at runtime to ensure the robustness of your system or do anything which needs dynamically created.

# Overview
This demo shows how to use **morejit**, it will generate machine code for Intel x86 architecture and call it immediately:
```cpp
#include "instrx86.hpp"
#include "morejit.hpp"

int main() {
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
    auto func =
        c.as_function<void (*)(int (*)(const char* const, ...), int, int)>();
    func(&printf, 0xcafe, 0xbabe);
    return 0;
}
```
Simplely put, this codelet allocates an `executable+writable+readable` memory arean using platform-dependent function(For windows, that's `VirtualAlloc/Free`), and fullfil machine code into it. Then it forcely covnerts this memory arean as a function `fun`, passes three arguments and calls it. We will get the following output:
![](docs/demo.png)

# Details
Intel architecture instructions format is as follows:
![](docs/instr_fmt1.png)
