#pragma once

#include <algorithm>
#include <filesystem>
#include <utility>
#include <string>

#include <cstdio>
#include <cstdint>

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/file_base.hpp>
#include <boost/beast/core/type_traits.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/assert.hpp>
#include <boost/optional.hpp>

#include "multipart_stream_parser.h"
#include "message_stream_parser.h"

template<class File>
struct basic_multipart_form_body
{
    static_assert(
            boost::beast::is_file<File>::value,
            "File requirements not met");

    /// The type of File this body uses
    using file_type = File;

    class reader;

    class writer;

    class value_type;

    static std::uint64_t size(value_type const& body);
};

template<class File>
class basic_multipart_form_body<File>::value_type
{
public:
    enum class mode
    {
        add,
        replace,
        add_or_replace,
        ignore,
    };
private:
    friend class reader;
    friend class writer;
    friend struct basic_multipart_form_body;

    std::filesystem::path dir_;

    struct value_t
    {
        explicit value_t(bool is_file_arg, std::string content_arg)
            : is_file(is_file_arg)
            , content(content_arg)
        {}
        bool is_file;
        bool written = true;
        std::string content;
    };

    std::multimap<std::string, value_t> values_;
    std::multimap<std::string, File> files_;
    mode mode_ = mode::add;
    using values_iterator = typename std::multimap<std::string, value_t>::iterator;
    using files_iterator = typename std::multimap<std::string, File>::iterator;
public:


    /** Destructor.

        If the file is open, it is closed first.
    */
    ~value_type() = default;

    /// Constructor
    value_type() = default;

    /// Constructor
    value_type(value_type&& other) = default;

    /// Move assignment
    value_type& operator=(value_type&& other) = default;

    void set_dir(const std::string& dir_path, boost::beast::error_code& ec);

    void set_mode(mode m);

    mode get_mode() const;

    const std::multimap<std::string, value_t>& get_values() const
    {
        return values_;
    }

    const std::multimap<std::string, File>& get_files() const
    {
        return files_;
    }
};

template<class File>
void basic_multipart_form_body<File>::value_type::set_dir(
        const std::string& dir_path,
        boost::beast::error_code& ec)
{
    dir_ = dir_path;

    std::error_code std_ec;

    bool is_dir = std::filesystem::is_directory(dir_, std_ec);

    if(!is_dir || std_ec)
    {
        ec = make_error_code(boost::system::errc::not_a_directory);
    }
}

template<class File>
void basic_multipart_form_body<File>::value_type::set_mode(mode m)
{
    mode_ = m;
}

template<class File>
typename basic_multipart_form_body<File>::value_type::mode
basic_multipart_form_body<File>::value_type::get_mode() const
{
    return mode_;
}
/** Algorithm for storing buffers when parsing.

    Objects of this type are created during parsing
    to store incoming buffers representing the body.
*/
template<class File>
class basic_multipart_form_body<File>::reader
{
    enum class state
    {
        name,
        filename,
        content,
        file,
    };
    value_type& body_;  // The body we are writing to
    std::string boundary_;

    multipart_stream_parser multipart_parser_;
    message_stream_parser message_parser_;

    bool message_ = false;
    bool is_writable_ = false;

    size_t skip_offset_ = 0;
    state state_ = state::name;

    std::string name_buffer_;
    typename value_type::values_iterator current_content_;
    typename value_type::files_iterator current_file_;
    boost::beast::multi_buffer pending_buffer_;

    template<class ConstBufferSequenceIterator>
    void next_byte(ConstBufferSequenceIterator& it, size_t& offset);

    template<class ConstBufferSequenceIterator>
    const char& get_byte(
            ConstBufferSequenceIterator const& it,
            size_t offset);

    template<class MutableBufferSequenceIterator>
    char& get_mut_byte(
            MutableBufferSequenceIterator const& it,
            size_t offset);
public:
    template<bool isRequest, class Fields>
    explicit reader(boost::beast::http::header<isRequest, Fields>& h, value_type& b);

    void init(boost::optional<std::uint64_t> const&, boost::beast::error_code& ec);

    template<class ConstBufferSequence>
    std::size_t put(
        ConstBufferSequence const& buffers,
        boost::beast::error_code& ec);

    void finish(boost::beast::error_code& ec);
};

template<class File>
template<bool isRequest, class Fields>
basic_multipart_form_body<File>::reader::reader(
        boost::beast::http::header<isRequest, Fields>& h,
        value_type& body)
    : body_(body)
{
    auto boundary_begin = h[boost::beast::http::field::content_type].find("=");
    if(boundary_begin != h[boost::beast::http::field::content_type].npos)
    {
        boundary_ = h[boost::beast::http::field::content_type].substr(
                boundary_begin + 1,
                h[boost::beast::http::field::content_type].size());
    }
}

