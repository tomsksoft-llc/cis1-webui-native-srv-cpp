#pragma once

#include <boost/asio/buffer.hpp>

template <class T>
class const_stream_adapter
{
public:
    const_stream_adapter(const T& buffer)
        : it_pos(boost::asio::buffer_sequence_begin(buffer))
        , it_end(boost::asio::buffer_sequence_end(buffer))
    {}
    using Ch = char;
    Ch Peek() const
    {
        if(it_pos == it_end)
        {
            return '\0';
        }
        return *((const char*)it_pos->data() + pos);
    }
    Ch Take()
    {
        if(it_pos == it_end)
        {
            return '\0';
        }
        auto ch = *((const char*)it_pos->data() + pos);
        ++pos;
        ++read;
        if(pos == it_pos->size())
        {
            pos = 0;
            ++it_pos;
        }
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
    decltype(boost::asio::buffer_sequence_begin(std::declval<T>())) it_pos;
    decltype(boost::asio::buffer_sequence_end(std::declval<T>())) it_end;
};
