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

    ranges::iterator_range<const decomposition_jumping_table_item*> decomposition_jumping_table() {
       return ranges::view::all(decomposition_jumping_table_data);
    }

    ranges::iterator_range<const char32_t*> decomposition_rules() {
        return ranges::view::all(decomposition_rules_data);
    }
    ranges::iterator_range<const combining_class_item*> combining_classes() {
        return ranges::view::all(combining_classes_data);
    }
}
