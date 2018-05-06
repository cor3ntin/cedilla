#pragma once
#include <cstdint>
#include <range/v3/view.hpp>

namespace unicode::details {

enum class hangul_syllable_type : uint8_t {
    invalid = 0x00,
    leading_jamo = 0x01,
    vowel_jamo = 0x02,
    trailing_jamo = 0x04,
    lv_syllable = 0x03,
    lvt_syllable = 0x07

};

static const char32_t hangul_lbase = 0x1100;    // First leading Jamo
static const char32_t hangul_lcount = 19;
static const char32_t hangul_vbase = 0x1161;    // First vowel
static const char32_t hangul_vcount = 21;
static const char32_t hangul_tbase = 0x11A7;    // First trailing
static const char32_t hangul_tcount = 28;
static const char32_t hangul_sbase = 0xAC00;    // First composed
static const char32_t hangul_ncount = 588;
static const char32_t hangul_scount = 11172;

struct alignas(4) hangul_syllable {
    char32_t codepoint : 24;
    hangul_syllable_type syllable_type : 8;
};

const ranges::iterator_range<const hangul_syllable*>& hangul_syllable_table();

inline bool operator<(const hangul_syllable& a, const hangul_syllable& b) {
    return a.codepoint < b.codepoint;
}

inline bool operator<(const hangul_syllable& a, const char32_t& b) {
    return a.codepoint < b;
}

inline bool is_decomposable_hangul(char32_t codepoint) {
    return codepoint >= hangul_sbase && codepoint < 0xD800;
}

inline char32_t hangul_syllable_index(char32_t codepoint) {
    return codepoint - hangul_sbase;
}

inline bool is_hangul_lpart(char32_t codepoint) {
    return codepoint >= 0x1100 && codepoint <= 0x1112;
}

inline bool is_hangul_vpart(char32_t codepoint) {
    return codepoint >= 0x1161 && codepoint <= 0x1175;
}

inline bool is_hangul_tpart(char32_t codepoint) {
    return codepoint >= 0x11A7 && codepoint <= 0x11C2;
}

inline bool is_hangul(char32_t codepoint) {
    return codepoint >= 0x1100 && codepoint < 0xD800;
}

inline hangul_syllable_type find_hangul_syllable_type(char32_t codepoint) {
    const auto table = hangul_syllable_table();
    const auto begin = std::begin(table);
    const auto end = std::end(table);
    if(codepoint < begin->codepoint)
        return hangul_syllable_type::invalid;
    auto it = std::lower_bound(begin, end, codepoint);
    if(it == end)
        return hangul_syllable_type::invalid;
    return it->syllable_type;
}

inline hangul_syllable_type find_decomposable_hangul_syllable_type(char32_t codepoint) {
    if(!is_decomposable_hangul(codepoint))
        return hangul_syllable_type::invalid;
    return find_hangul_syllable_type(codepoint);
}


}    // namespace unicode::details
