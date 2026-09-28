#include <iostream>
#include <format>
#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include <include/constant.hpp>

constexpr u64 get_bit(u64 val, int width, int pos) {
    return (val >> (width - pos)) & 1ULL;
}

u64 permute(u64 input, const int* table, int table_size, int input_width) {
    u64 output = 0;
    for (int i = 0; i < table_size; ++i) {
        output = (output << 1) | get_bit(input, input_width, table[i]);
    }
    return output;
}

constexpr u32 shift_left28(u32 val, int shifts) {
    val &= 0x0FFFFFFFu;
    return ((val << shifts) | (val >> (28 - shifts))) & 0x0FFFFFFFu;
}

u64 encrypt(u64 pt, const std::array<u64, 16>& rk) {
    pt = permute(pt, initial_perm, 64, 64);
    std::cout << std::format("After initial permutation {:016X}\n", pt);

    u32 left = static_cast<u32>(pt >> 32);
    u32 right = static_cast<u32>(pt & 0xFFFFFFFFu);

    for (int i = 0; i < 16; ++i) {
        u64 right_expanded = permute(right, exp_d, 48, 32);

        u64 xor_x = right_expanded ^ rk[i];

        u32 sbox_out = 0;
        for (int j = 0; j < 8; ++j) {
            u32 chunk = static_cast<u32>((xor_x >> (42 - 6 * j)) & 0x3F);
            u32 row = ((chunk >> 5) & 1) << 1 | (chunk & 1);
            u32 col = (chunk >> 1) & 0xF;
            u32 val = static_cast<u32>(sbox[j][row][col]);
            sbox_out = (sbox_out << 4) | val;
        }

        u32 sbox_perm = static_cast<u32>(permute(sbox_out, per, 32, 32));

        u32 result = left ^ sbox_perm;
        left = result;

        if (i != 15) std::swap(left, right);

        std::cout << std::format("Round {:2} {:08X} {:08X} {:012X}\n", i + 1, left, right, rk[i]);
    }

    u64 combine = (static_cast<u64>(left) << 32) | right;
    return permute(combine, final_perm, 64, 64);
}

std::string pkcs7_padding(const std::string& input) {
    size_t block_size = 8;
    size_t padding_len = block_size - (input.length() % block_size);
    
    std::string padded = input;
    padded.append(padding_len, static_cast<char>(padding_len));
    return padded;
}

u64 bytes_to_u64(const char* bytes) {
    u64 result = 0;
    for (int i = 0; i < 8; ++i) {
        result = (result << 8) | (static_cast<u8>(bytes[i]));
    }
    return result;
}

std::string u64_to_bytes(u64 value) {
    std::string bytes(8, ' ');
    for (int i = 7; i >= 0; --i) {
        bytes[i] = static_cast<char>(value & 0xFF);
        value >>= 8;
    }
    return bytes;
}

std::vector<u64> des_encrypt(u64 key56, const std::string& plain_text) {
    u32 left = static_cast<u32>((key56 >> 28) & 0x0FFFFFFFu);
    u32 right = static_cast<u32>(key56 & 0x0FFFFFFFu);
    
    std::array<u64, 16> rk{};
    for (int i = 0; i < 16; ++i) {
        left = shift_left28(left, shift_table[i]);
        right = shift_left28(right, shift_table[i]);
        
        u64 combined56 = (static_cast<u64>(left) << 28) | right;
        rk[i] = permute(combined56, key_comp, 48, 56);
    }

    std::string padded_text = pkcs7_padding(plain_text);
    std::vector<u64> encrypted_blocks;

    for (size_t i = 0; i < padded_text.length(); i += 8) {
        const char* block_bytes = padded_text.data() + i;
        
        u64 block_64bit = bytes_to_u64(block_bytes);

        u64 cipher_block = encrypt(block_64bit, rk);
        encrypted_blocks.push_back(cipher_block);
    }

    return encrypted_blocks;
}

std::string des_decrypt(u64 key56, std::vector<u64> cipher_text) {
    u32 left = static_cast<u32>((key56 >> 28) & 0x0FFFFFFFu);
    u32 right = static_cast<u32>(key56 & 0x0FFFFFFFu);
    
    std::array<u64, 16> rk_rev{};
    for (int i = 0; i < 16; ++i) {
        left = shift_left28(left, shift_table[i]);
        right = shift_left28(right, shift_table[i]);
    
        u64 combined56 = (static_cast<u64>(left) << 28) | right;
        rk_rev[15 - i] = permute(combined56, key_comp, 48, 56);
    }

    std::string decrypted_blocks;

    for (size_t i = 0; i < cipher_text.size(); ++i) {
        u64 text_block = encrypt(cipher_text[i], rk_rev);
        decrypted_blocks.append(u64_to_bytes(text_block));
    }

    return decrypted_blocks;
}

int main() {
    u64 pt = 0x48656C6C6F576F72ULL;
    u64 key = 0xAABB09182736CCDDULL;

    u64 key56 = permute(key, keyp, 56, 64);

    std::string plain_text = "halo cak, piye kabare?";

    std::cout << "Plaintext\n";
    std::cout << plain_text << std::endl;
    
    std::vector<u64> cipher_text = des_encrypt(key56, plain_text);
    std::cout << "Encryption\n";
    for (auto block: cipher_text) {
        std::cout << std::format("Cipher Text : {:016X}\n", block);
    }

    std::string decrypted_plain_text = des_decrypt(key56, cipher_text);
    std::cout << "Decryption\n";
    std::cout << decrypted_plain_text << std::endl;

    return 0;
}