#pragma once

#if __cplusplus < 202302L
#error out of date c++ version, compile with -stdc++=2c
#elif defined(__clang__) && __clang_major__ < 22
#error out of date clang, compile with latest version
#elif !defined(__clang__) && defined(__GNUC__) && __GNUC__ < 15
#error out of date g++, compile with latest version
#elif defined(_MSC_VER) && _MSC_VER < 19
#error out of date msvc, compile with latest version
#elif !defined(__clang__) && !defined(__GNUC__) && !defined(_MSC_VER)
#error compiler unknown, could not detect gcc, clang, or msvc
#else

#include <algorithm>
#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

namespace irv::views::xml {
    namespace detail {
        template<typename tp_type_t>
        concept character =
            std::same_as<
                char,
                tp_type_t
            >;

        template<typename tp_type_t>
        concept iterator_of_value_type_char = character<std::iter_value_t<tp_type_t>>;

        template<typename tp_type_t>
        concept forward_range_of_value_type_char =
            std::ranges::forward_range<tp_type_t> &&
            iterator_of_value_type_char<std::ranges::iterator_t<tp_type_t>>;

        struct is_iterator_default_initialized_fn {
            template<std::forward_iterator tp_iterator_t>
            [[nodiscard]]
            auto constexpr operator()(tp_iterator_t p_iterator)
            const noexcept(noexcept(p_iterator == tp_iterator_t{}))
            -> bool {
                return p_iterator == tp_iterator_t{};
            }
        };
        auto constexpr is_iterator_default_initialized = is_iterator_default_initialized_fn{};

        struct is_valid_tag_whitespace_fn {
            [[nodiscard]]
            auto constexpr operator()(const char p_char)
            const noexcept
            -> char {
                return
                    (p_char >= 9 && p_char <= 13) ||
                    p_char == 32;
            }
        };
        auto constexpr is_valid_tag_whitespace = is_valid_tag_whitespace_fn{};

        struct is_valid_tag_attribute_key_start_char_fn {
            [[nodiscard]]
            auto constexpr operator()(const char p_char)
            const noexcept
            -> char {
                return
                    (p_char >= 'a' && p_char <= 'z') ||
                    (p_char >= 'A' && p_char <= 'Z') ||
                    p_char == '_';
            }
        };
        auto constexpr is_valid_tag_attribute_key_start_char = is_valid_tag_attribute_key_start_char_fn{};

        struct is_valid_tag_attribute_key_char_fn {
            [[nodiscard]]
            auto constexpr operator()(const char p_char)
            const noexcept
            -> char {
                return
                    is_valid_tag_attribute_key_start_char(p_char) ||
                    (p_char >= '0' && p_char <= '9') ||
                    p_char == '-' ||
                    p_char == '.';
            }
        };
        auto constexpr is_valid_tag_attribute_key_char = is_valid_tag_attribute_key_char_fn{};

        struct is_valid_tag_name_start_char_fn {
            [[nodiscard]]
            auto constexpr operator()(const char p_char)
            const noexcept
            -> char {
                return
                    is_valid_tag_attribute_key_start_char(p_char) ||
                    p_char == ':' ||
                    p_char == '?';
            }
        };
        auto constexpr is_valid_tag_name_start_char = is_valid_tag_name_start_char_fn{};

        struct is_valid_tag_name_char_fn {
            [[nodiscard]]
            auto constexpr operator()(const char p_char)
            const noexcept
            -> char {
                return is_valid_tag_attribute_key_char(p_char) || p_char == ':';
            }
        };
        auto constexpr is_valid_tag_name_char = is_valid_tag_name_char_fn{};

        struct is_valid_body_char_fn {
            [[nodiscard]]
            auto constexpr operator()(const char p_char)
            const noexcept
            -> char {
                return
                    p_char == 9 ||
                    p_char == 10 ||
                    p_char == 13 ||
                    (p_char >= 32 && p_char <= 126 && p_char != 38 && p_char != 60);
            }
        };
        auto constexpr is_valid_body_char = is_valid_body_char_fn{};

        template<
            bool     tp_condition,
            typename tp_type_t
        >
        using const_if_t =
            std::conditional_t<
                tp_condition,
                std::add_const_t<tp_type_t>,
                tp_type_t
            >;
        
        template<forward_range_of_value_type_char>
        struct xml_elements_view;

