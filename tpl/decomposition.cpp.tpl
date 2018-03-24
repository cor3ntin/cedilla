#include <ucd/decomposition.h>

namespace unicode::details {
    static const std::array<decomposition_jumping_table_item, {{block_count}}+1> decomposition_jumping_table_data = {
        {{#blocks}}
        decomposition_jumping_table_item{ {{first}}, {{start}}, {{size}}, {{number_of_replacements}} },  //{{name}} [{{first}}, {{last}}]
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
    static const combining_class_item combining_classes_data[] = {
    {{#combining_classes}}
    { {{cp}}, {{ccc}} },
    {{/combining_classes}}
    { 0xFFFFFF, 0 }
    };

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

    ranges::iterator_range<const decomposition_jumping_table_item*> decomposition_jumping_table() {
       return ranges::view::all(decomposition_jumping_table_data);
    }

    ranges::iterator_range<const char32_t*> decomposition_rules() {
        return ranges::view::all(decomposition_rules_data);
    }
    ranges::iterator_range<const combining_class_item*> combining_classes() {
        return ranges::view::all(combining_classes_data);
    }

    ranges::iterator_range<const composable_sequence_jumping_table_item*> composable_sequence_jumping_table() {
        return ranges::view::all(composable_sequence_jumping_table_data);
    }

    ranges::iterator_range<const composable_sequence*> composable_sequences() {
        return ranges::view::all(recomposition_data);
    }

    ranges::iterator_range<const hangul_syllable*> hangul_syllable_table() {
        return ranges::view::all(hangul_data);
    }
}
