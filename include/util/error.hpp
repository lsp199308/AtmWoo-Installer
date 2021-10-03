/*
Copyright (c) 2017-2018 Adubbz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <cstring>
#include <stdexcept>
#include <cstdio>

#define ASSERT_OK(res_expr, desc) \
    ({ \
        const auto tmp_rc = (res_expr); \
        if (R_FAILED(tmp_rc)) { \
            char msg[256] = {}; std::snprintf(msg, 256-1, "%s:%u: %s.  Error code: 0x%08x\n", __func__, __LINE__, desc, tmp_rc); \
            throw std::runtime_error(msg); \
        } \
    })

#define THROW_FORMAT(format, ...) \
    ({ \
        char error_prefix[512] = {}; std::snprintf(error_prefix, 256-1, "%s:%u: ", __func__, __LINE__); \
        char formatted_msg[256] = {}; std::snprintf(formatted_msg, 256-1, format, ##__VA_ARGS__); \
        std::strncat(error_prefix, formatted_msg, 512-1); throw std::runtime_error(error_prefix); \
    })


#ifdef NXLINK_DEBUG
#define LOG_DEBUG(format, ...) { std::printf("%s:%u: ", __func__, __LINE__); std::printf(format, ##__VA_ARGS__); }
#else
#define LOG_DEBUG(format, ...) ;
#endif
