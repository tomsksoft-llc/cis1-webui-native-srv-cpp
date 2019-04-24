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
#include <boost/regex.hpp>

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
    friend class reader;
    friend class writer;
    friend struct basic_multipart_form_body;

    std::filesystem::path dir_;
    // This represents the open file
    File file_;

    // The cached file size
    std::uint64_t file_size_ = 0;

    struct value_t
    {
        explicit value_t(bool is_file_arg, std::string content_arg)
            : is_file(is_file_arg)
            , content(content_arg)
        {}
        bool is_file;
        std::string content;
    };

    std::multimap<std::string, value_t> values_;
    std::multimap<std::string, File> files_;
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
    if(!is_dir)
    {
        ec = make_error_code(boost::system::errc::not_a_directory);
    }
}

/** Algorithm for storing buffers when parsing.

    Objects of this type are created during parsing
    to store incoming buffers representing the body.
*/
template<class File>
class basic_multipart_form_body<File>::reader
{
    value_type& body_;  // The body we are writing to
    std::string boundary_;
    bool cr_ = false;
    bool if_file_ = false;
    bool content_disposition_parsed_ = false;
    enum class state
    {
        init,
        headers,
        body,
        next_block
    } parser_state_ = state::init;
    std::string block_buffer_;
    std::string next_block_buffer_;
    typename value_type::values_iterator current_content_;
    typename value_type::files_iterator current_file_;
    void handle_data(
            const char* data,
            size_t size,
            boost::system::error_code& ec);
    void handle_crlf(boost::system::error_code& ec);
    void parse_content_disposition(boost::system::error_code& ec);
public:
    template<bool isRequest, class Fields>
    explicit reader(boost::beast::http::header<isRequest, Fields>&h, value_type& b);

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
    std::string boundary;
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
    // Check dir_ is directory
    std::error_code std_ec;
    BOOST_ASSERT(std::filesystem::is_directory(body_.dir_, std_ec));

    // We don't do anything with this but a sophisticated
    // application might check available space on the device
    // to see if there is enough room to store the body.
    boost::ignore_unused(content_length);

    ec.assign(0, ec.category());
}

template<class File>
template<class ConstBufferSequence>
std::size_t basic_multipart_form_body<File>::reader::put(
        ConstBufferSequence const& buffers,
        boost::beast::error_code& ec)
{
    ec.assign(0, ec.category());

    // This function must return the total number of
    // bytes transferred from the input buffers.
    std::size_t nwritten = 0;

    // Loop over all the buffers in the sequence,
    // find crlf and call subsequent parsers.
    for(auto it = boost::asio::buffer_sequence_begin(buffers);
        it != boost::asio::buffer_sequence_end(buffers); ++it)
    {
        boost::asio::const_buffer buffer = *it;

        size_t offset = 0;
        std::string_view str_view(
                        static_cast<const char*>(buffer.data()),
                        buffer.size());
        while(offset != str_view.size())
        {
            if(!cr_)
            {
                auto pos = str_view.find('\r', offset);

                if(pos != str_view.npos)
                {
                    //copy [offset, pos]
                    handle_data(str_view.data() + offset,
                                pos - offset,
                                ec);
                    nwritten += pos - offset + 1;
                    offset = pos + 1;
                    cr_ = true;
                    continue;
                }
                else
                {
                    //copy [offset, buffer.size())
                    handle_data(str_view.data() + offset,
                                str_view.size() - offset,
                                ec);
                    nwritten += buffer.size() - offset;
                    break;
                }
            }
            else if(str_view[offset] == '\n')
            {
                //parse_block
                handle_crlf(ec);
            }
            else
            {
                //add '\r'
                handle_data("\r", 1, ec);
                //copy [offset]
                handle_data(str_view.data() + offset, 1, ec);
            }
            ++offset;
            ++nwritten;
            cr_ = false;
        }
    }

    return nwritten;
}

