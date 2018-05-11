#pragma once
#include <cedilla/detail/normalization_view.hpp>

namespace cedilla {
namespace views {
    struct normalize_fn {
    private:
        friend ranges::view::view_access;
        static auto bind(normalize_fn normalize, normalization_form Form) {
            return ranges::make_pipeable(std::bind(normalize, std::placeholders::_1, Form));
        }

    public:
        template<typename Rng>
        auto operator()(Rng&& rng, normalization_form Form) const {
            return detail::normalization_view(std::move(rng), Form);
        }
    };
    inline namespace { inline ranges::view::view<normalize_fn> normalize; }
}    // namespace views

template<typename Rng, typename Ot>
void copy(Rng&& rng, Ot&& ot) {
    for(auto&& v : rng) {
        *ot++ = v;
    }
}

inline std::u32string normalized(const std::u32string& in, normalization_form F) {
    std::u32string out;
    out.reserve(in.size());
    auto v = ranges::view::all(in) | views::normalize(F);
    copy(v, std::back_inserter(out));
    return out;
}
}    // namespace cedilla
