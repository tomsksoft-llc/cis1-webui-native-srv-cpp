#include "message_stream_parser.h"

const std::string_view crlf = "\r\n";
const std::string_view content_disposition = "Content-Disposition: form-data; name=\"";
const std::string_view filename = " filename=\"";
const std::string_view content_type = "\r\nContent-Type: ";
const std::string_view pre_body = "\r\n\r\n";

std::string_view::const_iterator message_stream_parser::tr_begin() const
{
    switch(message_parser_state_)
    {
        case state::content_disposition:
        {
            return content_disposition.begin();
        }
        case state::filename:
        {
            return filename.begin();
        }
        case state::content_type:
        {
            return content_type.begin();
        }
        case state::pre_body:
        {
            return pre_body.begin();
        }
        default:
        {
            //unreachable code
            std::terminate();
        }
    }
}

std::string_view::const_iterator message_stream_parser::tr_end() const
{
    switch(message_parser_state_)
    {
        case state::content_disposition:
        {
            return content_disposition.end();
        }
        case state::filename:
        {
            return filename.end();
        }
        case state::content_type:
        {
            return content_type.end();
        }
        case state::pre_body:
        {
            return pre_body.end();
        }
        default:
        {
            //unreachable code
            std::terminate();
        }
    }
}

void message_stream_parser::tr_reset()
{
    transition_it = tr_begin();
}

char message_stream_parser::tr_get_char()
{
	return *transition_it;
}

bool message_stream_parser::tr_next()
{
    ++transition_it;

    return transition_it == tr_end();
}

message_stream_parser::event message_stream_parser::handle_message_char(
        char c,
        boost::system::error_code& ec)
{
    switch(message_parser_state_)
    {
        case state::start:
        {
            message_parser_state_ = state::content_disposition;
            tr_reset();
            [[fallthrough]];
        }
        case state::content_disposition:
        {
            if(c == tr_get_char())
            {
                if(tr_next())
                {
                    message_parser_state_ = state::name;
                    return event::begin_name;
                }
            }
            else
            {
                ec.assign(1, ec.category());
            }
            break;
        }
        case state::name:
        {
            if(c == '\"')
            {
                message_parser_state_ = state::maybe_filename;
                return event::end_name;
            }
            break;
        }
        case state::maybe_filename:
        {
            if(c == ';')
            {
                message_parser_state_ = state::filename;
                tr_reset();
            }
            else if(c == '\r')
            {
                message_parser_state_ = state::pre_body;
                tr_reset();
                tr_next();
            }
            else
            {
                ec.assign(1, ec.category());
                return event::none;
            }
            break;
        }
        case state::filename:
        {
            if(c == tr_get_char())
            {
                if(tr_next())
                {
                    message_parser_state_ = state::filename_body;
                    return event::begin_filename;
                }
            }
            else
            {
                ec.assign(1, ec.category());
                return event::none;
            }
            break;
        }
        case state::filename_body:
        {
            if(c == '\"')
            {
                message_parser_state_ = state::content_type;
                tr_reset();
                return event::end_filename;
            }
            break;
        }
        case state::content_type:
        {
            if(c == tr_get_char())
            {
                if(tr_next())
                {
                    message_parser_state_ = state::content_type_body;
                }
            }
            else
            {
                ec.assign(1, ec.category());
                return event::none;
            }
            break;
        }
        case state::content_type_body:
        {
            if(c == '\r')
            {
                message_parser_state_ = state::pre_body;
                tr_reset();
                tr_next();
            }
            break;
        }
        case state::pre_body:
        {
            if(c == tr_get_char())
            {
                if(tr_next())
                {
                    message_parser_state_ = state::body;
                    return event::begin_body;
                }
            }
            else
            {
                ec.assign(1, ec.category());
                return event::none;
            }
            break;
        }
        case state::body:
        {
            break;
        }
        default:
        {
            std::terminate();
        }
    }
    return event::none;
}

message_stream_parser::event message_stream_parser::handle_message_end(
        boost::system::error_code& ec)
{
    if(message_parser_state_ != state::body)
    {
        ec.assign(1, ec.category());
        return event::none;
    }
    message_parser_state_ = state::start;
    return event::end_body;
}
