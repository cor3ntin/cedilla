#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

// clang-format off

/*
using namespace unicode;

template <typename I> std::string n2hexstr(I w, size_t hex_len = sizeof(I)<<1) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len,'0');
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        rc[i] = digits[(w>>j) & 0x0f];
    return rc;
}

inline std::ostream& operator << ( std::ostream& os, std::u32string const& value ) {
    for(auto codepoint : value) {
        os << std::hex << "U+" << n2hexstr(codepoint, 8) << " ";
    }
}



TEST_CASE("CJK") {
    SECTION(R"__( (鼻; 鼻; 鼻; 鼻; 鼻; ) CJK COMPATIBILITY IDEOGRAPH-2FA1C)__") {
        auto c1 = std::u32string{U"\U0002FA1C"};
        auto c2 = std::u32string{U"\U00009F3B"};
        auto c3 = std::u32string{U"\U00009F3B"};
        auto d1 = decomposed(c1);
        auto d2 = decomposed(c2);
        auto d3 = decomposed(c3);
        sort(std::begin(c1), std::end(c1));
        sort(std::begin(c2), std::end(c2));
        sort(std::begin(c3), std::end(c3));
        sort(std::begin(d1), std::end(d1));
        sort(std::begin(d2), std::end(d2));
        sort(std::begin(d3), std::end(d3));
        CHECK(c3 == d1);
        CHECK(c3 == d2);
        CHECK(c3 == d3);
    }
}*/