        template<typename tp_range_t>
        concept xml_elements_viewable = requires { xml_elements_view{std::declval<tp_range_t>()}; };

        template<typename tp_range_t>
        concept no_throw_xml_elements_viewable = noexcept(xml_elements_view{std::declval<tp_range_t>()});

        struct elements_fn : std::ranges::range_adaptor_closure<elements_fn> {
            template<std::ranges::viewable_range tp_range_t>
            requires(xml_elements_viewable<tp_range_t>)
            [[nodiscard]]
            auto constexpr operator()(tp_range_t&& p_range)
            const noexcept(no_throw_xml_elements_viewable<tp_range_t>)
            -> auto {
                return xml_elements_view{std::forward<tp_range_t>(p_range)};
            }
        };
    }
    auto constexpr elements = detail::elements_fn{};

    namespace detail {
        template<std::forward_iterator tp_iterator_t>
        struct element_segmentation {
        private:
            template<forward_range_of_value_type_char>
            friend struct xml_elements_view;
            
            using m_subrange_t = std::ranges::subrange<tp_iterator_t>;

        public:
            m_subrange_t tag{};
            m_subrange_t attributes{};
            m_subrange_t body{};

            element_segmentation()
            requires(std::default_initializable<tp_iterator_t>)
            = default;

            constexpr explicit element_segmentation(
                tp_iterator_t p_tag_begin,
                tp_iterator_t p_tag_end,
                tp_iterator_t p_attributes_begin,
                tp_iterator_t p_attributes_end,
                tp_iterator_t p_body_begin,
                tp_iterator_t p_body_end
            )
            noexcept(
                std::is_nothrow_constructible_v<
                    m_subrange_t,
                    tp_iterator_t,
                    tp_iterator_t
                >
            ) :
            tag{
                std::move(p_tag_begin),
                std::move(p_tag_end)
            },
            attributes{
                std::move(p_attributes_begin),
                std::move(p_attributes_end)
            },
            body{
                std::move(p_body_begin),
                std::move(p_body_end)
            }
            {}

            constexpr explicit element_segmentation(tp_iterator_t p_error_position)
            noexcept(
                std::is_nothrow_constructible_v<
                    m_subrange_t,
                    tp_iterator_t,
                    tp_iterator_t
                > &&
                std::is_nothrow_default_constructible_v<tp_iterator_t>
            ) :
            tag{
                tp_iterator_t{},
                std::move(p_error_position)
            }
            {}
            
            template<typename tp_self_t>
            [[nodiscard]]
            auto constexpr elements(this tp_self_t&& p_self)
            noexcept(noexcept(xml::elements(std::forward<tp_self_t>(p_self).body)))
            -> decltype(xml::elements(std::forward<tp_self_t>(p_self).body)) {
                return xml::elements(std::forward<tp_self_t>(p_self).body);
            }
        };
        template<std::forward_iterator tp_iterator_t>
        element_segmentation(
            tp_iterator_t,
            tp_iterator_t,
            tp_iterator_t,
            tp_iterator_t,
            tp_iterator_t,
            tp_iterator_t
        ) ->
        element_segmentation<tp_iterator_t>;

        template<std::forward_iterator tp_iterator_t>
        element_segmentation(tp_iterator_t) ->
        element_segmentation<tp_iterator_t>;

        template<typename tp_iterator_t>
        struct element_segmentation_and_end {
            element_segmentation<tp_iterator_t> m_element_segmentation{};
            tp_iterator_t                       m_end{};
        };
        
