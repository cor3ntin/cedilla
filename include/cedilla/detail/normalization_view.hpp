#include <algorithm>
#include <array>
#include <optional>
#include <type_traits>
#include <range/v3/view_facade.hpp>
#include <range/v3/view.hpp>
#include <range/v3/view/all.hpp>
#include <boost/container/small_vector.hpp>
#include <cedilla/detail/hangul.hpp>
#include <cedilla/detail/unicode_base_view.hpp>

namespace cedilla {

namespace detail {
    constexpr int canonical_normalization_mask = 0x10;
    constexpr int composition_normalization_mask = 0x20;
}    // namespace detail

enum class normalization_form {
    nfd = 1 | detail::canonical_normalization_mask,
    nfkd = 2,
    nfc = 3 | detail::canonical_normalization_mask | detail::composition_normalization_mask,
    nfkc = 4 | detail::composition_normalization_mask
};

namespace detail {

    struct decomposition_jumping_table_item {
        // smallest codepoint of the block
        char32_t cp;
        // start of the block, relative to the start of the decomposition_rules array
        uint32_t start;
        // Number of rules in the block
        uint16_t count;
        // Numbers of replacement characters. Each element is rule_size + 1 in size
        uint8_t rule_size;

        bool has_canonical;
    };

    //

    // cp, start, count
    struct alignas(8) composable_sequence_jumping_table_item {
        uint32_t cp;
        uint16_t start;
        uint16_t count;
    };

    struct alignas(8) composable_sequence {
        char32_t c;
        char32_t p;
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
    const ranges::iterator_range<const decomposition_jumping_table_item*>&
    decomposition_jumping_table();
    // Implementation is generated from unicode data
    const ranges::iterator_range<const char32_t*>& decomposition_rules();
    // Implementation is generated from unicode data
    // const ranges::iterator_range<const combining_class_item*>& combining_classes();

    const ranges::iterator_range<const composable_sequence_jumping_table_item*>&
    composable_sequence_jumping_table();

    const ranges::iterator_range<const composable_sequence*>& composable_sequences();


    bool is_nfc(char32_t c);
    bool is_nfkc(char32_t c);
    bool is_nfd(char32_t c);
    bool is_nfkd(char32_t c);


    BOOST_FORCEINLINE bool operator<(const combining_class_item& cci, char32_t codepoint) {
        return cci.cp < codepoint;
    }

    BOOST_FORCEINLINE bool operator<(const composable_sequence_jumping_table_item& i,
                                     char32_t codepoint) {
        return i.cp < codepoint;
    }

    BOOST_FORCEINLINE bool operator<(const composable_sequence& cs, char32_t codepoint) {
        return cs.c < codepoint;
    }


    uint8_t combining_class(char32_t codepoint);

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
        replacement() = delete;
        replacement(char32_t cp) : m_cp(cp), m_ccc(combining_class(codepoint())) {}
        replacement(
            std::experimental::text::character<std::experimental::text::unicode_character_set> cp) :
            m_cp(cp),
            m_ccc(combining_class(codepoint())) {}


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
        rule(rule_size_t rule_size, const char32_t* const data) : m_rule_size(rule_size) {
            u.data = data;
        }
        rule(char32_t codepoint) : m_rule_size(0) {
            u.data = nullptr;
            u.codepoint = codepoint;
        }

        rule(const rule& other) = default;
        rule(rule&& other) = default;

        char32_t codepoint() const {
            // if(unlikely(m_rule_size == 0))
            //    return u.codepoint;
            return (*u.data) & ~(canonical_bitmask);
        }

        bool is_canonical() const {
            // if(m_rule_size == 0)
            //    return true;
            return (*u.data) & canonical_bitmask;
        }

        auto replacements() const {
            if(m_rule_size == 0) {
                return replacements_view(&u.codepoint, 1);
            }
            return replacements_view(u.data + 1, m_rule_size);
        }


