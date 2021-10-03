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

#include "data/byte_buffer.hpp"

#include "util/error.hpp"
#include "util/debug.h"

namespace tin::data
{
    ByteBuffer::ByteBuffer(size_t reserveSize)
    {
        m_buffer.resize(reserveSize);
    }

    size_t ByteBuffer::GetSize()
    {
        return m_buffer.size();
    }

    u8* ByteBuffer::GetData()
    {
        return m_buffer.data();
    }

    void ByteBuffer::Resize(size_t size)
    {
        m_buffer.resize(size, 0);
    }

    void ByteBuffer::DebugPrintContents()
    {
        LOG_DEBUG("Buffer Size: 0x%lx\n", this->GetSize());
        printBytes(this->GetData(), this->GetSize(), true);
    }
}