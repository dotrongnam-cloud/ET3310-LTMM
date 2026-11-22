#include "des.h"
#include <fstream>
#include <iostream>
#include <string>

void runTestCase(const std::string& title,
                 uint64_t plaintext,
                 uint64_t key,
                 std::ostream& out) {
    out << "=============================================================\n";
    out << "TEST CASE: " << title << "\n";
    out << "=============================================================\n";

    out << "\n[1] KEY SCHEDULE\n";
    auto subkeys = des::generateSubkeys(key, &out);

    out << "\n[2] ENCRYPTION\n";
    uint64_t ciphertext = des::processBlock(plaintext, subkeys, false, &out);

    out << "\n[3] DECRYPTION VERIFICATION\n";
    uint64_t decrypted = des::processBlock(ciphertext, subkeys, true, &out);

    out << "\n[SUMMARY]\n";
    out << "Plaintext  : "; des::printHex(out, plaintext, 16);  out << "\n";
    out << "Key        : "; des::printHex(out, key, 16);        out << "\n";
    out << "Ciphertext : "; des::printHex(out, ciphertext, 16); out << "\n";
    out << "Decrypted  : "; des::printHex(out, decrypted, 16);  out << "\n";
    out << "Check      : " << (decrypted == plaintext ? "PASS (Khớp Plaintext)" : "FAIL") << "\n\n";

    // In tóm tắt ngắn gọn ra màn hình console
    std::cout << "[" << title << "] Ciphertext = ";
    des::printHex(std::cout, ciphertext, 16);
    std::cout << " | Decrypt = " << (decrypted == plaintext ? "PASS" : "FAIL") << "\n";
}

int main() {
    std::ofstream log_file("outputs/des_output.txt");
    if (!log_file.is_open()) {
        std::cerr << "Khong mo duoc file outputs/des_output.txt (hay chay lenh tu thu muc goc ET3310)\n";
        return 1;
    }

    // Test case 1: Bộ kiểm định tiêu chuẩn FIPS
    runTestCase("FIPS Standard Vector",
                0x0123456789ABCDEFULL,
                0x133457799BBCDFF1ULL,
                log_file);

    // Test case 2: Bộ kiểm định giáo trình Stallings (nguồn slide)
    runTestCase("Stallings Textbook Vector",
                0x02468ACEECA86420ULL,
                0x0F1571C947D9E859ULL,
                log_file);

   

    std::cout << "Da ghi day du trang thai kiem dinh vao outputs/des_output.txt\n";
    return 0;
}