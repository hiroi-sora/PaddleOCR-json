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
#include "configor_basic.hpp"

namespace configor
{

using config  = basic_config<>;
using wconfig = basic_config<wconfig_args>;

template <typename _ConfTy, typename = typename std::enable_if<is_config<_ConfTy>::value>::type>
inline void swap(_ConfTy& lhs, _ConfTy& rhs)
{
    lhs.swap(rhs);
}

}  // namespace configor

namespace std
{
template <typename _Args>
struct hash<::configor::basic_config<_Args>>
{
    using argument_type = ::configor::basic_config<_Args>;
    using result_type   = size_t;

    result_type operator()(argument_type const& config) const
    {
        return hash<typename argument_type::string_type>{}(config.dump());
    }
};
}  // namespace std
