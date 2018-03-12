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
    uint32_t start;
    // Number of rules in the block
    uint16_t count;
    // Numbers of replacement characters. Each element is rule_size + 1 in size
    uint16_t rule_size;
};

struct alignas(sizeof(char32_t)) combining_class_item {
    char32_t cp : 24;
    uint8_t ccc;
};

using rule_size_t = decltype(decomposition_jumping_table_item::rule_size);

// Placeholder for replacements rules (as opposed to decomposition)
constexpr char32_t nil_cp = 0x0000FDD0;
constexpr char32_t composed_bitmask = 0x80000000;
constexpr char32_t canonical_bitmask = 0x80000000;

// Implementation is generated from unicode data
// We split the data per code blocks to improve caching
// Also, the number of replacement depends on the block
ranges::iterator_range<const decomposition_jumping_table_item*> decomposition_jumping_table();
// Implementation is generated from unicode data
ranges::iterator_range<const char32_t*> decomposition_rules();
// Implementation is generated from unicode data
ranges::iterator_range<const combining_class_item*> combining_classes();


inline bool operator<(const combining_class_item& cci, char32_t codepoint) {
    return cci.cp < codepoint;
}

inline uint8_t combining_class(char32_t codepoint) {
    const auto ccc_data = combining_classes();
    const auto end = std::end(ccc_data);
    auto it = std::lower_bound(std::begin(ccc_data), end, codepoint);
    if(it == end || it->cp != codepoint)
        return 0;
    return it->ccc;
}

class replacement {
    char32_t m_cp;
    uint8_t m_ccc;

    friend class rule;

    friend void swap(replacement& a, replacement& b) {
        using std::swap;
        std::swap(a.m_ccc, b.m_ccc);
        std::swap(a.m_cp, b.m_cp);
    }

public:
    replacement(char32_t cp) : m_cp(cp) {
        m_ccc = combining_class(codepoint());
    }

    char32_t codepoint() const {
        return m_cp & ~(composed_bitmask);
    }

    bool is_composed() const {
        return m_cp & composed_bitmask;
    }
    uint8_t ccc() const {
        return m_ccc;
    }
};

class rule {
public:
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
            const char32_t* m_start;
            rule_size_t m_rule_size;

        public:
            cursor() = default;
            cursor(const char32_t* data, rule_size_t rule_size) :
                m_pos(data),
                m_start(data),
                m_rule_size(rule_size) {}
            replacement read() const {
                return replacement(*m_pos);
            }
            bool equal(ranges::v3::default_sentinel) const {
                return *m_pos == nil_cp || m_pos == m_start + m_rule_size;
            }
            void next() {
                m_pos++;
            }
            void prev() {
                m_pos--;
            }
        };
        cursor begin_cursor() const {
            return {m_start, m_rule_size};
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

    auto replacements() const {
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
    char32_t rcp = r.codepoint();
    return rcp < codepoint;
}

class decomposition_view : public ranges::v3::view_facade<decomposition_view> {
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

inline auto get_block_start(char32_t codepoint) {
    const auto jp = decomposition_jumping_table();

    const auto begin = std::begin(jp);
    const auto end = std::end(jp);
    auto needle = std::lower_bound(begin, end, codepoint,
                                   [](const decomposition_jumping_table_item& entry,
                                      const char32_t value) { return entry.cp < value; });
    if(needle->cp > codepoint && needle != begin)
        return needle - 1;
    return needle;
}

inline auto make_view(decltype(decomposition_jumping_table())::const_iterator& iterator) {
    const auto rules = decomposition_rules();
    const auto begin = std::begin(rules) + iterator->start;
    const auto count = iterator->count;
    const auto rule_size = iterator->rule_size;
    return decomposition_view(rule_size, begin, count);
}

inline auto find_rule_for_code_point(char32_t codepoint) {
    auto it = get_block_start(codepoint);
    if(it == decomposition_jumping_table().end())
        return rule{codepoint};
    auto view = make_view(it);
    auto needle = std::lower_bound(std::begin(view), std::end(view), codepoint);
    if(needle == std::end(view)) {
        return rule{codepoint};
    }
    const auto found = (*needle).codepoint();
    if(found != codepoint)
        return rule{codepoint};
    return *needle;
}

using buffer_t = std::vector<details::replacement>;
// boost::container::small_vector<details::replacement, 4>;

inline bool decompose(char32_t codepoint, buffer_t& buffer) {
    const auto rule = details::find_rule_for_code_point(codepoint);
    bool starter = false;
    for(auto&& replacement : rule.replacements()) {
        if(replacement.is_composed()) {
            details::decompose(replacement.codepoint(), buffer);
            continue;
        }
        if(!starter && replacement.ccc() == 0)
            starter = true;
        buffer.push_back(replacement);
    }
    return starter;
}

template<typename It>
void canonical_sort(It begin, It end) {
    if(std::distance(begin, end) < 2)
        return;
    bool found = false;
    do {
        found = false;
        for(auto it = begin; it + 1 != end; ++it) {
            const auto a = it->ccc(), b = (it + 1)->ccc();
            if(a > b && b > 0) {
                swap(*it, *(it + 1));
                found = true;
            }
        }
    } while(found);
}

template<typename Container>
void flush_buffer(buffer_t& buffer, Container& c) {
    const auto b = std::begin(buffer);
    const auto is_starter = [](const replacement& r) { return r.ccc() == 0; };
    const auto first = std::find_if(b, std::end(buffer), is_starter);
    const auto end = first == std::end(buffer)
                         ? std::end(buffer)
                         : std::find_if(first + 1, std::end(buffer), is_starter);
    canonical_sort(b, end);
    for(auto it = b; it != end; ++it) {
        c.push_back(it->codepoint());
    }
    buffer.erase(b, end);
}

}    // namespace unicode::details

namespace unicode {
template<typename It, typename Container>
void decompose(It begin, It end, Container& c) {
    int starter_count = 0;
    details::buffer_t buffer;
    for(auto it = begin; it != end; ++it) {
        bool found_starter = details::decompose(*it, buffer);
        if(found_starter) {
            starter_count++;
            if(starter_count > 1) {
                flush_buffer(buffer, c);
                starter_count--;
            }
        }
    }
    canonical_sort(std::begin(buffer), std::end(buffer));
    for(auto&& replacement : buffer)
        c.push_back(replacement.codepoint());
}

inline std::u32string decomposed(const std::u32string& in) {
    std::u32string out;
    decompose(std::begin(in), std::end(in), out);
    return out;
}


}    // namespace unicode
