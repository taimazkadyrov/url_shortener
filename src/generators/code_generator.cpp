#include "code_generator.h"
#include <random>
#include <stdexcept>
#include <algorithm>

const std::string CodeGenerator::ALPHABET =
    "abcdefghjkmnpqrstuvwxyz"
    "ABCDEFGHJKMNPQRSTUVWXYZ"
    "23456789";

std::string CodeGenerator::generate(int length) {
    if (length < 4 || length > 32) {
        throw std::invalid_argument("длина кода должна быть от 4 до 32 символов");
    }
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, ALPHABET.size() - 1);
    std::string code;
    code.reserve(length);
    for (int i = 0; i < length; i++) {
        code += ALPHABET[dist(rng)];
    }
    return code;
}

bool CodeGenerator::is_valid(const std::string& code) {
    if (code.empty() || code.size() > 32) {
        return false;
    }
    return std::all_of(code.begin(), code.end(), [](char c) {
        return std::isalnum(c);
    });
}