        struct find_next_element_fn {
            template<
                std::input_iterator              tp_iterator_t,
                std::sentinel_for<tp_iterator_t> tp_sentinel_t
            >
            requires(iterator_of_value_type_char<tp_iterator_t>)
            [[nodiscard]]
            auto constexpr operator()(
                tp_iterator_t p_first,
                tp_sentinel_t p_last
            )
            const
            -> element_segmentation_and_end<tp_iterator_t> {
                for (;; ++p_first) {
                    if (p_first == p_last)
                        return element_segmentation_and_end<tp_iterator_t>{};
                    if (!is_valid_tag_whitespace(*p_first))
                        break;
                }
                
                // note: tag
                for (;; ++p_first) {
                    if (p_first == p_last)
                        return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                    if (*p_first == '<')
                        break;
                }
                ++p_first;
                if (!is_valid_tag_name_start_char(*p_first))
                    return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                auto const l_is_processing_instruction = *p_first == '?';
                auto l_tag_begin = p_first++;
                for (;; ++p_first) {
                    if (p_first == p_last)
                        return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                    if (!is_valid_tag_name_char(*p_first))
                        break;
                }
                auto l_tag_end = p_first;

                // note: attributes
                auto l_attributes_begin = tp_iterator_t{};
                auto l_attributes_end   = tp_iterator_t{};
                for (;; ++p_first) {
                    if (p_first == p_last)
                        return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                    if (!is_valid_tag_whitespace(*p_first))
                        break;
                }
                if (*p_first == '>')
                    ++p_first;
                else {
                    l_attributes_begin = p_first;
                    if (l_is_processing_instruction) {
                        auto l_closing_delimiter = std::ranges::search(
                            std::ranges::subrange{
                                std::move(p_first),
                                p_last
                            },
                            std::string_view{"?>"}
                        );
                        if (std::ranges::empty(l_closing_delimiter))
                            return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                        l_attributes_end = p_first = std::ranges::end(std::move(l_closing_delimiter));
                    }
                    else {
                        for (;; ++p_first) {
                            if (p_first == p_last)
                                return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                            if (*p_first == '>') {
                                l_attributes_end = p_first++;
                                break;
                            }
                        }
                    }
                }
                if (l_is_processing_instruction)
                    return element_segmentation_and_end<tp_iterator_t>{
                        element_segmentation{
                            std::move(l_tag_begin),
                            std::move(l_tag_end),
                            std::move(l_attributes_begin),
                            std::move(l_attributes_end),
                            tp_iterator_t{},
                            tp_iterator_t{}
                        },
                        std::move(p_first)
                    };

                // note: body
                auto l_body_begin = p_first;
                auto l_body_end   = tp_iterator_t{};
                auto l_depth      = std::size_t{};
                for (;; ++p_first) {
                    if (p_first == p_last)
                        return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                    if (*p_first == '<') {
                        auto l_potential_body_end = p_first++;
                        if (p_first == p_last)
                            return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                        auto const l_is_closing = *p_first == '/';
                        if (l_is_closing) {
                            ++p_first;
                            if (p_first == p_last)
                                return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                            if (l_depth == 0) {
                                if (
                                    auto l_mismatch =
                                        std::ranges::mismatch(
                                            p_first,
                                            p_last,
                                            l_tag_begin,
                                            l_tag_end
                                        );
                                    l_mismatch.in2 == l_tag_end
                                ) {
                                    p_first = std::move(l_mismatch.in1);
                                    if (p_first == p_last)
                                        return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                                    if (*p_first == '>') {
                                        l_body_end = std::move(l_potential_body_end);
                                        ++p_first;
                                        break;
                                    }
                                    else if (is_valid_tag_whitespace(*p_first)) {
                                        l_body_end = std::move(l_potential_body_end);
                                        for (;; ++p_first) {
                                            if (p_first == p_last)
                                                return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                                            if (!is_valid_tag_whitespace(*p_first))
                                                break;
                                        }
                                        if (*p_first != '>')
                                            return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                                        break;
                                    }
                                }
                                else if (l_depth == 0)
                                    return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                            }
                            else --l_depth;
                        }
                        else ++l_depth;
                    }
                    else if (!is_valid_body_char(*p_first))
                        return element_segmentation_and_end{element_segmentation{std::move(p_first)}};
                }                
                return
                    l_body_end == tp_iterator_t{} ?
                    element_segmentation_and_end{element_segmentation{std::move(p_first)}} :
                    element_segmentation_and_end<tp_iterator_t>{
                        element_segmentation{
                            std::move(l_tag_begin),
                            std::move(l_tag_end),
                            std::move(l_attributes_begin),
                            std::move(l_attributes_end),
                            std::move(l_body_begin),
                            std::move(l_body_end)
                        },
                        std::move(p_first)
                    };
            }
        };
        auto constexpr find_next_element = find_next_element_fn{};
        
        template<forward_range_of_value_type_char tp_view_t>
        class xml_elements_view : std::ranges::view_interface<xml_elements_view<tp_view_t>> {
        private:
            template<typename tp_range_t>
            struct cached_position
            {};
            template<std::ranges::forward_range tp_range_t>
            struct cached_position<tp_range_t> :
            std::optional<element_segmentation_and_end<std::ranges::iterator_t<tp_range_t>>>
            {};

