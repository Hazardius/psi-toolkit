#include <iomanip>
#include <iostream>

#include "utf8_string.hpp"


UTF8String::UTF8String(const std::string& str) : std::string(str) {
    int i = 0;
    while (i < str.length()) {
        int symLen = utf8SymbolLength_(str[i]);
        s_.push_back(str.substr(i,symLen));
        i += symLen;
    }
}

std::string UTF8String::substr(size_t pos, size_t n) const {
    std::string result = "";
    for (int i = pos; i < pos + n; ++i) {
        result += s_[i];
    }
    return result;
}

int UTF8String::utf8SymbolLength_(unsigned char firstByte) {
    if ((firstByte & 0200) == 0000) return 1;
    if ((firstByte & 0340) == 0300) return 2;
    if ((firstByte & 0360) == 0340) return 3;
    if ((firstByte & 0370) == 0360) return 4;
    if ((firstByte & 0374) == 0370) return 5;
    if ((firstByte & 0376) == 0374) return 6;
}