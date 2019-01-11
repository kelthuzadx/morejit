#include "morejit.hpp"

jitcode::jitcode(size_t alloc_size) : alloc_size(alloc_size), text_size(0) {
    alloc_start = (char*)VirtualAlloc(
        NULL, alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    cur_code = (char*)alloc_start;
}

jitcode::~jitcode() {
    VirtualFree((char*)alloc_start, 0, MEM_RELEASE);
    cur_code = nullptr;
}

imm32 operator"" _i32(unsigned long long val) { return imm32(val); }

imm16 operator"" _i16(unsigned long long val) { return imm16(val); }

imm8 operator"" _i8(unsigned long long val) { return imm8(val); }

int jitcode::get_alloc_size() const { return alloc_size; }

int jitcode::get_code_size() const {
    return cur_code - (alloc_start + text_size);
}

int jitcode::get_text_size() const { return text_size; }

char* jitcode::get_text_ptr(int offset) {
    return (char*)(alloc_start + offset);
}

void jitcode::dump() {
    auto dumping = [=](const char* start, int limit) {
        int i = 0;
        while (i != limit) {
            printf("%02x ", (unsigned char)(start + i));

            i++;
            if (i > 15 && i % 16 == 0) {
                printf("\n");
            }
        }
    };
    printf(".text:\n");
    dumping(alloc_start, text_size);
    printf("\n");
    printf(".jitcode:\n");
    dumping(alloc_start + text_size + 1, get_code_size());
    printf("\n");
}

void jitcode::write_text(const char* str) {
    strcpy((char*)cur_code, str);
    cur_code[strlen(str)] = '\0';
    text_size = strlen(str);
    cur_code = (char*)alloc_start + strlen(str) + 1;
}