            using m_view_iterator_t = std::ranges::iterator_t<tp_view_t>;
            using m_view_sentinel_t = std::ranges::sentinel_t<tp_view_t>;

            template<bool tp_is_const>
            struct iterator {
            private:
                template<typename tp_type_t>
                using m_maybe_const_t =
                    const_if_t<
                        tp_is_const,
                        tp_type_t
                    >;

                using m_parent_t        = m_maybe_const_t<xml_elements_view>;
                using m_base_view_t     = m_maybe_const_t<tp_view_t>;
                using m_base_iterator_t = std::ranges::iterator_t<m_base_view_t>;
                using m_base_sentinel_t = std::ranges::sentinel_t<m_base_view_t>;

                m_parent_t*                                     m_parent{};
                element_segmentation_and_end<m_base_iterator_t> m_current{};
            public:
                using iterator_concept = std::forward_iterator_tag;
                using value_type       = element_segmentation<m_base_iterator_t>;
                using difference_type  = std::ranges::range_difference_t<m_base_view_t>;

                friend xml_elements_view;
                friend iterator<!tp_is_const>;
                
                iterator()
                requires(std::default_initializable<m_base_iterator_t>)
                = default;
                
                constexpr iterator(
                    m_parent_t*                                     p_parent,
                    element_segmentation_and_end<m_base_iterator_t> p_current
                )
                noexcept(std::is_nothrow_move_constructible_v<m_base_iterator_t>) :
                m_parent{p_parent},
                m_current{std::move(p_current)}
                {}

                constexpr iterator(iterator<!tp_is_const> p_iterator)
                noexcept(
                    std::is_nothrow_convertible_v<
                        m_view_iterator_t,
                        m_base_iterator_t
                    >
                )
                requires(
                    tp_is_const &&
                    std::convertible_to<
                        m_view_iterator_t,
                        m_base_iterator_t
                    >
                ) :
                m_parent{std::move(p_iterator.m_parent)},
                m_current{std::move(p_iterator.p_current)}
                {}

                template<typename tp_self_t>
                [[nodiscard]]
                auto constexpr base(this tp_self_t&& p_self)
                noexcept(noexcept(std::ranges::begin(std::forward<tp_self_t>(p_self).tag)))
                -> m_base_iterator_t {
                    return std::ranges::begin(std::forward<tp_self_t>(p_self).tag);
                }

                template<typename tp_self_t>
                [[nodiscard]]
                auto constexpr operator*(this tp_self_t&& p_self)
                noexcept(noexcept(element_segmentation<m_base_iterator_t>{std::forward<tp_self_t>(p_self).m_current.m_element_segmentation}))
                -> element_segmentation<m_base_iterator_t> {
                    return std::forward<tp_self_t>(p_self).m_current.m_element_segmentation;
                }

                [[maybe_unused]]
                auto constexpr operator++()
                -> iterator& {
                    m_current = find_next_element(
                        m_current.m_end,
                        std::ranges::end(m_parent->m_base)
                    );
                    return *this;
                }

                [[nodiscard]]
                auto constexpr operator++(int)
                noexcept(
                    noexcept(++*this) &&
                    noexcept(std::is_nothrow_copy_constructible_v<iterator>)
                )
                requires(std::ranges::forward_range<m_base_view_t>) {
                    auto l_copy = *this;
                    ++*this;
                    return l_copy;
                }

                [[nodiscard]]
                friend
                auto constexpr operator==(
                    const iterator& p_iterator1,
                    const iterator& p_iterator2
                )
                noexcept(noexcept(
                    std::ranges::begin(p_iterator1.m_current.m_element_segmentation.tag) ==
                    std::ranges::begin(p_iterator2.m_current.m_element_segmentation.tag)
                ))
                -> bool
                requires(std::equality_comparable<m_base_iterator_t>) {
                    return
                        std::ranges::begin(p_iterator1.m_current.m_element_segmentation.tag) ==
                        std::ranges::begin(p_iterator2.m_current.m_element_segmentation.tag);
                }

