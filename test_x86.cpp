#include "instr_x86.hpp"
#include "morejit_x86.hpp"
#include "type_x86.hpp"

void greeting() {
    // Allocate 500 bytes to store native code
    x86jitcode c{500};
    c.write_text("greeting %x%x!");
    auto p = &printf;
    // Fullfil native code
    __jitcode {
        c.push(ebp);
        c.mov(ebp, esp);
        c.push(addr(ebp, 16));
        c.push(addr(ebp, 12));
        c.push((int)c.get_text_ptr());
        c.call(addr(ebp,8));
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

int main() {
    greeting();
    getchar();
    return 0;
}
