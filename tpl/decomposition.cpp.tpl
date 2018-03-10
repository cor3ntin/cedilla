#include <ucd/decomposition.h>

namespace unicode::details {
    const std::array<decomposition_jumping_table_item, {{block_count}}> decomposition_jumping_table = {
        {{#blocks}}
        decomposition_jumping_table_item{ {{first}}, {{start}}, {{size}}, {{number_of_replacements}} } {{#comma}},{{/comma}}  //{{name}} [{{first}}, {{last}}]
        {{/blocks}}
    };

    const std::array<char32_t, {{total_decompositition_items}} + 1> decomposition_rules = {
        {{#blocks}}
        //{{name}} [{{first}}, {{last}}]

        {{#chars}}
        {{cp}}{{#canonical}}|canonical_bitmask{{/canonical}},{{#sub}}{{dec_cp}}{{^final}}|composed_bitmask{{/final}}{{#comma}},{{/comma}}{{/sub}}
        {{/chars}}


        {{/blocks}}
        0
    };
}
