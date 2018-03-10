#pragma once
#include <algorithm>
#include <array>
#include <boost/container/small_vector.hpp>
#include <range/v3/view_facade.hpp>
#include <range/v3/view.hpp>

namespace unicode::details {

struct decomposition_jumping_table_item {
    // smallest codepoint of the block
    char32_t cp;
    // start of the block, relative to the start of the decomposition_rules array
    char32_t start;
    // Number of rules in the block
    uint16_t count;
    // Numbers of replacement characters. Each element is rule_size + 1 in size
    uint16_t rule_size;
};

using rule_size_t = decltype(decomposition_jumping_table_item::rule_size);

// Placeholder for replacements rules (as opposed to decomposition)
constexpr char32_t nil_cp = 0x0000FDD0;
constexpr char32_t composed_bitmask = 0x80000000;
constexpr char32_t canonical_bitmask = 0x80000000;

// extern decomposition_jumping_table;
// extern decomposition_rules;
#include <ucd/details/decomposition_data.h>

class rule {
public:
    class replacement {
        const char32_t* const m_data;
        friend class rule;

    public:
        replacement(const char32_t* const data) : m_data(data) {}

        char32_t codepoint() const {
            return (*m_data) & ~(composed_bitmask);
        }

        bool is_composed() const {
            return (*m_data) & composed_bitmask;
        }
    };
    class replacements_view : public ranges::v3::view_facade<replacements_view, ranges::finite> {
    public:
        replacements_view(const char32_t* start, const rule_size_t rule_size) :
            m_start(start),
            m_rule_size(rule_size) {}

    private:
        const char32_t* m_start;
        const rule_size_t m_rule_size;
        friend ranges::v3::range_access;

        struct cursor {
        private:
            const char32_t* m_pos;

        public:
            cursor() = default;
            cursor(const char32_t* data) : m_pos(data) {}
            replacement read() const {
                return replacement(m_pos);
            }
            bool equal(cursor const& other) const {
                return m_pos == other.m_pos;
            }
            bool equal(ranges::v3::default_sentinel) const {
                return *m_pos == nil_cp;
            }
            void next() {
                m_pos++;
            }
            void prev() {
                m_pos--;
            }
            std::ptrdiff_t distance_to(cursor const& other) const {
                return (other.m_pos - m_pos);
            }
            void advance(std::ptrdiff_t n) {
                m_pos += n;
            }
        };
        cursor begin_cursor() const {
            return {m_start};
        }
        cursor end_cursor() const {
            return {m_start + m_rule_size};
        }
    };

    rule(rule_size_t rule_size, const char32_t* const data) :
        m_rule_size(rule_size),
        m_data(data) {}
    rule(char32_t codepoint) : m_rule_size(0), m_codepoint(codepoint) {}

    char32_t codepoint() const {
        if(m_rule_size == 0)
            return m_codepoint;
        return (*m_data) & ~(canonical_bitmask);
    }

    auto replacements() {
        if(m_rule_size == 0) {
            return replacements_view(&m_codepoint, 1);
        }
        return replacements_view(m_data + 1, m_rule_size);
    }

private:
    const rule_size_t m_rule_size;
    union {
        const char32_t* const m_data;
        const char32_t m_codepoint;
    };
};

inline bool operator<(const rule& r, char32_t codepoint) {
    return r.codepoint() < codepoint;
}

class decomposition_view : public ranges::v3::view_facade<decomposition_view, ranges::finite> {
public:
    decomposition_view(rule_size_t rule_size, const char32_t* data, uint32_t rule_count) :
        m_rule_size(rule_size),
        m_pos(data),
        m_rule_count(rule_count) {}

private:
    uint32_t m_rule_count;
    rule_size_t m_rule_size;
    const char32_t* m_pos;
    friend ranges::v3::range_access;

    struct cursor {
    private:
        rule_size_t m_rule_size;
        const char32_t* m_curser_pos;

    public:
        cursor() = default;
        cursor(rule_size_t rule_size, const char32_t* data) :
            m_rule_size(rule_size),
            m_curser_pos(data) {}
        rule read() const {
            return rule(m_rule_size, m_curser_pos);
        }
        bool equal(cursor const& other) const {
            return m_curser_pos == other.m_curser_pos;
        }
        void next() {
            m_curser_pos += factor();
        }
        void prev() {
            m_curser_pos -= factor();
        }
        std::ptrdiff_t distance_to(cursor const& other) const {
            return (other.m_curser_pos - m_curser_pos) / factor();
        }
        void advance(std::ptrdiff_t n) {
            m_curser_pos += n * factor();
        }

        std::make_unsigned_t<size_t> factor(size_t n = 1) const {
            return n * (m_rule_size + 1);
        }
    };
    cursor begin_cursor() const {
        return {m_rule_size, m_pos};
    }
    cursor end_cursor() const {
        return {m_rule_size, m_pos + m_rule_count * (m_rule_size + 1)};
    }
};

inline decltype(decomposition_jumping_table)::const_iterator get_block_start(char32_t codepoint) {
    const auto end = std::end(decomposition_jumping_table);
    auto needle = std::lower_bound(std::begin(decomposition_jumping_table), end, codepoint,
                                   [](const decomposition_jumping_table_item& entry,
                                      const char32_t value) { return entry.cp < value; });
    return needle;
}

inline auto make_view(decltype(decomposition_jumping_table)::const_iterator& iterator) {
    const char32_t* begin = decomposition_rules.data() + iterator->start;
    const auto count = iterator->count;
    const auto rule_size = iterator->rule_size;
    return decomposition_view(rule_size, begin, count);
}

inline auto find_rule_for_code_point(char32_t codepoint) {
    auto it = get_block_start(codepoint);
    if(it == std::end(decomposition_jumping_table))
        return rule{codepoint};
    auto view = make_view(it);
    auto needle = std::lower_bound(std::begin(view), std::end(view), codepoint);
    if(needle == std::end(view) || (*needle).codepoint() != codepoint)
        return rule{codepoint};
    return *needle;
}


}    // namespace unicode::details

namespace unicode {
template<typename Container>
void decompose(char32_t codepoint, Container& c) {
    auto rule = details::find_rule_for_code_point(codepoint);
    for(auto&& replacement : rule.replacements()) {
        if(replacement.is_composed()) {
            decompose(replacement.codepoint(), c);
            continue;
        }
        c.push_back(replacement.codepoint());
    }
}
inline std::u32string decomposed(const std::u32string& in) {
    std::u32string out;
    for(auto& codepoint : in) {
        decompose(codepoint, out);
    }
    // out.push_back('\0');
    return out;
}


}    // namespace unicode
