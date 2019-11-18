/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "beast_ext/multipart_stream_parser.h"

constexpr const char* crlf = "\r\n";
constexpr const char* ddash = "--";

void multipart_stream_parser::init(
        const std::string& boundary)
{
    preamble_delim_.append(ddash).append(boundary).append(crlf);
    encapsulation_delim_.append(crlf).append(ddash).append(boundary);
}

const std::string& multipart_stream_parser::tr_delim() const
{
    switch(parser_state_)
    {
        case state::preamble:
        {
            return preamble_delim_;
        }
        case state::first_message:
        {
            return encapsulation_delim_;
        }
        case state::encapsulation:
        {
            return encapsulation_delim_;
        }
        default:
        {
            //unreachable code
            std::terminate();
        }
    }
}

void multipart_stream_parser::tr_init()
{
    last_matched_ = 0;
    kmp_.init(tr_delim());
}

void multipart_stream_parser::tr_reset()
{
    last_matched_ = 0;
    kmp_.reset();
}

std::pair<size_t, bool> multipart_stream_parser::tr_check(char c)
{
    auto matched = kmp_.partial_search(c);
    bool result = matched == tr_delim().size();
    size_t advancement = 0;
    if(last_matched_ + 1 != matched)
    {
        advancement += last_matched_ + 1 - matched;
    }
    if(result)
    {
        advancement += tr_delim().size();
    }
    last_matched_ = matched;
    return {advancement, result};
}

std::pair<size_t, multipart_stream_parser::event> multipart_stream_parser::handle_char(
        char c,
        boost::system::error_code& ec)
{
    switch(parser_state_)
    {
        case state::start:
        {
            parser_state_ = state::preamble;
            tr_init();
            [[fallthrough]];
        }
        case state::preamble:
        {
            auto [advancement, result] = tr_check(c);
            if(result)
            {
                parser_state_ = state::first_message;
                tr_init();
                return {advancement, event::message_begin};
            }
            else
            {
                return {advancement, event::none};
            }
            break;
        }
        case state::first_message:
        {
            auto [advancement, result] = tr_check(c);
            if(result)
            {
                parser_state_ = state::maybe_end_or_encapsulation;
                return {advancement, event::message_end};
            }
            else
            {
                return {advancement, event::none};
            }
            break;
        }
        case state::maybe_end_or_encapsulation:
        {
            if(c == '-')
            {
                parser_state_ = state::maybe_end;
            }
            else if(c == '\r')
            {
                parser_state_ = state::maybe_encapsulation;
            }
            else
            {
                ec.assign(1, ec.category());
            }
            break;
        }
        case state::maybe_end:
        {
            if(c == '-')
            {
                parser_state_ = state::epilogue;
                return {2, event::none};
            }
            else
            {
                ec.assign(1, ec.category());
            }
            break;
        }
        case state::maybe_encapsulation:
        {
            if(c == '\n')
            {
                parser_state_ = state::encapsulation;
                tr_init();
                return {2, event::message_begin};
            }
            else
            {
                ec.assign(1, ec.category());
            }
            break;
        }
        case state::encapsulation:
        {
            auto [advancement, result] = tr_check(c);
            if(result)
            {
                parser_state_ = state::maybe_end_or_encapsulation;
                return {advancement, event::message_end};
            }
            else
            {
                return {advancement, event::none};
            }
            break;
        }
        case state::epilogue:
        {
            return {1, event::none};
            break;
        }
        default:
        {
            ec.assign(1, ec.category());
            return {0, event::none};
        }
    }
    return {0, event::none};
}
