#include <ucd/decomposition.h>

#define CATCH_CONFIG_FAST_COMPILE
#define CATCH_CONFIG_DISABLE_MATCHERS



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
    return os;
}

#include <catch2/catch.hpp>
#include <string>
#include <iostream>

struct test_data {
    std::u32string c1, c2, c3, c4, c5;
};

static const test_data data[] = {
    {{#tests}}
    { U"{{c1}}", U"{{c2}}", U"{{c3}}", U"{{c4}}", U"{{c5}}" },
    {{/tests}}
    {}
};


TEST_CASE( "Normalization tests {{idx}}") {

    for(auto && test : data) {
        //NFD
        CHECK(test.c3 == normalized(test.c1, NormalizationForm::NFD));
        CHECK(std::u32string{test.c3} == normalized(test.c2, NormalizationForm::NFD));
        CHECK(std::u32string{test.c3} == normalized(test.c3, NormalizationForm::NFD));

        CHECK(std::u32string{test.c5} == normalized(test.c5, NormalizationForm::NFD));
        CHECK(std::u32string{test.c5} == normalized(test.c4, NormalizationForm::NFD));

        //NFKD
        CHECK(std::u32string{test.c5} == normalized(test.c1, NormalizationForm::NFKD));
        CHECK(std::u32string{test.c5} == normalized(test.c2, NormalizationForm::NFKD));
        CHECK(std::u32string{test.c5} == normalized(test.c3, NormalizationForm::NFKD));
        CHECK(std::u32string{test.c5} == normalized(test.c4, NormalizationForm::NFKD));
        CHECK(std::u32string{test.c5} == normalized(test.c5, NormalizationForm::NFKD));

        //NFC
        CHECK(std::u32string{test.c2} == normalized(test.c1, NormalizationForm::NFC));
        CHECK(std::u32string{test.c2} == normalized(test.c2, NormalizationForm::NFC));
        CHECK(std::u32string{test.c2} == normalized(test.c3, NormalizationForm::NFC));
        CHECK(std::u32string{test.c4} == normalized(test.c4, NormalizationForm::NFC));
        CHECK(std::u32string{test.c4} == normalized(test.c5, NormalizationForm::NFC));

        //NFKC
        CHECK(std::u32string{test.c4} == normalized(test.c1, NormalizationForm::NFKC));
        CHECK(std::u32string{test.c4} == normalized(test.c2, NormalizationForm::NFKC));
        CHECK(std::u32string{test.c4} == normalized(test.c3, NormalizationForm::NFKC));
        CHECK(std::u32string{test.c4} == normalized(test.c4, NormalizationForm::NFKC));
        CHECK(std::u32string{test.c4} == normalized(test.c5, NormalizationForm::NFKC));
    }
}
