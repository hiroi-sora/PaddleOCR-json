// Copyright (c) 2018-2020 configor - Nomango
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// is the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included is
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
#include "configor_conversion.hpp"
#include "configor_declare.hpp"
#include "configor_iterator.hpp"
#include "configor_parser.hpp"
#include "configor_serializer.hpp"
#include "configor_value.hpp"

#include <sstream>      // std::stringstream
#include <algorithm>    // std::for_each, std::all_of
#include <type_traits>  // std::enable_if, std::is_same, std::is_integral, std::is_floating_point
#include <utility>      // std::forward, std::declval

namespace configor
{

template <typename _Args>
class basic_config
{
    friend struct detail::iterator<basic_config>;
    friend struct detail::iterator<const basic_config>;

public:
    template <typename _Ty>
    using allocator_type = typename _Args::template allocator_type<_Ty>;
    using boolean_type   = typename _Args::boolean_type;
    using integer_type   = typename _Args::integer_type;
    using float_type     = typename _Args::float_type;
    using char_type      = typename _Args::char_type;
    using string_type =
        typename _Args::template string_type<char_type, std::char_traits<char_type>, allocator_type<char_type>>;
    using array_type = typename _Args::template array_type<basic_config, allocator_type<basic_config>>;
    using object_type =
        typename _Args::template object_type<string_type, basic_config, std::less<string_type>,
                                             allocator_type<std::pair<const string_type, basic_config>>>;

    using value_type = detail::config_value<basic_config>;

    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    using iterator               = detail::iterator<basic_config>;
    using const_iterator         = detail::iterator<const basic_config>;
    using reverse_iterator       = detail::reverse_iterator<iterator>;
    using const_reverse_iterator = detail::reverse_iterator<const_iterator>;

    template <typename _Ty>
    using binder_type = typename _Args::template binder_type<_Ty>;

    template <typename _CharTy>
    using default_encoding = typename _Args::template default_encoding<_CharTy>;

    using reader = typename _Args::template reader_type<basic_config>;
    using writer = typename _Args::template writer_type<basic_config>;

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding>
    using parser = typename _Args::template parser_type<basic_config, _SourceEncoding, _TargetEncoding>;

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding>
    using serializer = typename _Args::template serializer_type<basic_config, _SourceEncoding, _TargetEncoding>;

public:
    basic_config(std::nullptr_t = nullptr) {}

    basic_config(const config_value_type type)
        : value_(type)
    {
    }

    basic_config(const basic_config& other)
        : value_(other.value_)
    {
    }

    basic_config(basic_config&& other) noexcept
        : value_(std::move(other.value_))
    {
        // invalidate payload
        other.value_.type        = config_value_type::null;
        other.value_.data.object = nullptr;
    }

    basic_config(const std::initializer_list<basic_config>& init_list,
                 config_value_type                          exact_type = config_value_type::null)
    {
        bool is_an_object = std::all_of(init_list.begin(), init_list.end(),
                                        [](const basic_config& config)
                                        { return (config.is_array() && config.size() == 2 && config[0].is_string()); });

        if (exact_type != config_value_type::object && exact_type != config_value_type::array)
        {
            exact_type = is_an_object ? config_value_type::object : config_value_type::array;
        }

        if (exact_type == config_value_type::object)
        {
            if (!is_an_object)
                throw configor_type_error("initializer_list is not object type");

            value_ = config_value_type::object;
            std::for_each(init_list.begin(), init_list.end(),
                          [this](const basic_config& config)
                          {
                              value_.data.object->emplace(*((*config.value_.data.vector)[0].value_.data.string),
                                                          (*config.value_.data.vector)[1]);
                          });
        }
        else
        {
            value_ = config_value_type::array;
            value_.data.vector->reserve(init_list.size());
            value_.data.vector->assign(init_list.begin(), init_list.end());
        }
    }

    template <typename _CompatibleTy, typename _UTy = typename detail::remove_cvref<_CompatibleTy>::type,
              typename std::enable_if<!is_config<_UTy>::value && detail::has_to_config<basic_config, _UTy>::value,
                                      int>::type = 0>
    basic_config(_CompatibleTy&& value)
    {
        binder_type<_UTy>::to_config(*this, std::forward<_CompatibleTy>(value));
    }

