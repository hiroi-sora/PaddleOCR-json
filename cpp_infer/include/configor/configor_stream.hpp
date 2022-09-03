// Copyright (c) 2021-2022 configor - Nomango
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
#include <algorithm>    // std::none_of
#include <cmath>        // std::ceil, std::floor
#include <cstdio>       // std::FILE, std::fgetc, std::fgets, std::fgetwc, std::fgetws, std::snprintf
#include <iomanip>      // std::fill, std::setw
#include <ios>          // std::basic_ios, std::streamsize, std::hex, std::dec, std::uppercase, std::nouppercase
#include <istream>      // std::basic_istream
#include <ostream>      // std::basic_ostream
#include <streambuf>    // std::streambuf
#include <string>       // std::basic_string
#include <type_traits>  // std::char_traits
#include <utility>      // std::forward

namespace configor
{

//
// basic_oadapter
//

template <typename _CharTy>
struct basic_oadapter
{
    using char_type   = _CharTy;
    using char_traits = std::char_traits<char_type>;

    virtual void write(const char_type ch) = 0;

    virtual void write(const char_type* str, std::size_t size)
    {
        if (size)
        {
            for (std::size_t i = 0; i < size; ++i)
                this->write(str[i]);
        }
    }
};

using oadapter  = basic_oadapter<char>;
using woadapter = basic_oadapter<wchar_t>;

template <typename _CharTy>
class basic_oadapterstream : public std::basic_ostream<_CharTy>
{
public:
    using char_type   = _CharTy;
    using char_traits = std::char_traits<char_type>;
    using int_type    = typename char_traits::int_type;

    explicit basic_oadapterstream(basic_oadapter<char_type>& adapter)
        : std::basic_ostream<char_type>(nullptr)
        , buf_(adapter)
    {
        this->rdbuf(&buf_);
    }

private:
    class streambuf : public std::basic_streambuf<char_type>
    {
    public:
        explicit streambuf(basic_oadapter<char_type>& adapter)
            : adapter_(adapter)
        {
        }

    protected:
        virtual int_type overflow(int_type c) override
        {
            if (c != EOF)
            {
                adapter_.write(char_traits::to_char_type(c));
            }
            return c;
        }

        virtual std::streamsize xsputn(const char_type* s, std::streamsize num) override
        {
            adapter_.write(s, static_cast<size_t>(num));
            return num;
        }

    private:
        basic_oadapter<char_type>& adapter_;
    };

    streambuf buf_;
};

using oadapterstream  = basic_oadapterstream<char>;
using woadapterstream = basic_oadapterstream<wchar_t>;

//
// basic_iadapter
//

template <typename _CharTy>
struct basic_iadapter
{
    using char_type   = _CharTy;
    using char_traits = std::char_traits<char_type>;

    virtual char_type read() = 0;

    virtual void read(char_type* s, size_t num)
    {
        if (num)
        {
            for (std::size_t i = 0; i < num; ++i)
                s[i] = this->read();
        }
    }
};

using iadapter  = basic_iadapter<char>;
using wiadapter = basic_iadapter<wchar_t>;

template <typename _CharTy>
class basic_iadapterstream : public std::basic_istream<_CharTy>
{
public:
    using char_type   = _CharTy;
    using char_traits = std::char_traits<char_type>;
    using int_type    = typename char_traits::int_type;

    explicit basic_iadapterstream(basic_iadapter<char_type>& adapter)
        : std::basic_istream<_CharTy>(nullptr)
        , buf_(adapter)
    {
        this->rdbuf(&buf_);
    }

private:
    class streambuf : public std::basic_streambuf<_CharTy>
    {
    public:
        explicit streambuf(basic_iadapter<char_type>& adapter)
            : adapter_(adapter)
            , last_char_(0)
        {
        }

    protected:
        virtual int_type underflow() override
        {
            if (last_char_ != 0)
                return last_char_;
            last_char_ = char_traits::to_char_type(adapter_.read());
            return last_char_;
        }

        virtual int_type uflow() override
        {
            int_type c = underflow();
            last_char_ = 0;
            return c;
        }

        virtual std::streamsize xsgetn(char_type* s, std::streamsize num) override
        {
            if (last_char_ != 0)
            {
                // read last char
                s[0]       = char_traits::to_char_type(last_char_);
                last_char_ = 0;
                ++s;
                --num;
            }
            adapter_.read(s, static_cast<size_t>(num));
            return num;
        }

