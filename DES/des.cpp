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
uint32_t feistelF(uint32_t R, uint64_t subkey, std::ostream* log) {
    // 1. Mở rộng R từ 32-bit lên 48-bit qua bảng E
    uint64_t expanded = permute(R, E, 32, 48);

    // 2. XOR với khóa vòng 48-bit
    uint64_t xored = expanded ^ subkey;

    // 3. Thay thế qua 8 hộp S-box (mỗi cụm 6-bit -> 4-bit)
    uint32_t sbox_out = 0;
    for (int box = 0; box < 8; ++box) {
        uint8_t six_bits = static_cast<uint8_t>((xored >> (42 - 6 * box)) & 0x3FULL);
        int row = ((six_bits & 0x20) >> 4) | (six_bits & 0x01);
        int col = (six_bits >> 1) & 0x0F;
        sbox_out = (sbox_out << 4) | SBOX[box][row][col];
    }

    // 4. Hoán vị 32-bit qua bảng P
    uint32_t p_out = static_cast<uint32_t>(permute(sbox_out, P, 32, 32));

    if (log != nullptr) {
        *log << "   E(R)      : "; printHex(*log, expanded, 12); *log << "\n";
        *log << "   E(R) ^ K  : "; printHex(*log, xored, 12);    *log << "\n";
        *log << "   S-box out : "; printHex(*log, sbox_out, 8);  *log << "\n";
        *log << "   P(S-box)  : "; printHex(*log, p_out, 8);     *log << "\n";
    }

    return p_out;
}

uint64_t processBlock(uint64_t block,
                      const std::array<uint64_t, 16>& subkeys,
                      bool decrypt,
                      std::ostream* log) {
    // 1. Hoán vị khởi đầu IP
    uint64_t ip = permute(block, IP, 64, 64);

    // 2. Tách thành 2 nửa 32-bit L0 và R0
    uint32_t L = static_cast<uint32_t>(ip >> 32);
    uint32_t R = static_cast<uint32_t>(ip & 0xFFFFFFFFULL);

    if (log != nullptr) {
        *log << "Input (64-bit): "; printHex(*log, block, 16); *log << "\n";
        *log << "IP(Input)     : "; printHex(*log, ip, 16);    *log << "\n";
        *log << "L0            : "; printHex(*log, L, 8);
        *log << " | R0 : ";         printHex(*log, R, 8);      *log << "\n";
        *log << "-------------------------------------------------------------\n";
    }

    // 3. Lặp 16 vòng Feistel (giải mã dùng khóa vòng ngược từ K16 về K1)
    for (int r = 0; r < 16; ++r) {
        int key_idx = decrypt ? (15 - r) : r;
        uint64_t round_key = subkeys[key_idx];

        if (log != nullptr) {
            *log << "Round " << std::setw(2) << (r + 1)
                 << " (using K" << std::setw(2) << (key_idx + 1) << " = ";
            printHex(*log, round_key, 12);
            *log << ")\n";
        }

        uint32_t prev_L = L;
        uint32_t f_val  = feistelF(R, round_key, log);
        L = R;
        R = prev_L ^ f_val;

        if (log != nullptr) {
            *log << "   L" << std::setw(2) << (r + 1) << "       : "; printHex(*log, L, 8);
            *log << " | R" << std::setw(2) << (r + 1) << " : ";      printHex(*log, R, 8);
            *log << "\n";
        }
    }

    // 4. 32-bit Swap (ghép R16 đứng trước L16) rồi qua IP_INV
    uint64_t pre_output = (static_cast<uint64_t>(R) << 32) | L;
    uint64_t output = permute(pre_output, IP_INV, 64, 64);

    if (log != nullptr) {
        *log << "-------------------------------------------------------------\n";
        *log << "32-bit Swap (R16||L16): "; printHex(*log, pre_output, 16); *log << "\n";
        *log << "Output (IP^-1)        : "; printHex(*log, output, 16);     *log << "\n";
    }

    return output;
}

// =============================================================================



} // namespace des