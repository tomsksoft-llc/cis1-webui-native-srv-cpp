#pragma once

#include <boost/asio/buffer.hpp>

template <class T>
class const_stream_adapter
{
public:
    const_stream_adapter(const T& buffer, size_t bytes_transferred)
        : it_pos((const char*)buffer.data())
        , it_end(it_pos + bytes_transferred)
    {}
    using Ch = char;
    Ch Peek() const
    {
        if(it_pos == it_end)
        {
            return '\0';
        }
        return *it_pos;
    }
    Ch Take()
    {
        if(it_pos == it_end)
        {
            return '\0';
        }
        auto ch = *it_pos;
        ++it_pos;
        return ch;
    }
    size_t Tell() const
    {
        return read;
    }
    /*should be unreachable*/
    Ch* PutBegin()
    {
        assert(false);
        return 0;
    }
    void Put(Ch)
    {
        assert(false);
    }
    void Flush()
    {
        assert(false);
    }
    size_t PutEnd(Ch*)
    {
        assert(false);
        return 0;
    }
private:
    size_t read = 0;
    size_t pos = 0;
    const char* it_pos;
    const char* it_end;
};
