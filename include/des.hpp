#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "constant.hpp"

namespace des {

inline constexpr std::size_t block_size = 8;
inline constexpr std::size_t rounds     = 16;

using SubKeys = std::array<u64, rounds>;

[[nodiscard]] constexpr u64 load_be64(std::span<const u8, block_size> bytes) noexcept {
    u64 result = 0;
    for (u8 b : bytes) result = (result << 8) | b;
    return result;
}

constexpr void store_be64(u64 value, std::span<u8, block_size> out) noexcept {
    for (std::size_t i = block_size; i-- > 0;) {
        out[i] = static_cast<u8>(value & 0xFF);
        value >>= 8;
    }
}

namespace detail {

inline constexpr u32 mask28 = 0x0FFFFFFFu;

[[nodiscard]] constexpr u64 get_bit(u64 value, int width, int pos) noexcept {
    return (value >> (width - pos)) & 1u;
}

template <std::size_t N>
[[nodiscard]] constexpr u64 permute(u64 input, const std::array<u8, N>& table, int input_width) noexcept {
    u64 output = 0;
    for (u8 pos : table) output = (output << 1) | get_bit(input, input_width, pos);
    return output;
}

[[nodiscard]] constexpr u32 rotl28(u32 value, int shift) noexcept {
    value &= mask28;
    return ((value << shift) | (value >> (28 - shift))) & mask28;
}

[[nodiscard]] constexpr SubKeys make_subkeys(u64 key64) noexcept {
    const u64 key56 = permute(key64, tables::pc1, 64);
    u32 c = static_cast<u32>(key56 >> 28) & mask28;
    u32 d = static_cast<u32>(key56) & mask28;

    SubKeys keys{};
    for (std::size_t i = 0; i < rounds; ++i) {
        c = rotl28(c, tables::key_shifts[i]);
        d = rotl28(d, tables::key_shifts[i]);
        keys[i] = permute((static_cast<u64>(c) << 28) | d, tables::pc2, 56);
    }
    return keys;
}

[[nodiscard]] constexpr u32 feistel(u32 right, u64 subkey) noexcept {
    const u64 mixed = permute(right, tables::expansion, 32) ^ subkey;

    u32 substituted = 0;
    for (std::size_t box = 0; box < 8; ++box) {
        const auto six = static_cast<u32>((mixed >> (42 - 6 * box)) & 0x3F);
        const u32 row = ((six >> 4) & 0b10) | (six & 1);
        const u32 col = (six >> 1) & 0xF;
        substituted = (substituted << 4) | tables::sboxes[box][row][col];
    }
    return static_cast<u32>(permute(substituted, tables::p_perm, 32));
}

[[nodiscard]] constexpr u64 crypt_block(u64 block, const SubKeys& keys) noexcept {
    const u64 permuted = permute(block, tables::ip, 64);
    u32 left  = static_cast<u32>(permuted >> 32);
    u32 right = static_cast<u32>(permuted);

    for (u64 key : keys) {
        const u32 next = left ^ feistel(right, key);
        left  = right;
        right = next;
    }
    return permute((static_cast<u64>(right) << 32) | left, tables::fp, 64);
}

}

[[nodiscard]] inline std::vector<u8> pkcs7_pad(std::string_view input) {
    const std::size_t pad = block_size - input.size() % block_size;
    std::vector<u8> out(input.begin(), input.end());
    out.insert(out.end(), pad, static_cast<u8>(pad));
    return out;
}

[[nodiscard]] inline std::optional<std::string> pkcs7_unpad(std::span<const u8> data) {
    if (data.empty() || data.size() % block_size != 0) return std::nullopt;

    const std::size_t pad = data.back();
    if (pad < 1 || pad > block_size || pad > data.size()) return std::nullopt;
    if (!std::ranges::all_of(data.last(pad), [pad](u8 b) { return b == pad; }))
        return std::nullopt;

    return std::string(data.begin(), data.end() - static_cast<std::ptrdiff_t>(pad));
}

class Cipher {
public:
    explicit constexpr Cipher(u64 key64) noexcept : enc_{detail::make_subkeys(key64)}, dec_{enc_} {
        std::ranges::reverse(dec_);
    }

    [[nodiscard]] constexpr u64 encrypt_block(u64 block) const noexcept {
        return detail::crypt_block(block, enc_);
    }
    [[nodiscard]] constexpr u64 decrypt_block(u64 block) const noexcept {
        return detail::crypt_block(block, dec_);
    }

    [[nodiscard]] std::vector<u8> encrypt(std::string_view plaintext) const {
        std::vector<u8> data = pkcs7_pad(plaintext);
        for (std::size_t i = 0; i < data.size(); i += block_size) {
            const auto block = std::span{data}.subspan(i, block_size).first<block_size>();
            store_be64(encrypt_block(load_be64(block)), block);
        }
        return data;
    }

    [[nodiscard]] std::optional<std::string> decrypt(std::span<const u8> ciphertext) const {
        if (ciphertext.empty() || ciphertext.size() % block_size != 0) return std::nullopt;

        std::vector<u8> data(ciphertext.begin(), ciphertext.end());
        for (std::size_t i = 0; i < data.size(); i += block_size) {
            const auto block = std::span{data}.subspan(i, block_size).first<block_size>();
            store_be64(decrypt_block(load_be64(block)), block);
        }
        return pkcs7_unpad(data);
    }

private:
    SubKeys enc_{};
    SubKeys dec_{};
};

namespace self_test {
inline constexpr Cipher cipher{0x133457799BBCDFF1ULL};
static_assert(cipher.encrypt_block(0x0123456789ABCDEFULL) == 0x85E813540F0AB405ULL, "DES encryption test vector failed");
static_assert(cipher.decrypt_block(0x85E813540F0AB405ULL) == 0x0123456789ABCDEFULL, "DES decryption test vector failed");
}

}