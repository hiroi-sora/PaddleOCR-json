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
#include "configor_encoding.hpp"
#include "configor_stream.hpp"
#include "configor_token.hpp"
#include "configor_value.hpp"

#include <initializer_list>  // std::initializer_list
#include <ios>               // std::noskipws
#include <istream>           // std::basic_istream
#include <streambuf>         // std::basic_streambuf
#include <type_traits>       // std::char_traits

namespace configor
{

namespace detail
{

template <typename _ConfTy>
class basic_reader
{
public:
    using integer_type = typename _ConfTy::integer_type;
    using float_type   = typename _ConfTy::float_type;
    using char_type    = typename _ConfTy::char_type;
    using string_type  = typename _ConfTy::string_type;
    using encoder_type = encoding::encoder<char_type>;

    virtual void source(std::basic_istream<char_type>& is, encoding::decoder<char_type> src_decoder,
                        encoding::encoder<char_type> target_encoder) = 0;

    virtual token_type scan() = 0;

    virtual void get_integer(integer_type& out) = 0;
    virtual void get_float(float_type& out)     = 0;
    virtual void get_string(string_type& out)   = 0;

    virtual error_handler* get_error_handler() = 0;
};

template <typename _ConfTy, typename... _Args>
struct can_parse
{
private:
    using parser_type  = typename _ConfTy::template parser<>;
    using char_type    = typename _ConfTy::char_type;
    using istream_type = std::basic_istream<char_type>;

    template <typename _UTy, typename... _UArgs>
    using parse_fn = decltype(_UTy::parse(std::declval<_UArgs>()...));

public:
    static constexpr bool value = is_detected<parse_fn, parser_type, _ConfTy&, istream_type&, _Args...>::value;
};

template <typename _ConfTy, template <typename> class _SourceEncoding, template <typename> class _TargetEncoding>
class parser
{
public:
    using config_type     = _ConfTy;
    using char_type       = typename _ConfTy::char_type;
    using string_type     = typename _ConfTy::string_type;
    using reader_type     = typename _ConfTy::reader;
    using istream_type    = std::basic_istream<char_type>;
    using source_encoding = _SourceEncoding<char_type>;
    using target_encoding = _TargetEncoding<char_type>;

    parser() = default;

    template <typename... _ReaderArgs, typename _ReaderTy = reader_type,
              typename = typename std::enable_if<std::is_constructible<_ReaderTy, _ReaderArgs...>::value>::type>
    static void parse(config_type& c, istream_type& is, _ReaderArgs&&... args)
    {
        _ReaderTy r{ std::forward<_ReaderArgs>(args)... };
        return parser{}.do_parse(c, r, is);
    }

private:
    void do_parse(config_type& c, reader_type& reader, istream_type& is)
    {
        try
        {
            reader.source(is, source_encoding::decode, target_encoding::encode);

            do_parse(c, reader, token_type::uninitialized);
            if (reader.scan() != token_type::end_of_input)
                fail(token_type::end_of_input);
        }
        catch (...)
        {
            auto eh = reader.get_error_handler();
            if (eh)
                eh->handle(std::current_exception());
            else
                throw;
        }
    }

    void do_parse(config_type& c, reader_type& reader, token_type last_token, bool read_next = true)
    {
        using string_type = typename _ConfTy::string_type;

        token_type token = last_token;
        if (read_next)
        {
            token = reader.scan();
        }

        switch (token)
        {
        case token_type::literal_true:
            c = true;
            break;

        case token_type::literal_false:
            c = false;
            break;

        case token_type::literal_null:
            c = config_value_type::null;
            break;

        case token_type::value_string:
            c = config_value_type::string;
            reader.get_string(*c.raw_value().data.string);
            break;

        case token_type::value_integer:
            c = config_value_type::number_integer;
            reader.get_integer(c.raw_value().data.number_integer);
            break;

        case token_type::value_float:
            c = config_value_type::number_float;
            reader.get_float(c.raw_value().data.number_float);
            break;

        case token_type::begin_array:
            c = config_value_type::array;
            while (true)
            {
                token = reader.scan();

                bool is_end = false;
                switch (token)
                {
                case token_type::literal_true:
                case token_type::literal_false:
                case token_type::literal_null:
                case token_type::value_string:
                case token_type::value_integer:
                case token_type::value_float:
                case token_type::begin_array:
                case token_type::begin_object:
                    break;
                default:
                    is_end = true;
                    break;
                }
                if (is_end)
                    break;

                c.raw_value().data.vector->push_back(_ConfTy());
                do_parse(c.raw_value().data.vector->back(), reader, token, false);

                // read ','
                token = reader.scan();
                if (token != token_type::value_separator)
                    break;
            }
            if (token != token_type::end_array)
                fail(token, token_type::end_array);
            break;

        case token_type::begin_object:
            c = config_value_type::object;
            while (true)
            {
                token = reader.scan();
                if (token != token_type::value_string)
                    break;

                string_type key{};
                reader.get_string(key);

                token = reader.scan();
                if (token != token_type::name_separator)
                    break;

                _ConfTy object;
                do_parse(object, reader, token);
                c.raw_value().data.object->emplace(key, object);

                // read ','
                token = reader.scan();
                if (token != token_type::value_separator)
                    break;
            }
            if (token != token_type::end_object)
                fail(token, token_type::end_object);
            break;

        default:
            fail(token);
            break;
        }
    }

    void fail(token_type actual_token, const std::string& msg = "unexpected token")
    {
        detail::fast_ostringstream ss;
        ss << msg << " '" << to_string(actual_token) << "'";
        throw configor_deserialization_error(ss.str());
    }

    void fail(token_type actual_token, token_type expected_token, const std::string& msg = "unexpected token")
    {
        fail(actual_token, msg + ", expect '" + to_string(expected_token) + "', but got");
    }
};

}  // namespace detail

}  // namespace configor
