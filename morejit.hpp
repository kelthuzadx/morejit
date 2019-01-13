#ifndef _MOREJIT_H
#define _MOREJIT_H
#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <type_traits>
#include <unordered_map>
#define __jitcode

class jitcode {
public:
    explicit jitcode(size_t alloc_size);
    ~jitcode();

public:
    int get_alloc_size() const;

    int get_code_size() const;

    int get_text_size() const;

    char* get_text_ptr(int offset = 0);

    void dump();

    template <typename FuncPtrType>
    inline FuncPtrType as_function() {
        static_assert(std::is_pointer<FuncPtrType>::value,
                      "expects function pointer type");
        return (FuncPtrType)(alloc_start + text_size + 1);
    }

    void write_text(const char* str);

    void emit_u8(std::uint8_t byte) { *cur_code++ = byte; }

    void label(const char* label_name) { labels.emplace(label_name, cur_code);}

    const char* get_label(const char* name) { return labels.find(name)->second; }

protected:
    std::unordered_map<std::string, char*> labels;
    const int alloc_size;
    const char* alloc_start;
    int text_size;
    char* cur_code;
};

#endif
