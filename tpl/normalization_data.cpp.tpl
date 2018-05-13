#include <cedilla/detail/normalization_view.hpp>

namespace cedilla::detail {
    static const std::array<decomposition_jumping_table_item, {{block_count}}+1> decomposition_jumping_table_data = {
        {{#blocks}}
        decomposition_jumping_table_item{ {{first}}, {{start}}, {{size}}, {{number_of_replacements}}, {{has_canonical}}},  //{{name}} [{{first}}, {{last}}]
        {{/blocks}}
        decomposition_jumping_table_item{ 0x0F0000, 0, 0, 0} //End sentinel
    };

    static const char32_t decomposition_rules_data[] = {
        {{#blocks}}
        //{{name}} [{{first}}, {{last}}]

        {{#chars}}
        {{cp}}{{#canonical}}|canonical_bitmask{{/canonical}},{{#sub}}{{dec_cp}}{{^final}}|composed_bitmask{{/final}}{{#comma}},{{/comma}}{{/sub}}
        {{/chars}}


        {{/blocks}}
        0
    };

    static const combining_class_item ccc_data[] = {
    {{#combining_classes}}{{#data}} {  {{cp}}, {{ccc}} }, {{/data}}{{/combining_classes}}
        0xFFFF
    };

    static const uint16_t ccc_data_idx[] = {
    {{#combining_classes}}{{#starts}}{{idx}},{{/starts}}{{/combining_classes}}
    (sizeof(ccc_data)/sizeof(uint16_t)) -1
    };


    /*static const combining_class_item combining_classes_data[] = {
    {{#combining_classes}}
    { {{cp}}, {{ccc}} },
    {{/combining_classes}}
    { 0xFFFFFF, 0 }
    };*/


    uint8_t combining_class(char32_t c) {
        const uint16_t truncated = c >> 8;
        const uint8_t  idx   = c & 0xff;
        const uint16_t start = ccc_data_idx[idx];
        const uint16_t end   = ccc_data_idx[idx+1];
        for(auto it = ccc_data+start; BOOST_UNLIKELY(it < ccc_data+end); ++it) {
            if(BOOST_LIKELY(it->cp > truncated))
                return 0;
            if(BOOST_UNLIKELY(it->cp == truncated))
                return it->ccc;
        }
        return 0;

        /*
        auto it = std::lower_bound(ccc_data+start, ccc_data+end, truncated, [] (const combining_class_item  & item, const uint16_t  & value) {
            return item.cp == value;
        });
        if(it == nullptr || it == ccc_data+end || it->cp != truncated)
            return 0;
        return it->ccc;*/
    }

    static const composable_sequence_jumping_table_item  composable_sequence_jumping_table_data[] = {
    {{#composites_c}}
    { {{c}}, {{start}}, {{count}} },
    {{/composites_c}}
    { 0xFFFFF, 0, 0}
    };

    static const composable_sequence recomposition_data[] = {
    {{#composites_l_r}}
    { {{l}}, {{r}}},
    {{/composites_l_r}}
    { 0xFFFFF, 0xFFFFF}
    };

    static const hangul_syllable hangul_data[] = {
    {{#hangul_syllables}}
    { {{cp}}, hangul_syllable_type::{{type}}},
    {{/hangul_syllables}}
    { 0xFFFFF, hangul_syllable_type(0)}
    };

    static const uint16_t nfc_qc_data[] = {
    {{#nfc_qc}}{{#data}}{{cp}},{{/data}}{{/nfc_qc}}
        0xFFFF
    };

    static const uint16_t nfc_qc_idx[] = {
    {{#nfc_qc}}{{#starts}}{{idx}},{{/starts}}{{/nfc_qc}}
    (sizeof(nfc_qc_data)/sizeof(uint16_t)) -1
    };


    static const uint16_t nfkc_qc_data[] = {
    {{#nfkc_qc}}{{#data}}{{cp}},{{/data}}{{/nfkc_qc}}
        0xFFFF
    };

    static const uint16_t nfkc_qc_idx[] = {
    {{#nfkc_qc}}{{#starts}}{{idx}},{{/starts}}{{/nfkc_qc}}
    (sizeof(nfkc_qc_data)/sizeof(uint16_t)) -1
    };


    //
    //

    static const uint16_t nfd_qc_data[] = {
    {{#nfd_qc}}{{#data}}{{cp}},{{/data}}{{/nfd_qc}}
        0xFFFF
    };

    static const uint16_t nfd_qc_idx[] = {
    {{#nfd_qc}}{{#starts}}{{idx}},{{/starts}}{{/nfd_qc}}
    (sizeof(nfd_qc_data)/sizeof(uint16_t)) -1
    };


    static const uint16_t nfkd_qc_data[] = {
    {{#nfkd_qc}}{{#data}}{{cp}},{{/data}}{{/nfkd_qc}}
        0xFFFF
    };

    static const uint16_t nfkd_qc_idx[] = {
    {{#nfkd_qc}}{{#starts}}{{idx}},{{/starts}}{{/nfkd_qc}}
    (sizeof(nfkd_qc_data)/sizeof(uint16_t)) -1
    };



    bool is_nfc(char32_t c) {
        const uint8_t  idx   = c & 0xff;
        const uint16_t start = nfc_qc_idx[idx];
        const uint16_t end   = nfc_qc_idx[idx+1];
        auto it = std::lower_bound(nfc_qc_data+start, nfc_qc_data+end, c >> 8);
        //The data represent the character with NFC != Y
        return it == nfc_qc_data+end || *it != c >> 8;
    }

    bool is_nfkc(char32_t c) {
        const uint8_t  idx   = c & 0xff;
        const uint16_t start = nfkc_qc_idx[idx];
        const uint16_t end   = nfkc_qc_idx[idx+1];
        auto it = std::lower_bound(nfkc_qc_data+start, nfkc_qc_data+end, c >> 8);
        //The data represent the character with NFKC != Y
        return it == nfkc_qc_data+end || *it != c >> 8;
    }

    bool is_nfd(char32_t c) {
        const uint8_t  idx   = c & 0xff;
        const uint16_t start = nfd_qc_idx[idx];
        const uint16_t end   = nfd_qc_idx[idx+1];
        const uint16_t truncated = c >> 8;
        for(auto it = nfd_qc_data+start; it < nfd_qc_data+end; ++it) {
            if(BOOST_LIKELY(*it > truncated))
                return true;
            if(BOOST_UNLIKELY(*it == truncated))
                return false;
        }
        return true;
    }

    bool is_nfkd(char32_t c) {
        const uint8_t  idx   = c & 0xff;
        const uint16_t start = nfkd_qc_idx[idx];
        const uint16_t end   = nfkd_qc_idx[idx+1];
        auto it = std::lower_bound(nfkd_qc_data+start, nfkd_qc_data+end, c >> 8);
        //The data represent the character with nfkd != Y
        return it == nfkd_qc_data+end || *it != c >> 8;
    }


    const ranges::iterator_range<const decomposition_jumping_table_item*> & decomposition_jumping_table() {
       static const auto v = ranges::view::all(decomposition_jumping_table_data);
       return v;
    }

    const ranges::iterator_range<const char32_t*> & decomposition_rules() {
        static const auto v = ranges::view::all(decomposition_rules_data);
        return v;
    }

    /*const ranges::iterator_range<const combining_class_item*> & combining_classes() {
        static const auto v = ranges::view::all(combining_classes_data);
        return v;
    }*/

    const ranges::iterator_range<const composable_sequence_jumping_table_item*> & composable_sequence_jumping_table() {
        static const auto v = ranges::view::all(composable_sequence_jumping_table_data);
        return v;
    }

    const ranges::iterator_range<const composable_sequence*> & composable_sequences() {
        static const auto v = ranges::view::all(recomposition_data);
        return v;
    }

    const ranges::iterator_range<const hangul_syllable*> & hangul_syllable_table() {
        static const auto v = ranges::view::all(hangul_data);
        return v;
    }
}
