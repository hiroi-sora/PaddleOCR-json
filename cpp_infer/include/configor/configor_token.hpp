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

namespace configor
{

enum class token_type
{
    uninitialized,

    literal_true,
    literal_false,
    literal_null,

    value_string,
    value_integer,
    value_float,

    begin_array,
    end_array,

    begin_object,
    end_object,

    name_separator,
    value_separator,

    end_of_input
};

inline const char* to_string(token_type token)
{
    switch (token)
    {
    case token_type::uninitialized:
        return "uninitialized";
    case token_type::literal_true:
        return "literal_true";
    case token_type::literal_false:
        return "literal_false";
    case token_type::literal_null:
        return "literal_null";
    case token_type::value_string:
        return "value_string";
    case token_type::value_integer:
        return "value_integer";
    case token_type::value_float:
        return "value_float";
    case token_type::begin_array:
        return "begin_array";
    case token_type::end_array:
        return "end_array";
    case token_type::begin_object:
        return "begin_object";
    case token_type::end_object:
        return "end_object";
    case token_type::name_separator:
        return "name_separator";
    case token_type::value_separator:
        return "value_separator";
    case token_type::end_of_input:
        return "end_of_input";
    }
    return "unknown";
}

}  // namespace configor
