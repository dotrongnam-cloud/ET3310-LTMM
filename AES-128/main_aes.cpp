#include "aes.h"
#include <fstream>
#include <iostream>
#include <string>

void runAesTestCase(const std::string& title,
                    const aes::Block& plaintext,
                    const aes::Block& key,
                    std::ostream& out) {
    out << "=============================================================\n";
    out << "TEST CASE: " << title << "\n";
    out << "=============================================================\n";
    out << "Plaintext : "; aes::printBlockHex(out, plaintext); out << "\n";
    out << "Key       : "; aes::printBlockHex(out, key);       out << "\n";

    out << "\n[1] KEY EXPANSION\n";
    auto round_keys = aes::keyExpansion(key, &out);

    out << "\n[2] ENCRYPTION\n";
    aes::Block ciphertext = aes::encryptBlock(plaintext, round_keys, &out);

    out << "\n[3] DECRYPTION VERIFICATION\n";
    aes::Block decrypted = aes::decryptBlock(ciphertext, round_keys, &out);

    bool ok = (decrypted == plaintext);
    out << "\n[SUMMARY]\n";
    out << "Plaintext  : "; aes::printBlockHex(out, plaintext);  out << "\n";
    out << "Key        : "; aes::printBlockHex(out, key);        out << "\n";
    out << "Ciphertext : "; aes::printBlockHex(out, ciphertext); out << "\n";
    out << "Decrypted  : "; aes::printBlockHex(out, decrypted);  out << "\n";
    out << "Check      : " << (ok ? "PASS (Khop Plaintext)" : "FAIL") << "\n\n";

    std::cout << "[" << title << "] Ciphertext = ";
    aes::printBlockHex(std::cout, ciphertext);
    std::cout << " | Decrypt = " << (ok ? "PASS" : "FAIL") << "\n";
}

int main() {
    std::ofstream log_file("outputs/aes_output.txt");
    if (!log_file.is_open()) {
        std::cerr << "Khong mo duoc file outputs/aes_output.txt (hay chay lenh tu thu muc goc ET3310)\n";
        return 1;
    }

    // Test case 1: Vi du tren bang trong vo
    aes::Block pt_class = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11
    };
    aes::Block key_class = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    runAesTestCase("Vi du AES-128 tren bang", pt_class, key_class, log_file);

    // Test case 2: Vi du trong slide bai giang
    aes::Block pt_slide = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    aes::Block key_slide = {
        0x0F, 0x15, 0x71, 0xC9, 0x47, 0xD9, 0xE8, 0x59,
        0x0C, 0xB7, 0xAD, 0xD6, 0xAF, 0x7F, 0x67, 0x98
    };
    runAesTestCase("Vi du AES-128 trong slide", pt_slide, key_slide, log_file);

    std::cout << "Da ghi day du trang thai kiem dinh vao outputs/aes_output.txt\n";
    return 0;
}