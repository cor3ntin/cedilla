#include <ucd/decomposition.h>

#define CATCH_CONFIG_FAST_COMPILE
#define CATCH_CONFIG_DISABLE_MATCHERS


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


using namespace unicode;

TEST_CASE( "Normalization tests {{idx}}" ) {
{{#tests}}
    SECTION( R"__({{comment}})__" ) {
        //NFD
        CHECK(std::u32string{U"{{c3}}"} == normalized(U"{{c1}}", NormalizationForm::NFD));
        CHECK(std::u32string{U"{{c3}}"} == normalized(U"{{c2}}", NormalizationForm::NFD));
        CHECK(std::u32string{U"{{c3}}"} == normalized(U"{{c3}}", NormalizationForm::NFD));

        CHECK(std::u32string{U"{{c5}}"} == normalized(U"{{c5}}", NormalizationForm::NFD));
        CHECK(std::u32string{U"{{c5}}"} == normalized(U"{{c4}}", NormalizationForm::NFD));

        //NFKD
        CHECK(std::u32string{U"{{c5}}"} == normalized(U"{{c1}}", NormalizationForm::NFKD));
        CHECK(std::u32string{U"{{c5}}"} == normalized(U"{{c2}}", NormalizationForm::NFKD));
        CHECK(std::u32string{U"{{c5}}"} == normalized(U"{{c3}}", NormalizationForm::NFKD));
        CHECK(std::u32string{U"{{c5}}"} == normalized(U"{{c4}}", NormalizationForm::NFKD));
        CHECK(std::u32string{U"{{c5}}"} == normalized(U"{{c5}}", NormalizationForm::NFKD));
    }
{{/tests}}
}
