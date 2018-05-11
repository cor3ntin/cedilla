#pragma once
#include <cedilla/detail/normalization_view.hpp>
#include <type_traits>

namespace cedilla {


namespace views {
    struct normalize_fn {
    private:
        friend ranges::view::view_access;
        static auto bind(normalize_fn normalize, normalization_form Form) {
            return ranges::make_pipeable(std::bind(normalize, std::placeholders::_1, Form));
        }

    public:
        template<typename Rng, CONCEPT_REQUIRES_(detail::concepts::CodepointInputRange<Rng>())>
        auto operator()(Rng&& rng, normalization_form Form) const {
            return detail::normalization_view(std::move(rng), Form);
        }
    };
    inline namespace { inline ranges::view::view<normalize_fn> normalize; }
}    // namespace views


template<typename Rng, CONCEPT_REQUIRES_(detail::concepts::CodepointInputRange<Rng>())>
inline auto normalized(Rng&& in, normalization_form F) {
    auto v = ranges::view::all(in) | views::normalize(F);
    return v;
}

template<typename Rng, CONCEPT_REQUIRES_(detail::concepts::Utf8InputRange<Rng>())>
inline auto normalized(Rng&& in, normalization_form F) {
    using namespace std::experimental::text;
    auto v = cedilla::detail::normalization_view(
        std::experimental::text::make_text_view<utf8_encoding>(in), F);
    return v;
}

template<typename Rng, CONCEPT_REQUIRES_(detail::concepts::Utf16InputRange<Rng>())>
inline auto normalized(Rng&& in, normalization_form F) {
    using namespace std::experimental::text;
    auto v = cedilla::detail::normalization_view(
        std::experimental::text::make_text_view<utf16_encoding>(in), F);
    return v;
}


}    // namespace cedilla