    private:
        basic_iadapter<char_type>& adapter_;
        int_type                   last_char_;
    };

    streambuf buf_;
};

using iadapterstream  = basic_iadapterstream<char>;
using wiadapterstream = basic_iadapterstream<wchar_t>;

namespace detail
{

//
// ostreambuf
//

template <typename _CharTy>
class fast_string_ostreambuf : public std::basic_streambuf<_CharTy>
{
public:
    using char_type   = _CharTy;
    using int_type    = typename std::basic_streambuf<_CharTy>::int_type;
    using char_traits = std::char_traits<char_type>;
    using string_type = std::basic_string<char_type>;

    explicit fast_string_ostreambuf(string_type& str)
        : str_(str)
    {
    }

protected:
    virtual int_type overflow(int_type c) override
    {
        if (c != char_traits::eof())
        {
            str_.push_back(char_traits::to_char_type(c));
        }
        return c;
    }

    virtual std::streamsize xsputn(const char_type* s, std::streamsize num) override
    {
        str_.append(s, static_cast<size_t>(num));
        return num;
    }

private:
    string_type& str_;
};

//
// istreambuf
//

template <typename _CharTy>
class fast_string_istreambuf : public std::basic_streambuf<_CharTy>
{
public:
    using char_type   = _CharTy;
    using int_type    = typename std::basic_streambuf<_CharTy>::int_type;
    using char_traits = std::char_traits<char_type>;
    using string_type = std::basic_string<char_type>;

    explicit fast_string_istreambuf(const string_type& str)
        : str_(str)
        , index_(0)
    {
    }

protected:
    virtual int_type underflow() override
    {
        if (index_ >= str_.size())
            return char_traits::eof();
        return char_traits::to_int_type(str_[index_]);
    }

    virtual int_type uflow() override
    {
        int_type c = underflow();
        ++index_;
        return c;
    }

    virtual std::streamsize xsgetn(char_type* s, std::streamsize num) override
    {
        const auto copied = str_.copy(s, static_cast<size_t>(num), index_);
        return static_cast<std::streamsize>(copied);
    }

private:
    const string_type& str_;
    size_t             index_;
};

template <typename _CharTy>
class fast_buffer_istreambuf : public std::basic_streambuf<_CharTy>
{
public:
    using char_type   = _CharTy;
    using int_type    = typename std::basic_streambuf<_CharTy>::int_type;
    using char_traits = std::char_traits<char_type>;
    using string_type = std::basic_string<char_type>;

    explicit fast_buffer_istreambuf(const char_type* buffer)
        : buffer_(buffer)
        , index_(0)
        , size_(char_traits::length(buffer))
    {
    }

protected:
    virtual int_type underflow() override
    {
        if (index_ >= size_)
            return char_traits::eof();
        return char_traits::to_int_type(buffer_[index_]);
    }

    virtual int_type uflow() override
    {
        int_type c = underflow();
        ++index_;
        return c;
    }

    virtual std::streamsize xsgetn(char_type* s, std::streamsize num) override
    {
        if (index_ + static_cast<size_t>(num) >= size_)
            num = static_cast<std::streamsize>(size_ - index_);
        if (num == 0)
            return 0;
        char_traits::copy(s, buffer_, static_cast<size_t>(num));
        return num;
    }

private:
    const char_type* buffer_;
    size_t           index_;
    const size_t     size_;
};

template <typename _CharTy>
class fast_cfile_istreambuf;

template <>
class fast_cfile_istreambuf<char> : public std::basic_streambuf<char>
{
public:
    using char_type   = char;
    using int_type    = typename std::basic_streambuf<char_type>::int_type;
    using char_traits = std::char_traits<char_type>;
    using string_type = std::basic_string<char_type>;

    explicit fast_cfile_istreambuf(std::FILE* file)
        : file_(file)
        , last_char_(0)
    {
    }

protected:
    virtual int_type underflow() override
    {
        if (last_char_ != 0)
            return last_char_;
        last_char_ = std::fgetc(file_);
        return last_char_;
    }

    virtual int_type uflow() override
    {
        int_type c = underflow();
        last_char_ = 0;
        return c;
    }

    virtual std::streamsize xsgetn(char_type* s, std::streamsize num) override
    {
        if (std::fgets(s, static_cast<int>(num), file_) == nullptr)
            return 0;
        return num;
    }

private:
    std::FILE* file_;
    int_type   last_char_;
};

template <>
class fast_cfile_istreambuf<wchar_t> : public std::basic_streambuf<wchar_t>
{
public:
    using char_type   = wchar_t;
    using int_type    = typename std::basic_streambuf<char_type>::int_type;
    using char_traits = std::char_traits<char_type>;
    using string_type = std::basic_string<char_type>;