template<class File>
void basic_multipart_form_body<File>::reader::handle_data(
        const char* data,
        size_t size,
        boost::system::error_code& ec)
{
    if(size == 0)
    {
        ec.assign(0, ec.category());
        return;
    }
    switch(parser_state_)
    {
        case state::init:
        {
            block_buffer_.append(data, size);
            break;
        }
        case state::headers:
        {
            block_buffer_.append(data, size);
            break;
        }
        case state::body:
        {
            if(!if_file_)
            {
                current_content_->second.content.append(data, size);
            }
            else
            {
                current_file_->second.write(
                    data, size, ec);
            }
            break;
        }
        case state::next_block:
        {
            next_block_buffer_.append(data, size);
            if(next_block_buffer_.size() >= boundary_.size() + 2)
            {
                if(next_block_buffer_.substr(0, 2 + boundary_.size())
                        == "--" + boundary_)
                {
                    parser_state_ = state::init;
                }
                else
                {
                    parser_state_ = state::body;
                }
                handle_data(
                        next_block_buffer_.data(),
                        next_block_buffer_.size(),
                        ec);
                next_block_buffer_.clear();
            }
            break;
        }
    }
    ec.assign(0, ec.category());
}

template<class File>
void basic_multipart_form_body<File>::reader::handle_crlf(
        boost::system::error_code& ec)
{
    switch(parser_state_)
    {
        case state::init:
        {
            if(block_buffer_ == "--" + boundary_)
            {
                block_buffer_.clear();
                parser_state_ = state::headers;
            }
            else if(block_buffer_ != "--" + boundary_ + "--")
            {
                ec.assign(74, ec.category()); //EBADMSG
                return;
            }
            break;
        }
        case state::headers:
        {
            if(!content_disposition_parsed_)
            {
                parse_content_disposition(ec);
                content_disposition_parsed_ = true;
            }
            if(block_buffer_.empty())
            {
                content_disposition_parsed_ = false;
                parser_state_ = state::body;
            }
            block_buffer_.clear();
            break;
        }
        case state::body:
        {   
            parser_state_ = state::next_block;
            break;
        }
        case state::next_block:
        {
            break;
        }
    }
    ec.assign(0, ec.category());
}

template<class File>
void basic_multipart_form_body<File>::reader::parse_content_disposition(
        boost::system::error_code& ec)
{
    static boost::regex r(R"rx(([^;=]+)(?:="([^"]+)")?(?:$|(?:; )))rx");
    auto start = block_buffer_.cbegin() + std::string("Content-Disposition: ").size();
    auto end = block_buffer_.cend();
    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;
    std::string name;
    std::string filename;
    while(regex_search(start, end, what, r, flags))
    {

        if(what[1] == "name")
        {
            name = what[2];
        }
        if(what[1] == "filename")
        {
            filename = what[2];
        }

        start = what[0].second;
        // update flags:
        flags |= boost::match_prev_avail;
        flags |= boost::match_not_bob;
    }
    if(filename.empty())
    {
        current_content_ = body_.values_.emplace(
                std::piecewise_construct,
                std::make_tuple(name),
                std::make_tuple(false, ""));
        if_file_ = false;
    }
    else
    {
        current_content_ = body_.values_.emplace(
                std::piecewise_construct,
                std::make_tuple(name),
                std::make_tuple(true, filename));
        current_file_ = body_.files_.emplace(
                std::piecewise_construct,
                std::make_tuple(filename),
                std::make_tuple());
        boost::beast::error_code ec;
        current_file_->second.open(
                (body_.dir_ / filename).c_str(),
                boost::beast::file_mode::write,
                ec);
        if_file_ = true;
    }
    ec.assign(0, ec.category());
}

template<class File>
void basic_multipart_form_body<File>::reader::finish(
        boost::beast::error_code& ec)
{
    ec.assign(0, ec.category());
}