    static inline basic_config object(const std::initializer_list<basic_config>& init_list)
    {
        return basic_config(init_list, config_value_type::object);
    }

    static inline basic_config array(const std::initializer_list<basic_config>& init_list)
    {
        return basic_config(init_list, config_value_type::array);
    }

    inline bool is_object() const
    {
        return value_.type == config_value_type::object;
    }

    inline bool is_array() const
    {
        return value_.type == config_value_type::array;
    }

    inline bool is_string() const
    {
        return value_.type == config_value_type::string;
    }

    inline bool is_bool() const
    {
        return value_.type == config_value_type::boolean;
    }

    inline bool is_integer() const
    {
        return value_.type == config_value_type::number_integer;
    }

    inline bool is_float() const
    {
        return value_.type == config_value_type::number_float;
    }

    inline bool is_number() const
    {
        return is_integer() || is_float();
    }

    inline bool is_null() const
    {
        return value_.type == config_value_type::null;
    }

    inline config_value_type type() const
    {
        return value_.type;
    }

    inline const char* type_name() const
    {
        return to_string(type());
    }

    inline void swap(basic_config& rhs)
    {
        value_.swap(rhs.value_);
    }

public:
    inline iterator begin()
    {
        iterator iter(this);
        iter.set_begin();
        return iter;
    }
    inline const_iterator begin() const
    {
        return cbegin();
    }
    inline const_iterator cbegin() const
    {
        const_iterator iter(this);
        iter.set_begin();
        return iter;
    }
    inline iterator end()
    {
        iterator iter(this);
        iter.set_end();
        return iter;
    }
    inline const_iterator end() const
    {
        return cend();
    }
    inline const_iterator cend() const
    {
        const_iterator iter(this);
        iter.set_end();
        return iter;
    }
    inline reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }
    inline const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }
    inline const_reverse_iterator crbegin() const
    {
        return rbegin();
    }
    inline reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }
    inline const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }
    inline const_reverse_iterator crend() const
    {
        return rend();
    }

public:
    inline size_type size() const
    {
        switch (type())
        {
        case config_value_type::null:
            return 0;
        case config_value_type::array:
            return value_.data.vector->size();
        case config_value_type::object:
            return value_.data.object->size();
        default:
            return 1;
        }
    }

    inline bool empty() const
    {
        if (is_null())
            return true;

        if (is_object())
            return value_.data.object->empty();

        if (is_array())
            return value_.data.vector->empty();

        return false;
    }

    inline iterator find(const typename object_type::key_type& key)
    {
        if (is_object())
        {
            iterator iter(this);
            iter.object_it_ = value_.data.object->find(key);
            return iter;
        }
        return end();
    }

    inline const_iterator find(const typename object_type::key_type& key) const
    {
        if (is_object())
        {
            const_iterator iter(this);
            iter.object_it_ = value_.data.object->find(key);
            return iter;
        }
        return cend();
    }

    inline size_type count(const typename object_type::key_type& key) const
    {
        return is_object() ? value_.data.object->count(key) : 0;
    }

    inline size_type erase(const typename object_type::key_type& key)
    {
        if (!is_object())
        {
            throw configor_invalid_key("cannot use erase() with non-object value");
        }
        return value_.data.object->erase(key);
    }

    inline void erase(const size_type index)
    {
        if (!is_array())
        {
            throw configor_invalid_key("cannot use erase() with non-array value");
        }
        value_.data.vector->erase(value_.data.vector->begin() + static_cast<difference_type>(index));
    }

    template <class _IterTy, typename = typename std::enable_if<std::is_same<_IterTy, iterator>::value
                                                                || std::is_same<_IterTy, const_iterator>::value>::type>
    _IterTy erase(_IterTy pos)
    {
        _IterTy result = end();

        switch (type())
        {
        case config_value_type::object:
        {
            result.object_it_ = value_.data.object->erase(pos.object_it_);
            break;
        }

        case config_value_type::array:
        {
            result.array_it_ = value_.data.vector->erase(pos.array_it_);
            break;
        }

        default:
            throw configor_invalid_iterator("cannot use erase() with non-object & non-array value");
        }
        return result;
    }

    template <class _IterTy, typename = typename std::enable_if<std::is_same<_IterTy, iterator>::value
                                                                || std::is_same<_IterTy, const_iterator>::value>::type>
    inline _IterTy erase(_IterTy first, _IterTy last)
    {
        _IterTy result = end();

        switch (type())
        {
        case config_value_type::object:
        {
            result.object_it_ = value_.data.object->erase(first.object_it_, last.object_it_);
            break;
        }

        case config_value_type::array:
        {
            result.array_it_ = value_.data.vector->erase(first.array_it_, last.array_it_);
            break;
        }

        default:
            throw configor_invalid_iterator("cannot use erase() with non-object & non-array value");
        }
        return result;
    }

    inline void push_back(basic_config&& config)
    {
        if (!is_null() && !is_array())
        {
            throw configor_type_error("cannot use push_back() with non-array value");
        }

        if (is_null())
        {
            value_ = config_value_type::array;
        }

        value_.data.vector->push_back(std::move(config));
    }

    inline basic_config& operator+=(basic_config&& config)
    {
        push_back(std::move(config));
        return (*this);
    }

    inline void clear()
    {
        switch (type())
        {
        case config_value_type::number_integer:
        {
            value_.data.number_integer = 0;
            break;
        }

        case config_value_type::number_float:
        {
            value_.data.number_float = static_cast<float_type>(0.0);
            break;
        }

        case config_value_type::boolean:
        {
            value_.data.boolean = false;
            break;
        }

        case config_value_type::string:
        {
            value_.data.string->clear();
            break;
        }

        case config_value_type::array:
        {
            value_.data.vector->clear();
            break;
        }

        case config_value_type::object:
        {
            value_.data.object->clear();
            break;
        }

        default:
            break;
        }
    }

