#include "des.h"
#include "des_tables.h"
#include <iomanip>

namespace des {

uint64_t permute(uint64_t input, const int* table, int in_len, int out_len) {
    uint64_t result = 0;
    for (int i = 0; i < out_len; ++i) {
        int shift = in_len - table[i];
        uint64_t bit = (input >> shift) & 1ULL;
        result = (result << 1) | bit;
    }
    return result;
}

void printHex(std::ostream& os, uint64_t value, int hex_digits) {
    os << std::uppercase << std::hex
       << std::setw(hex_digits) << std::setfill('0') << value
       << std::dec << std::setfill(' ');
}
uint32_t leftRotate28(uint32_t value, int shift) {
    // Dịch vòng trái trong phạm vi 28 bit và chặn mặt nạ 0x0FFFFFFF để xóa 4 bit thừa ở đầu
    return ((value << shift) | (value >> (28 - shift))) & 0x0FFFFFFFU;
}

std::array<uint64_t, 16> generateSubkeys(uint64_t key, std::ostream* log) {
    std::array<uint64_t, 16> subkeys{};

    // Bước 1: Qua bảng PC1 để rút gọn khóa từ 64-bit xuống 56-bit (bỏ 8 bit parity)
    uint64_t k56 = permute(key, PC1, 64, 56);

    // Bước 2: Tách thành 2 nửa 28-bit C0 và D0
    uint32_t C = static_cast<uint32_t>((k56 >> 28) & 0x0FFFFFFFULL);
    uint32_t D = static_cast<uint32_t>(k56 & 0x0FFFFFFFULL);

    if (log != nullptr) {
        *log << "Key (64-bit) : "; printHex(*log, key, 16); *log << "\n";
        *log << "PC1 (56-bit) : "; printHex(*log, k56, 14); *log << "\n";
        *log << "C0           : "; printHex(*log, C, 7);
        *log << " | D0 : ";        printHex(*log, D, 7);  *log << "\n";
        *log << "-------------------------------------------------------------\n";
    }

    // Bước 3: Lặp 16 vòng dịch trái và nén qua PC2 để sinh K1..K16
    for (int i = 0; i < 16; ++i) {
        C = leftRotate28(C, SHIFTS[i]);
        D = leftRotate28(D, SHIFTS[i]);

        uint64_t cd = (static_cast<uint64_t>(C) << 28) | D;
        subkeys[i] = permute(cd, PC2, 56, 48);

        if (log != nullptr) {
            *log << "Round " << std::setw(2) << (i + 1) << " | C: ";
            printHex(*log, C, 7);
            *log << " | D: ";
            printHex(*log, D, 7);
            *log << " | K" << std::setw(2) << (i + 1) << ": ";
            printHex(*log, subkeys[i], 12);
            *log << "\n";
        }
    }

    return subkeys;
}

} // namespace des