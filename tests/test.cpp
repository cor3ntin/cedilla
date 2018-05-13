#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "common.h"
#include <cedilla/normalization.hpp>


TEST_CASE("Normalization tests Cedilla") {

    for(auto&& test : test_cases()) {
        // NFD
        CHECK(test.c3 == std::u32string{normalized(test.c1, cedilla::normalization_form::nfd)});
        CHECK(std::u32string{test.c3} ==
              std::u32string{normalized(test.c2, cedilla::normalization_form::nfd)});
        CHECK(std::u32string{test.c3} ==
              std::u32string{normalized(test.c3, cedilla::normalization_form::nfd)});

        CHECK(std::u32string{test.c5} ==
              std::u32string{normalized(test.c5, cedilla::normalization_form::nfd)});
        CHECK(std::u32string{test.c5} ==
              std::u32string{normalized(test.c4, cedilla::normalization_form::nfd)});

        // NFKD
        CHECK(std::u32string{test.c5} ==
              std::u32string{normalized(test.c1, cedilla::normalization_form::nfkd)});
        CHECK(std::u32string{test.c5} ==
              std::u32string{normalized(test.c2, cedilla::normalization_form::nfkd)});
        CHECK(std::u32string{test.c5} ==
              std::u32string{normalized(test.c3, cedilla::normalization_form::nfkd)});
        CHECK(std::u32string{test.c5} ==
              std::u32string{normalized(test.c4, cedilla::normalization_form::nfkd)});
        CHECK(std::u32string{test.c5} ==
              std::u32string{normalized(test.c5, cedilla::normalization_form::nfkd)});
        // NFC
        CHECK(std::u32string{test.c2} ==
              std::u32string{normalized(test.c1, cedilla::normalization_form::nfc)});
        CHECK(std::u32string{test.c2} ==
              std::u32string{normalized(test.c2, cedilla::normalization_form::nfc)});
        CHECK(std::u32string{test.c2} ==
              std::u32string{normalized(test.c3, cedilla::normalization_form::nfc)});
        CHECK(std::u32string{test.c4} ==
              std::u32string{normalized(test.c4, cedilla::normalization_form::nfc)});
        CHECK(std::u32string{test.c4} ==
              std::u32string{normalized(test.c5, cedilla::normalization_form::nfc)});

        // NFKC
        CHECK(std::u32string{test.c4} ==
              std::u32string{normalized(test.c1, cedilla::normalization_form::nfkc)});
        CHECK(std::u32string{test.c4} ==
              std::u32string{normalized(test.c2, cedilla::normalization_form::nfkc)});
        CHECK(std::u32string{test.c4} ==
              std::u32string{normalized(test.c3, cedilla::normalization_form::nfkc)});
        CHECK(std::u32string{test.c4} ==
              std::u32string{normalized(test.c4, cedilla::normalization_form::nfkc)});
        CHECK(std::u32string{test.c4} ==
              std::u32string{normalized(test.c5, cedilla::normalization_form::nfkc)});
    }
}