private:
    // get pointer value

    inline boolean_type* do_get_ptr(boolean_type*) noexcept
    {
        return is_bool() ? &value_.data.boolean : nullptr;
    }

    inline const boolean_type* do_get_ptr(const boolean_type*) const noexcept
    {
        return is_bool() ? &value_.data.boolean : nullptr;
    }

    inline integer_type* do_get_ptr(integer_type*) noexcept
    {
        return is_integer() ? &value_.data.number_integer : nullptr;
    }

    inline const integer_type* do_get_ptr(const integer_type*) const noexcept
    {
        return is_integer() ? &value_.data.number_integer : nullptr;
    }

    inline float_type* do_get_ptr(float_type*) noexcept
    {
        return is_float() ? &value_.data.number_float : nullptr;
    }

    inline const float_type* do_get_ptr(const float_type*) const noexcept
    {
        return is_float() ? &value_.data.number_float : nullptr;
    }

    inline string_type* do_get_ptr(string_type*) noexcept
    {
        return is_string() ? value_.data.string : nullptr;
    }

    inline const string_type* do_get_ptr(const string_type*) const noexcept
    {
        return is_string() ? value_.data.string : nullptr;
    }

    inline array_type* do_get_ptr(array_type*) noexcept
    {
        return is_array() ? value_.data.vector : nullptr;
    }

    inline const array_type* do_get_ptr(const array_type*) const noexcept
    {
        return is_array() ? value_.data.vector : nullptr;
    }

    inline object_type* do_get_ptr(object_type*) noexcept
    {
        return is_object() ? value_.data.object : nullptr;
    }

    inline const object_type* do_get_ptr(const object_type*) const noexcept
    {
        return is_object() ? value_.data.object : nullptr;
    }

public:
    // get_ptr

    template <typename _Ty, typename = typename std::enable_if<std::is_pointer<_Ty>::value>::type>
    auto get_ptr() const -> decltype(std::declval<const basic_config&>().do_get_ptr(std::declval<_Ty>()))
    {
        return do_get_ptr(static_cast<_Ty>(nullptr));
    }

    template <typename _Ty, typename = typename std::enable_if<std::is_pointer<_Ty>::value>::type>
    auto get_ptr() -> decltype(std::declval<basic_config&>().do_get_ptr(std::declval<_Ty>()))
    {
        return do_get_ptr(static_cast<_Ty>(nullptr));
    }

