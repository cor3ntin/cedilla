#define CATCH_CONFIG_MAIN
#include <ucd/decomposition.h>

using namespace unicode;

template<typename I>
std::string n2hexstr(I w, size_t hex_len = sizeof(I) << 1) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len, '0');
    for(size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
        rc[i] = digits[(w >> j) & 0x0f];
    return rc;
}

inline std::ostream& operator<<(std::ostream& os, std::u32string const& value) {
    for(auto codepoint : value) {
        os << std::hex << "U+" << n2hexstr(codepoint, 8) << " ";
    }
    return os;
}

#include <catch2/catch.hpp>
#include <string>
#include <iostream>

struct test_data {
    std::u32string c1, c2, c3, c4, c5;
};

static const test_data data[] = {
    {U"\U00000CCA\U00000334\U00000CD5", U"\U00000CCA\U00000334\U00000CD5",
     U"\U00000CC6\U00000CC2\U00000334\U00000CD5", U"\U00000CCA\U00000334\U00000CD5",
     U"\U00000CC6\U00000CC2\U00000334\U00000CD5"},
};


TEST_CASE("Normalization test debug") {

    for(auto&& test : data) {
        CHECK(std::u32string{U"\U00000CC6\U00000CC2\U00000334\U00000CD5"} ==
              normalized(U"\U00000CCA\U00000334\U00000CD5", NormalizationForm::NFKD));
    }
}
