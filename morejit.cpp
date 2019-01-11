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

int jitcode::get_alloc_size() const { return alloc_size; }

int jitcode::get_code_size() const {
    return (alloc_start + text_size) - cur_code;
}

int jitcode::get_text_size() const { return text_size; }

char* jitcode::get_text_ptr(int offset) {
    return (char*)(alloc_start + offset);
}

void jitcode::print() {
    int i = 0;
    while (i != alloc_size) {
        if (alloc_start[i] == '\n') {
            printf("\n");
            i++;
            continue;
        }

        printf("%02x ", (unsigned char)alloc_start[i]);

        i++;
    }
}

void jitcode::write_text(const char* str) {
    strcpy((char*)cur_code, str);
    cur_code[strlen(str)] = '\0';
    text_size = strlen(str);
    cur_code = (char*)alloc_start + strlen(str) + 1;
}
