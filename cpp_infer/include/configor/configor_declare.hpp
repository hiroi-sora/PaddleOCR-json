// Copyright (c) 2018-2021 configor - Nomango
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once
#include "configor_encoding.hpp"

#include <cstdint>      // std::int64_t
#include <map>          // std::map
#include <string>       // std::string
#include <type_traits>  // std::false_type, std::true_type, std::remove_cv, std::remove_reference
#include <vector>       // std::vector

namespace configor
{

//
// forward declare
//

template <typename _Ty>
class config_binder;

namespace detail
{
template <typename _ConfTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
class parser;

template <typename _ConfTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
class serializer;

struct nonesuch
{
    nonesuch()                 = delete;
    ~nonesuch()                = delete;
    nonesuch(nonesuch const&)  = delete;
    nonesuch(nonesuch const&&) = delete;
    void operator=(nonesuch const&) = delete;
    void operator=(nonesuch&&) = delete;
};
}  // namespace detail

struct config_args
{
    using boolean_type = bool;

    using integer_type = int64_t;

    using float_type = double;

    using char_type = char;

    template <class _CharTy, class... _Args>
    using string_type = std::basic_string<_CharTy, _Args...>;

    template <class _Kty, class... _Args>
    using array_type = std::vector<_Kty, _Args...>;

    template <class _Kty, class _Ty, class... _Args>
    using object_type = std::map<_Kty, _Ty, _Args...>;

    template <class _Ty>
    using allocator_type = std::allocator<_Ty>;

    template <class _ConfTy>
    using reader_type = detail::nonesuch;

    template <typename _ConfTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
    using parser_type = detail::parser<_ConfTy, _SourceEncoding, _TargetEncoding>;

    template <class _ConfTy>
    using writer_type = detail::nonesuch;

    template <typename _ConfTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
    using serializer_type = detail::serializer<_ConfTy, _SourceEncoding, _TargetEncoding>;

    template <class _Ty>
    using binder_type = config_binder<_Ty>;

    template <typename _CharTy>
    using default_encoding = encoding::ignore<_CharTy>;
};

struct wconfig_args : config_args
{
    using char_type = wchar_t;
};

template <typename _Args = config_args>
class basic_config;

//
// is_config
//

template <typename>
struct is_config : std::false_type
{
};

template <typename _Args>
struct is_config<basic_config<_Args>> : std::true_type
{
};

namespace detail
{

// priority

template <int p>
struct priority : priority<p - 1>
{
};

template <>
struct priority<0>
{
};

// remove_cvref

template <typename _Ty>
struct remove_cvref
{
    using type = typename std::remove_cv<typename std::remove_reference<_Ty>::type>::type;
};

// always_void

template <typename _Ty>
struct always_void
{
    using type = void;
};

// exact_detect

template <class _Void, template <class...> class _Op, class... _Args>
struct detect_impl
{
    struct dummy
    {
        dummy()             = delete;
        ~dummy()            = delete;
        dummy(const dummy&) = delete;
    };

    using type = dummy;

    static constexpr bool value = false;
};

template <template <class...> class _Op, class... _Args>
struct detect_impl<typename always_void<_Op<_Args...>>::type, _Op, _Args...>
{
    using type = _Op<_Args...>;

    static constexpr bool value = true;
};

template <class _Expected, template <class...> class _Op, class... _Args>
using exact_detect = std::is_same<_Expected, typename detect_impl<void, _Op, _Args...>::type>;

template <template <class...> class _Op, class... _Args>
using is_detected = std::integral_constant<bool, detect_impl<void, _Op, _Args...>::value>;

// static_const

template <typename _Ty>
struct static_const
{
    static constexpr _Ty value = {};
};

template <typename _Ty>
constexpr _Ty static_const<_Ty>::value;

}  // namespace detail

}  // namespace configor
