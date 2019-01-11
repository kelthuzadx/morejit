# morejit
morejit can facilitate generating architecture-dependent machine code at runtime. In general, it was used to build a **JIT** compiler, you can also use it to write some shellcode at runtime to ensure the robustness of your system or do anything which needs dynamically created.

# Overview
This demo shows how to use **morejit**, it will generate machine code for Intel x86 architecture and call it immediately:
```cpp
#include "instrx86.hpp"
#include "morejit.hpp"

int main() {
    gen_jitcode {
        jitcode c{500};
        c.write_text("greeting %x%x!");
        push(c, ebp);
        mov(c, ebp, esp);
        push(c, addr(ebp, imm32(16)));
        push(c, addr(ebp, imm32(12)));
        push(c, imm32((int)c.get_text_ptr()));
        call(c, addr(ebp, imm32(8)));
        add(c, esp, imm32(12));
        pop(c, ebp);
        ret(c);
        auto func = c.as_function<void (*)(int (*)(const char* const, ...), int,
                                        int)>();
        func(&printf, 0xcafe, 0xbabe);
    }
    return 0;
}
```
Simplely put, this codelet allocates an `executable+writable+readable` memory arean using platform-dependent function(For windows, that's `VirtualAlloc/Free`), and fullfil machine code into it. Then it forcely covnerts this memory arean as a function `fun`, passes three arguments and calls it. We will get the following output:
![](docs/demo.png)

# Details
Intel architecture instructions format is as follows:
![](docs/instr_fmt1.png)