private:
    // get reference value
    template <typename _RefTy, typename _ConfTy, typename _PtrTy = typename std::add_pointer<_RefTy>::type>
    static auto do_get_ref(_ConfTy& c) -> decltype(c.template get_ptr<_PtrTy>(), std::declval<_RefTy>())
    {
        auto* ptr = c.template get_ptr<_PtrTy>();
        if (ptr != nullptr)
        {
            return *ptr;
        }
        throw configor_type_error("incompatible reference type for get, actual type is " + std::string(c.type_name()));
    }

    // get value

    template <typename _Ty, typename = typename std::enable_if<std::is_same<basic_config, _Ty>::value>::type>
    _Ty do_get(detail::priority<3>) const
    {
        return *this;
    }

    template <typename _Ty, typename = typename std::enable_if<std::is_pointer<_Ty>::value>::type>
    auto do_get(detail::priority<2>) const noexcept
        -> decltype(std::declval<const basic_config&>().do_get_ptr(std::declval<_Ty>()))
    {
        return do_get_ptr(static_cast<_Ty>(nullptr));
    }

    template <typename _Ty,
              typename = typename std::enable_if<detail::has_non_default_from_config<basic_config, _Ty>::value>::type>
    _Ty do_get(detail::priority<1>) const
    {
        return binder_type<_Ty>::from_config(*this);
    }

    template <typename _Ty,
              typename = typename std::enable_if<std::is_default_constructible<_Ty>::value
                                                 && detail::has_from_config<basic_config, _Ty>::value>::type>
    _Ty do_get(detail::priority<0>) const
    {
        _Ty value{};
        binder_type<_Ty>::from_config(*this, value);
        return value;
    }

    // get value by reference

    template <typename _Ty, typename = typename std::enable_if<std::is_same<basic_config, _Ty>::value>::type>
    _Ty& do_get(_Ty& value, detail::priority<3>) const
    {
        return (value = *this);
    }

    template <typename _Ty, typename = typename std::enable_if<std::is_pointer<_Ty>::value>::type>
    auto do_get(_Ty& value, detail::priority<2>) const noexcept
        -> decltype(std::declval<const basic_config&>().do_get_ptr(std::declval<_Ty>()))
    {
        return (value = do_get_ptr(static_cast<_Ty>(nullptr)));
    }

    template <typename _Ty, typename = typename std::enable_if<detail::has_from_config<basic_config, _Ty>::value>::type>
    _Ty& do_get(_Ty& value, detail::priority<1>) const
    {
        binder_type<_Ty>::from_config(*this, value);
        return value;
    }

    template <typename _Ty,
              typename = typename std::enable_if<detail::has_non_default_from_config<basic_config, _Ty>::value>::type>
    _Ty& do_get(_Ty& value, detail::priority<0>) const
    {
        value = binder_type<_Ty>::from_config(*this);
        return value;
    }

