#include "aes.h"
#include <iostream>

int main() {
    // Khóa 1: Ví dụ trên bảng trong vở (16 byte toàn 0x00)
    aes::Block key_class = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // Khóa 2: Ví dụ trong slide bài giảng
    aes::Block key_slide = {
        0x0F, 0x15, 0x71, 0xC9, 0x47, 0xD9, 0xE8, 0x59,
        0x0C, 0xB7, 0xAD, 0xD6, 0xAF, 0x7F, 0x67, 0x98
    };

    std::cout << "=== KEY EXPANSION: VI DU TREN BANG ===\n";
    aes::keyExpansion(key_class, &std::cout);

    std::cout << "\n=== KEY EXPANSION: VI DU TRONG SLIDE ===\n";
    aes::keyExpansion(key_slide, &std::cout);

    return 0;
}