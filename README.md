# ET3310 - Lý thuyết mật mã

- **Họ và tên:** Đỗ Trọng Nam
- **MSSV:** 20233541

## Mô tả bài tập
Cài đặt thuật toán mã hóa và giải mã khối đối xứng **DES** và **AES-128** bằng C++17 (không sử dụng thư viện mật mã có sẵn), có xuất đầy đủ giá trị kiểm định trung gian cho từng trạng thái và từng vòng.

## Cấu trúc thư mục
```text
ET3310-LTMM/
├── DES/
│   ├── des_tables.h      # Các bảng hoán vị chuẩn FIPS 46-3 (IP, IP^-1, E, P, PC1, PC2, S1..S8)
│   ├── des.h / des.cpp   # Cài đặt sinh khóa 16 vòng, hàm F, mã hóa và giải mã DES
│   └── main_des.cpp      # Chạy các test vector DES và xuất log kiểm định
├── AES-128/
│   ├── aes_tables.h      # Bảng S-box, Inv S-box và hằng số vòng Rcon
│   ├── aes.h / aes.cpp   # Phép toán trên GF(2^8), KeyExpansion, SubBytes, ShiftRows, MixColumns, AddRoundKey
│   └── main_aes.cpp      # Chạy các test vector AES-128 và xuất log kiểm định
└── outputs/
    ├── des_output.txt    # Kết quả chạy và trạng thái từng vòng của DES
    └── aes_output.txt    # Kết quả chạy và ma trận trạng thái từng bước của AES-128