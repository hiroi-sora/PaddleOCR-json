// Copyright (c) 2018-2020 configor - Nomango
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
#include "configor_exception.hpp"
#include "configor_stream.hpp"

#include <cmath>    // std::fabs
#include <limits>   // std::numeric_limits
#include <memory>   // std::allocator_traits
#include <utility>  // std::swap

namespace configor
{
namespace detail
{
template <typename _Ty>
bool nearly_equal(_Ty a, _Ty b)
{
    return std::fabs(a - b) < std::numeric_limits<_Ty>::epsilon();
}
}  // namespace detail

enum class config_value_type
{
    number_integer,
    number_float,
    string,
    array,
    object,
    boolean,
    null,
};

inline const char* to_string(config_value_type t) noexcept
{
    switch (t)
    {
    case config_value_type::object:
        return "object";
    case config_value_type::array:
        return "array";
    case config_value_type::string:
        return "string";
    case config_value_type::number_integer:
        return "integer";
    case config_value_type::number_float:
        return "float";
    case config_value_type::boolean:
        return "boolean";
    case config_value_type::null:
        return "null";
    }
    return "unknown";
}

namespace detail
{

//
// config_value
//

template <typename _ConfTy>
struct config_value
{
    using string_type  = typename _ConfTy::string_type;
    using char_type    = typename _ConfTy::char_type;
    using integer_type = typename _ConfTy::integer_type;
    using float_type   = typename _ConfTy::float_type;
    using boolean_type = typename _ConfTy::boolean_type;
    using array_type   = typename _ConfTy::array_type;
    using object_type  = typename _ConfTy::object_type;

    config_value_type type;
    union
    {
        boolean_type boolean;
        integer_type number_integer;
        float_type   number_float;
        string_type* string;
        object_type* object;
        array_type*  vector;
    } data;

    config_value()
        : type(config_value_type::null)
        , data{}
    {
    }

    config_value(const config_value_type t)
    {
        type = t;
        switch (type)
        {
        case config_value_type::object:
            data.object = create<object_type>();
            break;
        case config_value_type::array:
            data.vector = create<array_type>();
            break;
        case config_value_type::string:
            data.string = create<string_type>();
            break;
        case config_value_type::number_integer:
            data.number_integer = integer_type(0);
            break;
        case config_value_type::number_float:
            data.number_float = float_type(0.0);
            break;
        case config_value_type::boolean:
            data.boolean = boolean_type(false);
            break;
        default:
            break;
        }
    }

    config_value(config_value const& other)
        : type(other.type)
    {
        switch (other.type)
        {
        case config_value_type::object:
            data.object = create<object_type>(*other.data.object);
            break;
        case config_value_type::array:
            data.vector = create<array_type>(*other.data.vector);
            break;
        case config_value_type::string:
            data.string = create<string_type>(*other.data.string);
            break;
        case config_value_type::number_integer:
            data.number_integer = other.data.number_integer;
            break;
        case config_value_type::number_float:
            data.number_float = other.data.number_float;
            break;
        case config_value_type::boolean:
            data.boolean = other.data.boolean;
            break;
        default:
            data.object = nullptr;
            break;
        }
    }

    config_value(config_value&& other)
    {
        type              = other.type;
        data              = other.data;
        other.type        = config_value_type::null;
        other.data.object = nullptr;
    }

    ~config_value()
    {
        clear();
    }

    void swap(config_value& other)
    {
        std::swap(type, other.type);
        std::swap(data, other.data);
    }

    void clear()
    {
        switch (type)
        {
        case config_value_type::object:
            destroy<object_type>(data.object);
            break;
        case config_value_type::array:
            destroy<array_type>(data.vector);
            break;
        case config_value_type::string:
            destroy<string_type>(data.string);
            break;
        default:
            break;
        }
    }

    template <typename _Ty, typename... _Args>
    inline _Ty* create(_Args&&... args)
    {
        using allocator_type   = typename _ConfTy::template allocator_type<_Ty>;
        using allocator_traits = std::allocator_traits<allocator_type>;

        allocator_type allocator;

        _Ty* ptr = allocator_traits::allocate(allocator, 1);
        allocator_traits::construct(allocator, ptr, std::forward<_Args>(args)...);
        return ptr;
    }

    template <typename _Ty>
    inline void destroy(_Ty* ptr)
    {
        using allocator_type   = typename _ConfTy::template allocator_type<_Ty>;
        using allocator_traits = std::allocator_traits<allocator_type>;

        allocator_type allocator;
        allocator_traits::destroy(allocator, ptr);
        allocator_traits::deallocate(allocator, ptr, 1);
    }

    inline config_value& operator=(config_value const& other)
    {
        config_value{ other }.swap(*this);
        return (*this);
    }

    inline config_value& operator=(config_value&& other)
    {
        clear();
        type = other.type;
        data = std::move(other.data);
        // invalidate payload
        other.type        = config_value_type::null;
        other.data.object = nullptr;
        return (*this);
    }

    friend bool operator==(const config_value& lhs, const config_value& rhs)
    {
        if (lhs.type == rhs.type)
        {
            switch (lhs.type)
            {
            case config_value_type::array:
                return (*lhs.data.vector == *rhs.data.vector);

            case config_value_type::object:
                return (*lhs.data.object == *rhs.data.object);

            case config_value_type::null:
                return true;

            case config_value_type::string:
                return (*lhs.data.string == *rhs.data.string);

            case config_value_type::boolean:
                return (lhs.data.boolean == rhs.data.boolean);

            case config_value_type::number_integer:
                return (lhs.data.number_integer == rhs.data.number_integer);

            case config_value_type::number_float:
                return detail::nearly_equal(lhs.data.number_float, rhs.data.number_float);

            default:
                return false;
            }
        }
        else if (lhs.type == config_value_type::number_integer && rhs.type == config_value_type::number_float)
        {
            return detail::nearly_equal<float_type>(static_cast<float_type>(lhs.data.number_integer),
                                                    rhs.data.number_float);
        }
        else if (lhs.type == config_value_type::number_float && rhs.type == config_value_type::number_integer)
        {
            return detail::nearly_equal<float_type>(lhs.data.number_float,
                                                    static_cast<float_type>(rhs.data.number_integer));
        }
        return false;
    }
};

namespace
{
inline configor_type_error make_conversion_error(config_value_type t, config_value_type want)
{
    fast_ostringstream ss;
    ss << "cannot convert type '" << to_string(t) << "' to type '" << to_string(want) << "' (implicitly)";
    return configor_type_error(ss.str());
}
}  // namespace
}  // namespace detail

}  // namespace configor
