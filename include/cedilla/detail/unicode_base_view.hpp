#pragma once
#include <type_traits>
#include <range/v3/view_facade.hpp>
#include <range/v3/view.hpp>
#include <range/v3/view/all.hpp>
#include <experimental/text_view>
#include <string>


namespace cedilla {
namespace detail {
    template<typename Rng, typename Ot>
    void copy(Rng&& rng, Ot&& ot) {
        for(auto&& v : rng) {
            *ot++ = v;
        }
    }
    namespace concepts {
        template<typename T>
        using actual_type = std::remove_cv_t<std::remove_reference_t<std::remove_cv_t<T>>>;
        template<typename CU>
        constexpr bool is_code_unit_utf32() {
            return std::is_same_v<actual_type<CU>, char32_t>;
        }
        template<typename CU>
        constexpr bool is_code_unit_utf16() {
            return std::is_same_v<actual_type<CU>, char16_t>;
        }
        template<typename CU>
        constexpr bool is_code_unit_utf8() {
            return std::is_same_v<actual_type<CU>, char>;
        }

        namespace models {
            using namespace ranges::v3::concepts;
            struct CodepointInputRange : refines<InputRange> {
                template<typename C>
                auto requires_(C&& c)
                    -> decltype(ranges::concepts::valid_expr(ranges::concepts::is_true(
                        std::integral_constant<
                            bool, is_code_unit_utf32<ranges::range_value_type_t<C>>()>{})));
            };
            struct Utf8InputRange : refines<InputRange> {
                template<typename C>
                auto
                requires_(C&& c) -> decltype(ranges::concepts::valid_expr(ranges::concepts::is_true(
                    std::integral_constant<bool,
                                           is_code_unit_utf8<ranges::range_value_type_t<C>>()>{})));
            };
            struct Utf16InputRange : refines<InputRange> {
                template<typename C>
                auto requires_(C&& c)
                    -> decltype(ranges::concepts::valid_expr(ranges::concepts::is_true(
                        std::integral_constant<
                            bool, is_code_unit_utf16<ranges::range_value_type_t<C>>()>{})));
            };
        }    // namespace models
        template<typename T>
        using CodepointInputRange = ranges::concepts::models<models::CodepointInputRange, T>;
        template<typename T>
        using Utf8InputRange = ranges::concepts::models<models::Utf8InputRange, T>;
        template<typename T>
        using Utf16InputRange = ranges::concepts::models<models::Utf16InputRange, T>;
    }    // namespace concepts

    template<typename Derived, typename Range>
    struct unicode_view_base
        : public ranges::view_facade<Derived, ranges::is_finite<Range>::value ? ranges::finite
                                                                              : ranges::unknown> {


        template<typename Rng, typename Cont, typename I = ranges::range_common_iterator_t<Rng>>
        using Utf8Container =
            meta::strict_and<ranges::Range<Cont>, meta::not_<ranges::View<Cont>>,
                             ranges::MoveConstructible<Cont>,
                             ranges::Same<char, ranges::range_value_type_t<Cont>>>;
        template<typename Rng, typename Cont, typename I = ranges::range_common_iterator_t<Rng>>
        using Utf16Container =
            meta::strict_and<ranges::Range<Cont>, meta::not_<ranges::View<Cont>>,
                             ranges::MoveConstructible<Cont>,
                             ranges::Same<char16_t, ranges::range_value_type_t<Cont>>>;
        template<typename Rng, typename Cont, typename I = ranges::range_common_iterator_t<Rng>>
        using Utf32Container =
            meta::strict_and<ranges::Range<Cont>, meta::not_<ranges::View<Cont>>,
                             ranges::MoveConstructible<Cont>,
                             ranges::Same<char32_t, ranges::range_value_type_t<Cont>>>;


        // Convert to utf8 implicitely
        template<typename Container, typename D = Derived,
                 typename = typename Container::allocator_type,    // HACKHACK
                 CONCEPT_REQUIRES_(Utf8Container<D, Container>())>
        operator Container() {
            return convert<Container, std::experimental::text::utf8_encoding>();
        }
        // overload
        template<typename Container, typename D = Derived,
                 typename = typename Container::allocator_type,    // HACKHACK
                 CONCEPT_REQUIRES_(Utf8Container<D, Container>())>
        operator Container() const {
            return convert<Container, std::experimental::text::utf8_encoding>();
        }

        // Convert to utf16 implicitely
        template<typename Container, typename D = Derived,
                 typename = typename Container::allocator_type,    // HACKHACK
                 CONCEPT_REQUIRES_(Utf16Container<D, Container>())>
        operator Container() {
            return convert<Container, std::experimental::text::utf16_encoding>();
        }
        // overload
        template<typename Container, typename D = Derived,
                 typename = typename Container::allocator_type,    // HACKHACK
                 CONCEPT_REQUIRES_(Utf16Container<D, Container>())>
        operator Container() const {
            return convert<Container, std::experimental::text::utf16_encoding>();
        }

        // Convert to utf32 implicitely
        template<typename Container, typename D = Derived,
                 typename = typename Container::allocator_type,    // HACKHACK
                 CONCEPT_REQUIRES_(Utf32Container<D, Container>())>
        operator Container() {
            return ranges::to_<Container>(this->derived());
        }
        // overload
        template<typename Container, typename D = Derived,
                 typename = typename Container::allocator_type,    // HACKHACK
                 CONCEPT_REQUIRES_(Utf32Container<D, Container>())>
        operator Container() const {
            return ranges::to_<Container>(this->derived());
        }


    private:
        template<typename Container, typename EC>
        Container convert() {
            Container c;
            auto ot = std::experimental::text::make_otext_iterator<EC>(ranges::back_inserter(c));
            auto view = std::experimental::make_text_view<std::experimental::text::utf32_encoding>(
                this->derived());
            ranges::copy(view, ot);
            return c;
        }
        template<typename Container, typename EC>
        Container convert() const {
            Container c;
            auto ot = std::experimental::text::make_otext_iterator<EC>(ranges::back_inserter(c));
            auto view = std::experimental::make_text_view<std::experimental::text::utf32_encoding>(
                this->derived());
            ranges::copy(view, ot);
            return c;
        }
    };


}    // namespace detail
}    // namespace cedilla