    private:
        const rule_size_t m_rule_size;
        union {
            const char32_t* data;
            char32_t codepoint;
        } u;

    public:
        class replacements_view
            : public ranges::v3::view_facade<replacements_view, ranges::finite> {
        public:
            replacements_view(const char32_t* start, const rule_size_t rule_size) :
                m_start(start),
                m_rule_size(rule_size) {}

        private:
            const char32_t* m_start;
            const rule_size_t m_rule_size;
            friend struct ranges::v3::range_access;

            struct cursor {
            private:
                const char32_t* m_pos;
                const char32_t* m_start;
                rule_size_t m_rule_size;

            public:
                cursor() : m_pos(nullptr), m_start(nullptr), m_rule_size(0) {}
                cursor(const char32_t* data, rule_size_t rule_size) :
                    m_pos(data),
                    m_start(data),
                    m_rule_size(rule_size) {}
                replacement read() const {
                    return replacement(*m_pos);
                }
                bool equal(ranges::v3::default_sentinel) const {
                    return m_pos == m_start + m_rule_size || BOOST_UNLIKELY(*m_pos == nil_cp);
                }
                void next() {
                    m_pos++;
                }
            };
            cursor begin_cursor() const {
                return cursor(m_start, m_rule_size);
            }
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
        friend struct ranges::v3::range_access;
        struct cursor {
        private:
            rule_size_t m_rule_size;
            const char32_t* m_curser_pos;
            std::make_unsigned_t<size_t> m_factor;

        public:
            cursor() = default;
            cursor(rule_size_t rule_size, const char32_t* data) :
                m_rule_size(rule_size),
                m_factor(factor()),
                m_curser_pos(data) {}
            BOOST_FORCEINLINE rule read() const {
                return rule(m_rule_size, m_curser_pos);
            }

            BOOST_FORCEINLINE bool equal(cursor const& other) const {
                return m_curser_pos == other.m_curser_pos;
            }

            BOOST_FORCEINLINE void next() {
                m_curser_pos += m_factor;
            }

            std::ptrdiff_t distance_to(cursor const& other) const {
                return (other.m_curser_pos - m_curser_pos) / m_factor;
            }
            void advance(std::ptrdiff_t n) {
                m_curser_pos += n * m_factor;
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
        static const auto& jp = decomposition_jumping_table();
        const auto begin = std::begin(jp);
        const auto end = std::end(jp);

        auto needle = std::lower_bound(begin, end, codepoint,
                                       [](const decomposition_jumping_table_item& entry,
                                          const char32_t& value) { return entry.cp < value; });
        if(needle->cp > codepoint && needle != begin)
            return needle - 1;
        return needle;
    }

    inline auto make_view(
        std::remove_reference_t<
            std::remove_cv_t<decltype(decomposition_jumping_table())>>::const_iterator& iterator) {
        static const auto& rules = decomposition_rules();
        const auto begin = std::begin(rules) + iterator->start;
        const auto count = iterator->count;
        const auto rule_size = iterator->rule_size;
        return decomposition_view(rule_size, begin, count);
    }

    inline rule find_rule_for_code_point(char32_t codepoint, bool canonical) {
        auto it = get_block_start(codepoint);
        if(canonical && !it->has_canonical) {
            return rule{codepoint};
        }

        // TODO : assert if it == end
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

    using buffer_t = boost::container::small_vector<detail::replacement, 6>;

    inline int decompose(char32_t codepoint, buffer_t& buffer, bool canonical = false) {
        if(is_decomposable_hangul(codepoint)) {
            auto index = hangul_syllable_index(codepoint);
            auto lpart = hangul_lbase + (index / hangul_ncount);
            auto vpart = hangul_vbase + (index % hangul_ncount) / hangul_tcount;

            buffer.emplace_back(replacement{lpart});
            buffer.emplace_back(replacement{vpart});

            auto tpart = index % hangul_tcount;
            if(tpart > 0) {
                tpart = tpart + hangul_tbase;
                buffer.emplace_back(replacement{tpart});
            }
            return tpart > 0 ? 3 : 2;
        }

        bool quick_check =
            !is_hangul(codepoint) && (canonical ? is_nfd(codepoint) : is_nfkd(codepoint));
        const rule r =
            quick_check ? rule{codepoint} : detail::find_rule_for_code_point(codepoint, canonical);

        int starter = 0;

        for(auto replacement : r.replacements()) {
            if(BOOST_UNLIKELY(replacement.is_composed())) {
                starter += detail::decompose(replacement.codepoint(), buffer,
                                             canonical);    // consider everything when recursing
                continue;
            }
            if(replacement.ccc() == 0)
                starter++;
            buffer.emplace_back(std::move(replacement));
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

    int constexpr as_int(normalization_form F) {
        return static_cast<std::underlying_type_t<normalization_form>>(F);
    }

    template<typename It>
    It do_decompose_next(It begin, It end, detail::buffer_t& buffer, int& starter_count,
                         bool canonical = true) {
        for(auto it = begin; it != end; it++) {
            starter_count += detail::decompose(*it, buffer, canonical);
            if(starter_count > 1)
                return std::next(it);
        }
        return end;
    }

    inline std::optional<char32_t> find_primary_composite(char32_t c, char32_t l) {
        using namespace detail;
        // find c
        const auto& jp_table = composable_sequence_jumping_table();
        const auto it = std::lower_bound(std::begin(jp_table), std::end(jp_table), c);
        if(it == jp_table.end() || it->cp != c)
            return {};
        const auto composable_with_c =
            ranges::view::slice(composable_sequences(), it->start, it->start + it->count);
        const auto jt =
            std::lower_bound(std::begin(composable_with_c), std::end(composable_with_c), l);
        if(jt == composable_with_c.end() || jt->c != l)
            return {};

        return jt->p;
    }

    inline void composition_algorithm(detail::buffer_t& buffer) {
        const auto end = std::end(buffer);
        const auto begin = std::begin(buffer);
        if(end == begin || begin->ccc() != 0)
            return;
        auto starter = begin;
        for(auto it = begin + 1; it != std::end(buffer);) {
            auto prev = std::prev(it);
            if(!(prev == starter || (prev->ccc() != 0 && prev->ccc() < it->ccc()))) {
                // if(std::prev(it)->ccc() == 0)
                //     starter = std::prev(it);
                ++it;
                continue;
            }
            if(detail::is_hangul_lpart(starter->codepoint())) {
                const auto lpart = starter->codepoint();
                const auto vpart = it->codepoint();
                if(detail::is_hangul_vpart(vpart)) {
                    const auto tit = it + 1;
                    const bool has_tpart =
                        tit != std::end(buffer) && detail::is_hangul_tpart(tit->codepoint());
                    const auto tpart = has_tpart ? tit->codepoint() : detail::hangul_tbase;

                    const auto lindex = lpart - detail::hangul_lbase;
                    const auto vindex = vpart - detail::hangul_vbase;
                    const auto lvindex =
                        lindex * detail::hangul_ncount + vindex * detail::hangul_tcount;
                    const auto tindex = tpart - detail::hangul_tbase;
                    const auto replacement = detail::hangul_sbase + lvindex + tindex;

                    *starter = replacement;
                    it = buffer.erase(it, (has_tpart ? tit : it) + 1);
                    continue;
                } else {
                    ++it;
                }
            } else if(detail::is_hangul_tpart(it->codepoint()) &&
                      detail::is_decomposable_hangul(starter->codepoint())) {

                auto index = detail::hangul_syllable_index(starter->codepoint());
                if((index % detail::hangul_tcount) != 0) {
                    ++it;
                    continue;
                }

                const auto tindex = it->codepoint() - (detail::hangul_tbase);
                const auto replacement = starter->codepoint() + tindex;
                *starter = replacement;
                it = buffer.erase(it);
                continue;
            } else if(auto replacement =
                          find_primary_composite(it->codepoint(), starter->codepoint())) {
                *starter = *replacement;
                it = buffer.erase(it);
            } else {
                ++it;
            }
            if(std::prev(it)->ccc() == 0)
                starter = std::prev(it);
        }
    }

    template<typename Rng>
    struct normalization_view : unicode_view_base<normalization_view<Rng>, Rng> {
        normalization_view(Rng&& rng, normalization_form normalization_form) :
            m_rng(std::forward<Rng>(rng)),
            m_normalization_form(normalization_form) {}
        struct cursor {
        private:
            using RngIt = typename Rng::iterator;
            RngIt m_it, m_end;
            detail::buffer_t m_buffer;
            std::size_t m_buffer_pos;
            int m_starter_count;
            bool m_canonical;
            bool m_combine;

        public:
            cursor() = default;
            cursor(RngIt begin, RngIt end, normalization_form normalization_form) :
                m_it(begin),
                m_end(end),
                m_starter_count(0),
                m_canonical(as_int(normalization_form) & detail::canonical_normalization_mask),
                m_combine(as_int(normalization_form) & detail::composition_normalization_mask) {

                m_buffer_pos = 0;
                decompose_next();
            }
            char32_t read() const {
                return m_buffer.at(m_buffer_pos).codepoint();
            }
            bool equal(ranges::default_sentinel) const {
                return (m_it == m_end) && m_buffer.empty();
            }
            void next() {
                if(m_buffer.at(m_buffer_pos).ccc() == 0)
                    m_starter_count--;

                m_buffer_pos++;
                if(m_buffer_pos == m_buffer.size() || (m_combine && m_starter_count < 3)) {
                    decompose_next();
                    return;
                }
            }
            void decompose_next() {
                const auto dist =
                    std::distance(std::begin(m_buffer) + m_buffer_pos, std::end(m_buffer));
                m_buffer.erase(std::begin(m_buffer), std::begin(m_buffer) + m_buffer_pos);
                m_buffer_pos = 0;

                if(m_combine) {
                    uint8_t ccc = 0;
                    auto saved_it = m_it;
                    while(m_it != m_end) {
                        // We found a new starter, copy the character before it
                        auto c_ccc = detail::combining_class(*m_it);
                        if(m_it != saved_it && c_ccc == 0) {
                            std::copy(saved_it, m_it, std::back_inserter(m_buffer));
                            saved_it = m_it;
                            m_starter_count++;
                        }
                        auto is_nfc = [this]() {
                            return m_canonical ? detail::is_nfc(*m_it) : detail::is_nfkc(*m_it);
                        };
                        if((ccc > c_ccc && c_ccc != 0) || !is_nfc()) {
                            break;
                        }
                        ccc = c_ccc;
                        m_it++;
                    }
                    m_it = saved_it;
                }

                while(m_it != m_end && (m_buffer.empty() || (m_combine && m_starter_count < 3))) {
                    m_it = do_decompose_next(m_it, m_end, m_buffer, m_starter_count, m_canonical);
                }
                canonical_sort(std::begin(m_buffer) + dist, std::end(m_buffer));

                if(m_combine) {
                    composition_algorithm(m_buffer);
                    m_starter_count =
                        std::count_if(std::begin(m_buffer), std::end(m_buffer),
                                      [](const detail::replacement& c) { return c.ccc() == 0; });
                }

                m_buffer_pos = 0;
            }
        };
        cursor begin_cursor() const {
            return {std::begin(m_rng), std::end(m_rng), m_normalization_form};
        }

    private:
        friend struct ranges::range_access;
        Rng m_rng;
        normalization_form m_normalization_form;
    };
}    // namespace detail
}    // namespace cedilla