                template<typename tp_self_t>
                [[nodiscard]]
                auto constexpr has_error(this tp_self_t&& p_self)
                noexcept(noexcept(
                    is_iterator_default_initialized(std::ranges::begin(std::forward<tp_self_t>(p_self).m_current.m_element_segmentation.tag)) &&
                    !is_iterator_default_initialized(std::ranges::end(std::forward<tp_self_t>(p_self).m_current.m_element_segmentation.tag))
                ))
                -> bool {
                    return
                        is_iterator_default_initialized(std::ranges::begin(std::forward<tp_self_t>(p_self).m_current.m_element_segmentation.tag)) &&
                        !is_iterator_default_initialized(std::ranges::end(std::forward<tp_self_t>(p_self).m_current.m_element_segmentation.tag));
                }

                template<typename tp_self_t>
                [[nodiscard]]
                auto constexpr get_error_position(this tp_self_t&& p_self)
                noexcept(std::is_nothrow_copy_constructible_v<iterator>)
                -> m_base_iterator_t {
                    return std::ranges::end(std::forward<tp_self_t>(p_self).m_current.m_element_segmentation.tag);
                }
            };

            struct sentinel {
            private:                
                using m_parent_t        = xml_elements_view;
                using m_base_view_t     = tp_view_t;
                using m_base_sentinel_t = std::ranges::iterator_t<m_base_view_t>;

            public:                
                sentinel() = default;

                template<bool tp_is_const2>
                requires(
                    std::sentinel_for<
                        m_base_sentinel_t,
                        std::ranges::iterator_t<
                            const_if_t<
                                tp_is_const2,
                                tp_view_t
                            >
                        >
                    >
                )
                [[nodiscard]]
                friend
                auto constexpr operator==(
                    const iterator<tp_is_const2>& p_iterator,
                    [[maybe_unused]] const sentinel& p_sentinel
                )
                noexcept(noexcept(is_iterator_default_initialized(std::ranges::begin(p_iterator.m_current.m_element_segmentation.tag))))
                -> bool {
                    return is_iterator_default_initialized(std::ranges::begin(p_iterator.m_current.m_element_segmentation.tag));
                }
            };

            auto constexpr static m_can_fetch_const_begin_or_end =
                std::ranges::input_range<const tp_view_t> &&
                !std::ranges::forward_range<const tp_view_t>;

            auto constexpr static m_using_cached_begin = std::ranges::forward_range<tp_view_t>;
            
            tp_view_t m_base;
        public:
            [[no_unique_address]]
            cached_position<tp_view_t> m_cached_begin;
            
            xml_elements_view()
            requires(std::default_initializable<tp_view_t>)
            = default;

            constexpr explicit xml_elements_view(tp_view_t p_base) :
            m_base{std::move(p_base)}
            {}

            [[nodiscard]]
            auto constexpr base()
            const & noexcept
            -> tp_view_t
            requires(std::copy_constructible<tp_view_t>) {
                return m_base;
            }

            [[nodiscard]]
            auto constexpr base()
            && noexcept
            -> tp_view_t {
                return tp_view_t{std::move(m_base)};
            }

            [[nodiscard]]
            auto constexpr begin()
            -> iterator<false> {
                if constexpr (m_using_cached_begin)
                    if (m_cached_begin.has_value())
                        return iterator<false>{
                            this,
                            m_cached_begin.value()
                        };
                auto l_element_segmentation_and_end = find_next_element(
                    std::ranges::begin(m_base),
                    std::ranges::end(m_base)
                );
                if constexpr (m_using_cached_begin)
                    m_cached_begin.emplace(l_element_segmentation_and_end);
                return iterator<false>{
                    this,
                    std::move(l_element_segmentation_and_end)
                };
            }

            [[nodiscard]]
            auto constexpr begin()
            const
            -> iterator<true>
            requires(m_can_fetch_const_begin_or_end) {
                return iterator<true>{
                    this,
                    find_next_element(
                        std::ranges::begin(m_base),
                        std::ranges::end(m_base)
                    )
                };
            }

            [[nodiscard]]
            auto constexpr end()
            noexcept(std::is_nothrow_default_constructible_v<sentinel>) {
                return sentinel{};
            }

            [[nodiscard]]
            auto constexpr end()
            const noexcept(std::is_nothrow_default_constructible_v<sentinel>)
            requires(m_can_fetch_const_begin_or_end) {
                return sentinel{};
            }
        };
        template<typename tp_range_t>
        xml_elements_view(tp_range_t&&) -> xml_elements_view<std::views::all_t<tp_range_t>>;
    }
}

#endif
