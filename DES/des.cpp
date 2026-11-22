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

} // namespace des