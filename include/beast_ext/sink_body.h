#pragma once

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/file_base.hpp>
#include <boost/beast/core/type_traits.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/assert.hpp>
#include <boost/optional.hpp>

namespace beast_ext
{

struct sink_body
{
    struct value_type{};

    struct reader
    {
        template<bool isRequest, class Fields>
        explicit reader(
                boost::beast::http::header<isRequest, Fields>&h,
                sink_body::value_type& b){}
        void init(
                boost::optional<std::uint64_t> const&,
                boost::beast::error_code& ec)
        {
            ec.assign(0, ec.category());
        }
        template<class ConstBufferSequence>
        std::size_t put(
                ConstBufferSequence const& buffers,
                boost::beast::error_code& ec)
        {
            ec.assign(0, ec.category());
            return boost::asio::buffer_size(buffers);
        }
        void finish(boost::beast::error_code& ec)
        {
            ec.assign(0, ec.category());
        }
    };

    static std::uint64_t size(value_type const& body)
    {
        return 0;
    }
};

} // namespace beast_ext