public:
    // get

    template <typename _Ty, typename _UTy = typename detail::remove_cvref<_Ty>::type,
              typename = typename std::enable_if<!std::is_reference<_Ty>::value>::type>
    auto get() const -> decltype(std::declval<const basic_config&>().template do_get<_UTy>(detail::priority<3>{}))
    {
        return do_get<_UTy>(detail::priority<3>{});
    }

    template <typename _Ty, typename = typename std::enable_if<std::is_pointer<_Ty>::value>::type>
    auto get() -> decltype(std::declval<basic_config&>().template get_ptr<_Ty>())
    {
        return get_ptr<_Ty>();
    }

    template <typename _Ty, typename std::enable_if<std::is_reference<_Ty>::value, int>::type = 0>
    auto get() -> decltype(basic_config::template do_get_ref<_Ty>(std::declval<basic_config&>()))
    {
        return do_get_ref<_Ty>(*this);
    }

    template <typename _Ty,
              typename std::enable_if<std::is_reference<_Ty>::value
                                          && std::is_const<typename std::remove_reference<_Ty>::type>::value,
                                      int>::type = 0>
    auto get() const -> decltype(basic_config::template do_get_ref<_Ty>(std::declval<const basic_config&>()))
    {
        return do_get_ref<_Ty>(*this);
    }

    template <typename _Ty>
    auto get(_Ty& value) const
        -> decltype(std::declval<const basic_config&>().template do_get<_Ty>(std::declval<_Ty&>(),
                                                                             detail::priority<3>{}),
                    bool{})
    {
        try
        {
            do_get<_Ty>(value, detail::priority<3>{});
            return true;
        }
        catch (...)
        {
        }
        return false;
    }

    template <typename _Ty, typename = typename std::enable_if<detail::is_configor_getable<basic_config, _Ty>::value
                                                               && !std::is_pointer<_Ty>::value
                                                               && !std::is_reference<_Ty>::value>::type>
    inline operator _Ty() const
    {
        return get<_Ty>();
    }

    boolean_type as_bool() const
    {
        switch (type())
        {
        case config_value_type::number_integer:
            return value_.data.number_integer != 0;
        case config_value_type::number_float:
            return value_.data.number_float != 0;
        case config_value_type::string:
            return !(*value_.data.string).empty();
        case config_value_type::array:
        case config_value_type::object:
            return !empty();
        case config_value_type::boolean:
            return value_.data.boolean;
        case config_value_type::null:
            break;
        }
        return false;
    }

    integer_type as_integer() const
    {
        switch (type())
        {
        case config_value_type::number_integer:
            return value_.data.number_integer;
        case config_value_type::number_float:
            return static_cast<integer_type>(value_.data.number_float);
        case config_value_type::boolean:
            return value_.data.boolean ? integer_type(1) : integer_type(0);
        case config_value_type::string:
        case config_value_type::array:
        case config_value_type::object:
        case config_value_type::null:
            throw detail::make_conversion_error(type(), config_value_type::number_integer);
        }
        return 0;
    }

    float_type as_float() const
    {
        switch (type())
        {
        case config_value_type::number_integer:
            return static_cast<float_type>(value_.data.number_integer);
        case config_value_type::number_float:
            return value_.data.number_float;
        case config_value_type::boolean:
            return value_.data.boolean ? float_type(1) : float_type(0);
        case config_value_type::string:
        case config_value_type::array:
        case config_value_type::object:
        case config_value_type::null:
            throw detail::make_conversion_error(type(), config_value_type::number_float);
        }
        return 0;
    }

    string_type as_string() const
    {
        switch (type())
        {
        case config_value_type::string:
            return *value_.data.string;
        case config_value_type::number_integer:
        {
            std::basic_stringstream<char_type> ss;
            ss << detail::serialize_integer(value_.data.number_integer);
            return ss.str();
        }
        case config_value_type::number_float:
        {
            std::basic_stringstream<char_type> ss;
            ss << std::defaultfloat << detail::serialize_float(value_.data.number_float);
            return ss.str();
        }
        case config_value_type::boolean:
        case config_value_type::array:
        case config_value_type::object:
            throw detail::make_conversion_error(type(), config_value_type::string);
        case config_value_type::null:
            break;
        }
        return string_type{};
    }

public:
    // operator= functions

    inline basic_config& operator=(basic_config rhs)
    {
        swap(rhs);
        return (*this);
    }

public:
    // operator[] functions

    inline basic_config& operator[](const size_type index)
    {
        if (is_null())
        {
            value_ = config_value_type::array;
        }

        if (!is_array())
        {
            throw configor_invalid_key("operator[] called on a non-array object");
        }

        if (index >= value_.data.vector->size())
        {
            value_.data.vector->insert(value_.data.vector->end(), index - value_.data.vector->size() + 1,
                                       basic_config());
        }
        return (*value_.data.vector)[index];
    }

    inline basic_config& operator[](const size_type index) const
    {
        return at(index);
    }

    inline basic_config& operator[](const typename object_type::key_type& key)
    {
        if (is_null())
        {
            value_ = config_value_type::object;
        }

        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object type");
        }
        return (*value_.data.object)[key];
    }

    inline basic_config& operator[](const typename object_type::key_type& key) const
    {
        return at(key);
    }

    inline basic_config& at(const size_type index) const
    {
        if (!is_array())
        {
            throw configor_invalid_key("operator[] called on a non-array type");
        }

        if (index >= value_.data.vector->size())
        {
            throw std::out_of_range("operator[] index out of range");
        }
        return (*value_.data.vector)[index];
    }

    template <typename _CharTy>
    inline basic_config& operator[](_CharTy* key)
    {
        if (is_null())
        {
            value_ = config_value_type::object;
        }

        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object object");
        }
        return (*value_.data.object)[key];
    }

    template <typename _CharTy>
    inline basic_config& operator[](_CharTy* key) const
    {
        return at(key);
    }

    inline basic_config& at(const typename object_type::key_type& key) const
    {
        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object object");
        }

        auto iter = value_.data.object->find(key);
        if (iter == value_.data.object->end())
        {
            throw std::out_of_range("operator[] key out of range");
        }
        return iter->second;
    }

    template <typename _CharTy>
    inline basic_config& at(_CharTy* key) const
    {
        if (!is_object())
        {
            throw configor_invalid_key("operator[] called on a non-object object");
        }

        auto iter = value_.data.object->find(key);
        if (iter == value_.data.object->end())
        {
            throw std::out_of_range("operator[] key out of range");
        }
        return iter->second;
    }