    explicit fast_cfile_istreambuf(std::FILE* file)
        : file_(file)
        , last_char_(0)
    {
    }

protected:
    virtual int_type underflow() override
    {
        if (last_char_ != 0)
            return last_char_;
        last_char_ = std::fgetwc(file_);
        return last_char_;
    }

    virtual int_type uflow() override
    {
        int_type c = underflow();
        last_char_ = 0;
        return c;
    }

    virtual std::streamsize xsgetn(char_type* s, std::streamsize num) override
    {
        if (std::fgetws(s, static_cast<int>(num), file_) == nullptr)
            return 0;
        return num;
    }

private:
    std::FILE* file_;
    int_type   last_char_;
};

//
// fast I/O stream
//

template <typename _CharTy>
class fast_basic_ostringstream : public std::basic_ostream<_CharTy>
{
public:
    using char_type   = _CharTy;
    using char_traits = std::char_traits<char_type>;
    using string_type = std::basic_string<char_type>;

    explicit fast_basic_ostringstream()
        : std::basic_ostream<char_type>(nullptr)
        , str_()
        , buf_(str_)
    {
        this->rdbuf(&buf_);
    }

    inline const string_type& str() const
    {
        return str_;
    }

    inline void str(const string_type& s)
    {
        str_ = s;
    }

private:
    string_type                       str_;
    fast_string_ostreambuf<char_type> buf_;
};

using fast_ostringstream  = fast_basic_ostringstream<char>;
using fast_wostringstream = fast_basic_ostringstream<wchar_t>;

//
// serialization functions
//

template <typename _CharTy>
struct snprintf_t
{
    using char_type = _CharTy;

    static inline void one_integer(std::basic_ostream<char_type>& os, const signed char val)
    {
        internal_one_integer(os, "%hhd", val);
    }

    static inline void one_integer(std::basic_ostream<char_type>& os, const short val)
    {
        internal_one_integer(os, "%hd", val);
    }

    static inline void one_integer(std::basic_ostream<char_type>& os, const int val)
    {
        internal_one_integer(os, "%d", val);
    }

    static inline void one_integer(std::basic_ostream<char_type>& os, const long val)
    {
        internal_one_integer(os, "%ld", val);
    }

    static inline void one_integer(std::basic_ostream<char_type>& os, const long long val)
    {
        internal_one_integer(os, "%lld", val);
    }

    static inline void one_integer(std::basic_ostream<char_type>& os, const unsigned char val)
    {
        internal_one_integer(os, "%hhu", val);
    }

    static inline void one_integer(std::basic_ostream<char_type>& os, const unsigned short val)
    {
        internal_one_integer(os, "%hu", val);
    }

    static inline void one_integer(std::basic_ostream<char_type>& os, const unsigned int val)
    {
        internal_one_integer(os, "%u", val);
    }

    static inline void one_integer(std::basic_ostream<char_type>& os, const unsigned long val)
    {
        internal_one_integer(os, "%lu", val);
    }

    static inline void one_integer(std::basic_ostream<char_type>& os, const unsigned long long val)
    {
        internal_one_integer(os, "%llu", val);
    }

    template <typename _IntTy>
    static inline void one_hex(std::basic_ostream<char_type>& os, const _IntTy val)
    {
        std::array<char, 7> buffer = {};
        internal_snprintf(os, buffer.data(), sizeof(buffer), "\\u%04X", val);
    }

    template <typename _FloatTy>
    static inline void one_float(std::basic_ostream<char_type>& os, const _FloatTy val)
    {
        std::array<char, 32> buffer = {};

        const auto p   = static_cast<int>(os.precision());
        const auto len = internal_snprintf(os, buffer.data(), sizeof(buffer), "%.*g", p, val);
        // determine if need to append ".0"
        if (std::none_of(buffer.data(), buffer.data() + len + 1, [](char c) { return c == '.'; }))
        {
            os.put(char_type('.')).put(char_type('0'));
        }
    }

private:
    template <typename _IntTy>
    static inline void internal_one_integer(std::basic_ostream<char_type>& os, const char* format, const _IntTy val)
    {
        std::array<char, 32> buffer = {};
        internal_snprintf(os, buffer.data(), sizeof(buffer), format, val);
    }

