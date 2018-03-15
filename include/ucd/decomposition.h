#pragma once
#include <algorithm>
#include <array>
#include <boost/container/small_vector.hpp>
#include <range/v3/view_facade.hpp>
#include <range/v3/view.hpp>
#include <range/v3/view/all.hpp>

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

//

// cp, start, count
struct alignas(8) composable_sequence_jumping_table_item {
    uint32_t cp;
    uint16_t count;
    uint16_t start;
};

struct alignas(8) composable_sequence {
    char32_t c;
    char32_t l;
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
ranges::iterator_range<const composable_sequence*> composable_sequences();


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

    bool is_canonical() const {
        if(m_rule_size == 0)
            return true;
        return (*m_data) & canonical_bitmask;
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

inline auto find_rule_for_code_point(char32_t codepoint, bool canonical) {
    auto it = get_block_start(codepoint);
    if(it == decomposition_jumping_table().end())
        return rule{codepoint};
    auto view = make_view(it);
    auto needle = std::lower_bound(std::begin(view), std::end(view), codepoint);
    if(needle == std::end(view)) {
        return rule{codepoint};
    }
    const auto found = (*needle).codepoint();
    if(found != codepoint || (canonical && !(*needle).is_canonical()))
        return rule{codepoint};
    return *needle;
}

using buffer_t =
    std::vector<replacement>;    // boost::container::small_vector<details::replacement, 4>;

inline int decompose(char32_t codepoint, buffer_t& buffer, bool canonical = false) {
    const auto rule = details::find_rule_for_code_point(codepoint, canonical);
    int starter = 0;
    for(auto&& replacement : rule.replacements()) {
        auto c = replacement.codepoint();
        if(replacement.is_composed()) {
            starter += details::decompose(replacement.codepoint(), buffer,
                                          canonical);    // consider everything when recursing
            continue;
        }
        if(replacement.ccc() == 0)
            starter++;
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

inline auto sort_decomposition_buffer(buffer_t& buffer) {
    const auto b = std::begin(buffer);
    const auto is_starter = [](const replacement& r) { return r.ccc() == 0; };
    const auto first = std::find_if(b, std::end(buffer), is_starter);
    const auto end = (first == std::end(buffer)) ? std::end(buffer) : first + 1;
    canonical_sort(b, end);
    return end;
}

}    // namespace unicode::details

namespace unicode {

enum class NormalizationForm {
    NFD = 0x01,
    NFKD = 0x10,
    NFC = 0x02 | NFD,
    NFKC = 0x20 | NFD,
};

int constexpr as_int(NormalizationForm F) {
    return static_cast<std::underlying_type_t<NormalizationForm>>(F);
}

template<typename It>
It do_decompose_next(It begin, It end, details::buffer_t& buffer, bool canonical = true) {
    int starter_count = 0;
    for(auto it = begin; it != end; it++) {
        starter_count += details::decompose(*it, buffer, canonical);
        if(starter_count > 1)
            return std::next(it);
    }
    return end;
}

template<typename Rng>
struct normalization_view : ranges::v3::view_facade<normalization_view<Rng>, ranges::finite> {
    normalization_view(Rng&& rng, NormalizationForm normalization_form) :
        m_rng(std::forward<Rng>(rng)),
        m_normalization_form(normalization_form) {}
    struct cursor {
    private:
        using RngIt = typename Rng::const_iterator;
        RngIt m_it, m_end;
        details::buffer_t m_buffer;
        NormalizationForm m_normalization_form;
        details::buffer_t::iterator m_buffer_end, m_buffer_pos;

    public:
        cursor() = default;
        cursor(RngIt begin, RngIt end, NormalizationForm normalization_form) :
            m_it(begin),
            m_end(end),
            m_normalization_form(normalization_form) {

            decompose_next();
        }
        char32_t read() const {
            return m_buffer_pos->codepoint();
        }
        bool equal(ranges::default_sentinel) const {
            return (m_it == m_end) && m_buffer.empty();
        }
        void next() {
            m_buffer_pos++;
            if(m_buffer_pos == m_buffer_end) {
                decompose_next();
                return;
            }
        }
        void decompose_next() {
            m_buffer.erase(std::begin(m_buffer), m_buffer_end);
            if(m_buffer.empty()) {
                m_it = do_decompose_next(
                    m_it, m_end, m_buffer,
                    (as_int(m_normalization_form) & as_int(NormalizationForm::NFD)));
            }
            m_buffer_end = sort_decomposition_buffer(m_buffer);
            m_buffer_pos = std::begin(m_buffer);
        }
    };
    cursor begin_cursor() const {
        return {std::begin(m_rng), std::end(m_rng), m_normalization_form};
    }

private:
    friend struct ranges::range_access;
    Rng m_rng;
    NormalizationForm m_normalization_form;
};

template<NormalizationForm F, typename... Rng>
auto make_normalization_view(Rng... rng) {
    return normalization_view<decltype(ranges::view::all(rng...))>(ranges::view::all(rng...));
}

namespace views {
    struct normalize_fn {
    private:
        friend ranges::view::view_access;
        static auto bind(normalize_fn normalize, NormalizationForm Form) {
            return ranges::make_pipeable(std::bind(normalize, std::placeholders::_1, Form));
        }

    public:
        template<typename Rng>
        auto operator()(Rng&& rng, NormalizationForm Form) const {
            return normalization_view(std::move(rng), Form);
        }
    };    // namespace views
    inline namespace { inline ranges::view::view<normalize_fn> normalize; }
}    // namespace views

template<typename Rng, typename Ot>
void copy(Rng&& rng, Ot&& ot) {
    for(auto&& v : rng) {
        *ot++ = v;
    }
}

inline std::u32string normalized(const std::u32string& in, NormalizationForm F) {
    std::u32string out;
    auto v = ranges::view::all(in) | views::normalize(F);
    copy(v, std::back_inserter(out));
    return out;
}
}    // namespace unicode