public:
    // dump to stream
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _DumpArgs,
              typename = typename std::enable_if<detail::can_serialize<basic_config, _DumpArgs...>::value>::type>
    void dump(std::basic_ostream<char_type>& os, _DumpArgs&&... args) const
    {
        serializer<_SourceEncoding, _TargetEncoding>::dump(*this, os, std::forward<_DumpArgs>(args)...);
    }

    // dump to string
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _DumpArgs,
              typename = typename std::enable_if<detail::can_serialize<basic_config, _DumpArgs...>::value>::type>
    void dump(string_type& str, _DumpArgs&&... args) const
    {
        detail::fast_string_ostreambuf<char_type> buf{ str };
        std::basic_ostream<char_type>             os{ &buf };
        return dump<_SourceEncoding, _TargetEncoding>(os, std::forward<_DumpArgs>(args)...);
    }

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _DumpArgs,
              typename = typename std::enable_if<detail::can_serialize<basic_config, _DumpArgs...>::value>::type>
    string_type dump(_DumpArgs&&... args) const
    {
        string_type result;
        dump<_SourceEncoding, _TargetEncoding>(result, std::forward<_DumpArgs>(args)...);
        return result;
    }

    // parse from stream
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename = typename std::enable_if<detail::can_parse<basic_config, _ParserArgs...>::value>::type>
    static void parse(basic_config& c, std::basic_istream<char_type>& is, _ParserArgs&&... args)
    {
        parser<_SourceEncoding, _TargetEncoding>::parse(c, is, std::forward<_ParserArgs>(args)...);
    }

    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename = typename std::enable_if<detail::can_parse<basic_config, _ParserArgs...>::value>::type>
    static basic_config parse(std::basic_istream<char_type>& is, _ParserArgs&&... args)
    {
        basic_config c;
        parse<_SourceEncoding, _TargetEncoding>(c, is, std::forward<_ParserArgs>(args)...);
        return c;
    }

    // parse from string
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename = typename std::enable_if<detail::can_parse<basic_config, _ParserArgs...>::value>::type>
    static basic_config parse(const string_type& str, _ParserArgs&&... args)
    {
        detail::fast_string_istreambuf<char_type> buf{ str };
        std::basic_istream<char_type>             is{ &buf };
        return parse<_SourceEncoding, _TargetEncoding>(is, std::forward<_ParserArgs>(args)...);
    }

    // parse from c-style string
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename = typename std::enable_if<detail::can_parse<basic_config, _ParserArgs...>::value>::type>
    static basic_config parse(const char_type* str, _ParserArgs&&... args)
    {
        detail::fast_buffer_istreambuf<char_type> buf{ str };
        std::basic_istream<char_type>             is{ &buf };
        return parse<_SourceEncoding, _TargetEncoding>(is, std::forward<_ParserArgs>(args)...);
    }

    // parse from c-style file
    template <template <typename> class _SourceEncoding = default_encoding,
              template <typename> class _TargetEncoding = _SourceEncoding, typename... _ParserArgs,
              typename = typename std::enable_if<detail::can_parse<basic_config, _ParserArgs...>::value>::type>
    static basic_config parse(std::FILE* file, _ParserArgs&&... args)
    {
        detail::fast_cfile_istreambuf<char_type> buf{ file };
        std::basic_istream<char_type>            is{ &buf };
        return parse<_SourceEncoding, _TargetEncoding>(is, std::forward<_ParserArgs>(args)...);
    }

    // wrap

    template <typename _Ty, typename = typename std::enable_if<!is_config<_Ty>::value
                                                               && detail::is_configor_getable<basic_config, _Ty>::value
                                                               && !std::is_pointer<_Ty>::value>::type>
    static inline detail::write_configor_wrapper<basic_config, _Ty> wrap(_Ty& v)
    {
        return detail::write_configor_wrapper<basic_config, _Ty>(v);
    }

    template <typename _Ty,
              typename = typename std::enable_if<!std::is_same<basic_config, _Ty>::value
                                                 && detail::has_to_config<basic_config, _Ty>::value>::type>
    static inline detail::read_configor_wrapper<basic_config, _Ty> wrap(const _Ty& v)
    {
        return detail::read_configor_wrapper<basic_config, _Ty>(v);
    }

