# morejit
morejit can facilitate generating architecture-dependent machine code at runtime. In general, it can be used to build a **JIT** compiler, you can also use it to write some shellcode at runtime to ensure the robustness of your system or do anything which requires dynamical creation behaviors.

# Overview
This demo shows how to use **morejit**, it will generate machine code for Intel x86 architecture and call it immediately:
```cpp
#include "instr_x86.hpp"
#include "morejit_x86.hpp"
#include "type_x86.hpp"

int main() {
    // Allocate 500 bytes to store native code
    x86jitcode c{500};
    c.write_text("greeting %x%x!");

    // Fullfil native code
    __jitcode {
        c.push(ebp);
        c.mov(ebp, esp);
        c.push(addr(ebp, 16));
        c.push(addr(ebp, 12));
        c.push((int)c.get_text_ptr());
        c.call(addr(ebp, 8));
        c.add(esp, 12);
        c.pop(ebp);
        c.ret();
        c.dump();
    }

    // Perform function
    auto func =
        c.as_function<void (*)(int (*)(const char* const, ...), int, int)>();
    func(&printf, 0xcafe, 0xbabe);
}
```
Simplely put, this codelet allocates an `executable+writable+readable` memory arean using platform-dependent function(For windows, that's `VirtualAlloc/Free`), and fullfil machine code into it. Note that `_i32/_i16/_i8` are user defined literal suffixs, they are just analogous to `imm32(val)` `imm16(val)` and `imm8(val)`. After that, it forcely covnerts the pointer points to this memory arean as a function `fun`, passes three arguments and calls it. We will get the following output:
![](docs/demo.png)

# Details
Intel architecture instructions formats are as follows:
![](docs/instr_fmt1.png)