template<class File>
void basic_multipart_form_body<File>::reader::init(
    boost::optional<std::uint64_t> const& content_length,
    boost::beast::error_code& ec)
{
    if(boundary_.empty())
    {
        ec.assign(1, ec.category());
        return;
    }

    // Check dir_ is directory
    std::error_code std_ec;
    BOOST_ASSERT(
            std::filesystem::is_directory(body_.dir_, std_ec)
         || body_.mode_ == basic_multipart_form_body::value_type::mode::ignore);

    multipart_parser_.init(boundary_);
    // We don't do anything with this but a sophisticated
    // application might check available space on the device
    // to see if there is enough room to store the body.
    boost::ignore_unused(content_length);

    ec = {};
}

template<class File>
template<class ConstBufferSequenceIterator>
const char& basic_multipart_form_body<File>::reader::get_byte(
        ConstBufferSequenceIterator const& it,
        size_t offset)
{
    return static_cast<const char*>((*it).data())[offset];
}

template<class File>
template<class MutableBufferSequenceIterator>
char& basic_multipart_form_body<File>::reader::get_mut_byte(
        MutableBufferSequenceIterator const& it,
        size_t offset)
{
    return static_cast<char*>((*it).data())[offset];
}

template<class File>
template<class ConstBufferSequenceIterator>
void basic_multipart_form_body<File>::reader::next_byte(
        ConstBufferSequenceIterator& it,
        size_t& offset)
{
    ++offset;
    if(offset == (*it).size())
    {
        ++it;
        offset = 0;
    }
};

