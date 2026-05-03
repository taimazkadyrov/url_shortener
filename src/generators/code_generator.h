#pragma once
#include <string>

class CodeGenerator {
public:
    static std::string generate(int length = 6);
    static bool is_valid(const std::string& code);

private:
    static const std::string ALPHABET;
};