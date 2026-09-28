#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "des.hpp"

inline constexpr des::Cipher session_cipher{0xAABB09182736CCDDULL};

inline bool send_all(int fd, std::span<const des::u8> data) {
    while (!data.empty()) {
        const ssize_t n = send(fd, data.data(), data.size(), 0);
        if (n <= 0) return false;
        data = data.subspan(static_cast<std::size_t>(n));
    }
    return true;
}

inline bool recv_all(int fd, std::span<des::u8> out) {
    while (!out.empty()) {
        const ssize_t n = recv(fd, out.data(), out.size(), 0);
        if (n <= 0) return false;
        out = out.subspan(static_cast<std::size_t>(n));
    }
    return true;
}

inline std::string hex_blocks(std::span<const des::u8> bytes) {
    std::string out;
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        if (i != 0 && i % des::block_size == 0) out += ' ';
        out += std::format("{:02X}", bytes[i]);
    }
    return out;
}

inline bool send_encrypted(int fd, std::string_view msg) {
    const std::vector<des::u8> cipher = session_cipher.encrypt(msg);
    std::cout << std::format("  [sent ciphertext: {}]\n", hex_blocks(cipher));

    const std::uint32_t count = htonl(static_cast<std::uint32_t>(cipher.size() / des::block_size));
    std::vector<des::u8> wire(sizeof count + cipher.size());
    std::memcpy(wire.data(), &count, sizeof count);
    std::ranges::copy(cipher, wire.begin() + sizeof count);

    return send_all(fd, wire);
}

inline std::optional<std::string> recv_encrypted(int fd) {
    std::array<des::u8, 4> header{};
    if (!recv_all(fd, header)) return std::nullopt;

    std::uint32_t count_be = 0;
    std::memcpy(&count_be, header.data(), sizeof count_be);
    const std::uint32_t count = ntohl(count_be);

    if (count == 0 || count > 4096) return std::nullopt;

    std::vector<des::u8> cipher(static_cast<std::size_t>(count) * des::block_size);
    if (!recv_all(fd, cipher)) return std::nullopt;

    std::cout << std::format("  [received ciphertext: {}]\n", hex_blocks(cipher));
    return session_cipher.decrypt(cipher);
}