//FIXME make human-readable
template<class File>
template<class ConstBufferSequence>
std::size_t basic_multipart_form_body<File>::reader::put(
        ConstBufferSequence const& buffers,
        boost::beast::error_code& ec)
{
    ec = {};

    auto buffers_view = boost::beast::buffers_cat(pending_buffer_.data(), buffers);

    std::size_t nwritten = 0;

    size_t parsed_offset = 0;

    auto msg_it = boost::asio::buffer_sequence_begin(buffers_view);
    size_t msg_offset = 0;

    auto part_it = boost::asio::buffer_sequence_begin(buffers_view);
    size_t part_offset = 0;
    auto part_end = boost::asio::buffer_sequence_begin(buffers_view);
    size_t part_offset_end = 0;

    auto write_part = [&]()
    {
        switch(state_)
        {
            case state::name:
            {
                //TODO iterate first and reserve all
                for(;part_it != part_end; ++part_it)
                {
                    name_buffer_.reserve(
                            name_buffer_.size() + (*part_it).size() - part_offset);
                    std::copy(
                            (char*)(*part_it).data() + part_offset,
                            (char*)(*part_it).data() + (*part_it).size(),
                            std::back_inserter(name_buffer_));
                    part_offset = 0;
                }
                name_buffer_.reserve(
                        name_buffer_.size() + part_offset_end - part_offset);
                std::copy(
                        (char*)(*part_it).data() + part_offset,
                        (char*)(*part_it).data() + part_offset_end,
                        std::back_inserter(name_buffer_));
                part_offset += part_offset_end - part_offset;
                break;
            }
            case state::content:
            {
                [[fallthrough]];
            }
            case state::filename:
            {
                auto& filename_buffer = current_content_->second.content;
                for(;part_it != part_end; ++part_it)
                {
                    filename_buffer.reserve(
                            filename_buffer.size() + (*part_it).size() - part_offset);
                    std::copy(
                            (char*)(*part_it).data() + part_offset,
                            (char*)(*part_it).data() + (*part_it).size(),
                            std::back_inserter(filename_buffer));
                    part_offset = 0;
                }
                if(part_end !=  boost::asio::buffer_sequence_end(buffers_view))
                {
                    filename_buffer.reserve(
                            filename_buffer.size() + part_offset_end - part_offset);
                    std::copy(
                            (char*)(*part_it).data() + part_offset,
                            (char*)(*part_it).data() + part_offset_end,
                            std::back_inserter(filename_buffer));
                    part_offset += part_offset_end - part_offset;
                }
                break;
            }
            case state::file:
            {
                for(;part_it != part_end; ++part_it)
                {
                    if(current_file_->second.is_open())
                    {
                        current_file_->second.write(
                                (char*)(*part_it).data() + part_offset,
                                (*part_it).size() - part_offset,
                                ec);
                    }
                    part_offset = 0;
                }
                if(part_end != boost::asio::buffer_sequence_end(buffers_view))
                {
                    if(current_file_->second.is_open())
                    {
                        current_file_->second.write(
                                (char*)(*part_it).data() + part_offset,
                                part_offset_end - part_offset,
                                ec);
                    }
                    part_offset += part_offset_end - part_offset;
                }
                break;
            }
            default:
            {}
        }
    };

    auto message_part_event = [&](message_stream_parser::event ev)
    {
        switch(ev)
        {
            case message_stream_parser::event::begin_name:
            {
                is_writable_ = true;
                part_it = msg_it;
                part_offset = msg_offset;
                next_byte(part_it, part_offset);
                break;
            }
            case message_stream_parser::event::end_name:
            {
                is_writable_ = false;
                part_end = msg_it;
                part_offset_end = msg_offset;
                write_part();
                state_ = state::content;
                break;
            }
            case message_stream_parser::event::begin_filename:
            {
                current_content_ = body_.values_.emplace(
                        std::piecewise_construct,
                        std::make_tuple(name_buffer_),
                        std::make_tuple(true, ""));
                name_buffer_.clear();
                state_ = state::filename;
                is_writable_ = true;
                part_it = msg_it;
                part_offset = msg_offset;
                next_byte(part_it, part_offset);
                break;
            }
            case message_stream_parser::event::end_filename:
            {
                is_writable_ = false;
                part_end = msg_it;
                part_offset_end = msg_offset;
                write_part();
                auto& filename = current_content_->second.content;

                std::error_code std_ec;
                auto file_exists = std::filesystem::exists(body_.dir_ / filename, std_ec);
                //ignore ec

                current_file_ = body_.files_.emplace(
                        std::piecewise_construct,
                        std::make_tuple(filename),
                        std::make_tuple());

                if(body_.mode_
                == basic_multipart_form_body::value_type::mode::ignore)
                {
                    current_content_->second.written = false;
                }
                else if(body_.mode_
                == basic_multipart_form_body::value_type::mode::replace
                && !file_exists)
                {
                    current_content_->second.written = false;
                }
                else if(body_.mode_
                == basic_multipart_form_body::value_type::mode::add
                && file_exists)
                {
                    current_content_->second.written = false;
                }
                else
                {
                    current_file_->second.open(
                            (body_.dir_ / filename).string().c_str(),
                            boost::beast::file_mode::write,
                            ec);
                }

                state_ = state::file;
                break;
            }
            case message_stream_parser::event::begin_body:
            {
                if(state_ == state::content)
                {
                    current_content_ = body_.values_.emplace(
                            std::piecewise_construct,
                            std::make_tuple(name_buffer_),
                            std::make_tuple(false, ""));
                    name_buffer_.clear();
                }
                is_writable_ = true;
                part_it = msg_it;
                part_offset = msg_offset;
                next_byte(part_it, part_offset);
                break;
            }
            case message_stream_parser::event::end_body:
            {
                is_writable_ = false;
                part_end = msg_it;
                part_offset_end = msg_offset;
                write_part();
                if(state_ == state::file)
                {
                    current_file_->second.close(ec);
                }
                state_ = state::name;
                break;
            }
        }
    };

    auto on_parser_event = [&](
            multipart_stream_parser::event ev,
            boost::system::error_code ec)
    {
        switch(ev)
        {
            case multipart_stream_parser::event::message_begin:
            {
                message_ = true;
                break;
            }
            case multipart_stream_parser::event::message_end:
            {
                auto ev = message_parser_.handle_message_end(ec);
                message_part_event(ev);
                message_ = false;
                break;
            }
            default:
            {}
        }
    };

    auto advance = [&](size_t adv, boost::system::error_code& ec)
    {
        parsed_offset += adv;
        for(size_t i = 0; i < adv; ++i, next_byte(msg_it, msg_offset))
        {
            if(message_)
            {
                char c = get_byte(msg_it, msg_offset);
                auto ev = message_parser_.handle_message_char(c, ec);
                message_part_event(ev);
            }
        }
    };

    for(auto it = boost::asio::buffer_sequence_begin(buffers);
        it != boost::asio::buffer_sequence_end(buffers); ++it)
    {
        const auto& buffer = *it;
        for(size_t offset = 0; offset < buffer.size(); ++offset)
        {
            char c = get_byte(it, offset);

            auto [adv, event] = multipart_parser_.handle_char(c, ec);

            if(ec)
            {
                return nwritten;
            }

            if(event == multipart_stream_parser::event::message_begin)
            {
                advance(adv, ec);
                on_parser_event(event, ec);
            }
            else
            {
                on_parser_event(event, ec);
                advance(adv, ec);
            }

            if(ec)
            {
                return nwritten;
            }

            ++nwritten;
        }
    }

    if(is_writable_)
    {
        part_end = msg_it;
        part_offset_end = msg_offset;
        write_part();
    }

    auto consumed_size = parsed_offset < pending_buffer_.size() ?
                        parsed_offset : pending_buffer_.size();

    intmax_t buf_size = nwritten + consumed_size - parsed_offset;

    auto buf = pending_buffer_.prepare(buf_size);
    {
        auto src_begin = msg_it;
        auto src_end = boost::asio::buffer_sequence_end(buffers_view);
        size_t src_offset = msg_offset;
        auto dst_begin = boost::asio::buffer_sequence_begin(buf);
        auto dst_end = boost::asio::buffer_sequence_end(buf);
        size_t dst_offset = 0;
        while(src_begin != src_end && dst_begin != dst_end)
        {
            get_mut_byte(dst_begin, dst_offset) = get_byte(src_begin, src_offset);
            next_byte(src_begin, src_offset);
            next_byte(dst_begin, dst_offset);
        }
    }
    pending_buffer_.commit(buf_size);

    pending_buffer_.consume(consumed_size);

    ec = {};

    return nwritten;
}

template<class File>
void basic_multipart_form_body<File>::reader::finish(
        boost::beast::error_code& ec)
{
    if(state_ == state::name)
    {
        ec = {};
    }
    else
    {
        ec.assign(1, ec.category());
    }
}
