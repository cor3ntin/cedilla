#define CATCH_CONFIG_MAIN
#include <ucd/decomposition.h>


// clang-format off

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

#include <catch2/catch.hpp>

TEST_CASE("Sort") {
    SECTION( R"__( (Ǆ; Ǆ; Ǆ; DŽ; DZ◌̌; ) LATIN CAPITAL LETTER DZ WITH CARON)__" ) {
            //NFC
             //CHECK(std::u32string{U"\U00000044\U0000017D"} == normalized(U"\U00000044\U0000005A\U0000030C", NormalizationForm::NFKC));
         CHECK(std::u32string{U"\U00000DDD"} == normalized(U"\U00000DD9\U00000DCF\U00000DCA", NormalizationForm::NFC));
}

}