public:
    // eq functions

    friend inline bool operator==(const basic_config& lhs, const basic_config& rhs)
    {
        return lhs.value_ == rhs.value_;
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator==(const basic_config& lhs, _ScalarTy rhs)
    {
        return lhs == basic_config(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator==(_ScalarTy lhs, const basic_config& rhs)
    {
        return basic_config(lhs) == rhs;
    }

    // ne functions

    friend inline bool operator!=(const basic_config& lhs, const basic_config& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator!=(const basic_config& lhs, _ScalarTy rhs)
    {
        return lhs != basic_config(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator!=(_ScalarTy lhs, const basic_config& rhs)
    {
        return basic_config(lhs) != rhs;
    }

    // lt functions

    friend bool operator<(const basic_config& lhs, const basic_config& rhs)
    {
        const auto lhs_type = lhs.type();
        const auto rhs_type = rhs.type();

        if (lhs_type == rhs_type)
        {
            switch (lhs_type)
            {
            case config_value_type::array:
                return (*lhs.value_.data.vector) < (*rhs.value_.data.vector);

            case config_value_type::object:
                return (*lhs.value_.data.object) < (*rhs.value_.data.object);

            case config_value_type::null:
                return false;

            case config_value_type::string:
                return (*lhs.value_.data.string) < (*rhs.value_.data.string);

            case config_value_type::boolean:
                return (lhs.value_.data.boolean < rhs.value_.data.boolean);

            case config_value_type::number_integer:
                return (lhs.value_.data.number_integer < rhs.value_.data.number_integer);

            case config_value_type::number_float:
                return (lhs.value_.data.number_float < rhs.value_.data.number_float);

            default:
                return false;
            }
        }
        else if (lhs_type == config_value_type::number_integer && rhs_type == config_value_type::number_float)
        {
            return (static_cast<float_type>(lhs.value_.data.number_integer) < rhs.value_.data.number_float);
        }
        else if (lhs_type == config_value_type::number_float && rhs_type == config_value_type::number_integer)
        {
            return (lhs.value_.data.number_float < static_cast<float_type>(rhs.value_.data.number_integer));
        }

        return false;
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<(const basic_config& lhs, _ScalarTy rhs)
    {
        return lhs < basic_config(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<(_ScalarTy lhs, const basic_config& rhs)
    {
        return basic_config(lhs) < rhs;
    }

    // lte functions

    friend inline bool operator<=(const basic_config& lhs, const basic_config& rhs)
    {
        return !(rhs < lhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<=(const basic_config& lhs, _ScalarTy rhs)
    {
        return lhs <= basic_config(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator<=(_ScalarTy lhs, const basic_config& rhs)
    {
        return basic_config(lhs) <= rhs;
    }

    // gt functions

    friend inline bool operator>(const basic_config& lhs, const basic_config& rhs)
    {
        return rhs < lhs;
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>(const basic_config& lhs, _ScalarTy rhs)
    {
        return lhs > basic_config(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>(_ScalarTy lhs, const basic_config& rhs)
    {
        return basic_config(lhs) > rhs;
    }

    // gte functions

    friend inline bool operator>=(const basic_config& lhs, const basic_config& rhs)
    {
        return !(lhs < rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>=(const basic_config& lhs, _ScalarTy rhs)
    {
        return lhs >= basic_config(rhs);
    }

    template <typename _ScalarTy, typename = typename std::enable_if<std::is_scalar<_ScalarTy>::value>::type>
    friend inline bool operator>=(_ScalarTy lhs, const basic_config& rhs)
    {
        return basic_config(lhs) >= rhs;
    }

public:
    const value_type& raw_value() const
    {
        return value_;
    }

    value_type& raw_value()
    {
        return value_;
    }

private:
    value_type value_;
};

}  // namespace configor