    template <typename... _Args>
    static inline int internal_snprintf(std::basic_ostream<char_type>& os, char* buffer, size_t size,
                                        const char* format, _Args&&... args)
    {
        const auto len = std::snprintf(buffer, size, format, std::forward<_Args>(args)...);
        if (len > 0)
            copy_simple_string(os, buffer, static_cast<size_t>(len));
        else
            os.setstate(std::ios_base::failbit);
        return len;
    }

    static inline void copy_simple_string(std::basic_ostream<char>& os, const char* str, size_t len)
    {
        os.write(str, static_cast<std::streamsize>(len));
    }

    template <typename _UCharTy>
    static inline void copy_simple_string(std::basic_ostream<_UCharTy>& os, const char* str, size_t len)
    {
        for (size_t i = 0; i < len; i++)
            os.put(static_cast<_UCharTy>(str[i]));
    }
};

template <typename _IntTy>
struct serializable_integer
{
    const _IntTy i;

    template <typename _CharTy>
    friend inline std::basic_ostream<_CharTy>& operator<<(std::basic_ostream<_CharTy>& os,
                                                          const serializable_integer&  i)
    {
        snprintf_t<_CharTy>::one_integer(os, i.i);
        return os;
    }

    friend std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const serializable_integer& i)
    {
        return os << i.i;
    }

    friend std::basic_ostream<wchar_t>& operator<<(std::basic_ostream<wchar_t>& os, const serializable_integer& i)
    {
        return os << i.i;
    }
};

template <typename _IntTy>
struct serializable_hex
{
    const _IntTy i;

    template <typename _CharTy>
    friend inline std::basic_ostream<_CharTy>& operator<<(std::basic_ostream<_CharTy>& os, const serializable_hex& i)
    {
        snprintf_t<_CharTy>::one_hex(os, i.i);
        return os;
    }

    friend std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const serializable_hex& i)
    {
        os << std::setfill('0') << std::hex << std::uppercase;
        os << '\\' << 'u' << std::setw(sizeof(i.i)) << i.i;
        os << std::dec << std::nouppercase;
        return os;
    }

    friend std::basic_ostream<wchar_t>& operator<<(std::basic_ostream<wchar_t>& os, const serializable_hex& i)
    {
        os << std::setfill(wchar_t('0')) << std::hex << std::uppercase;
        os << '\\' << 'u' << std::setw(sizeof(i.i)) << i.i;
        os << std::dec << std::nouppercase;
        return os;
    }
};

template <typename _FloatTy>
struct serializable_float
{
    const _FloatTy f;

    template <typename _CharTy>
    friend inline std::basic_ostream<_CharTy>& operator<<(std::basic_ostream<_CharTy>& os, const serializable_float& f)
    {
        snprintf_t<_CharTy>::one_float(os, f.f);
        return os;
    }

    friend std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const serializable_float& f)
    {
        if (std::ceil(f.f) == std::floor(f.f))
        {
            // integer
            return os << static_cast<int64_t>(f.f) << ".0";
        }
        return os << f.f;
    }

    friend std::basic_ostream<wchar_t>& operator<<(std::basic_ostream<wchar_t>& os, const serializable_float& f)
    {
        if (std::ceil(f.f) == std::floor(f.f))
        {
            // integer
            return os << static_cast<int64_t>(f.f) << L".0";
        }
        return os << f.f;
    }
};

namespace
{

template <typename _IntTy>
inline serializable_integer<_IntTy> serialize_integer(const _IntTy i)
{
    return serializable_integer<_IntTy>{ i };
}

template <typename _IntTy>
inline serializable_hex<_IntTy> serialize_hex(const _IntTy i)
{
    return serializable_hex<_IntTy>{ i };
}

template <typename _FloatTy>
inline serializable_float<_FloatTy> serialize_float(const _FloatTy f)
{
    return serializable_float<_FloatTy>{ f };
}

}  // namespace

template <typename _CharTy>
inline void copy_fmt(const std::basic_ios<_CharTy>& from, std::basic_ios<_CharTy>& to)
{
    // only copy flags
    to.setf(from.flags());
}

inline void copy_fmt(const std::basic_ios<char>& from, std::basic_ios<char>& to)
{
    to.copyfmt(from);
}

inline void copy_fmt(const std::basic_ios<wchar_t>& from, std::basic_ios<wchar_t>& to)
{
    to.copyfmt(from);
}

}  // namespace detail

}  // namespace configor
