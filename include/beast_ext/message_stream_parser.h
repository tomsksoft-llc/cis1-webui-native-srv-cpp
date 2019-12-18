/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string_view>
#include <array>

#include <boost/system/error_code.hpp>

class message_stream_parser
{
public:
    enum class event
    {
        none,
        begin_name,
        end_name,
        begin_filename,
        end_filename,
        begin_body,
        end_body,
    };
    event handle_message_char(char c, boost::system::error_code& ec);
    event handle_message_end(boost::system::error_code& ec);
private:
    enum class state
    {
        start,
        content_disposition,
        name,
        maybe_filename,
        filename,
        filename_body,
        content_type,
        content_type_body,
        pre_body,
        body,
    };

    state message_parser_state_ = state::start;

    std::string_view::const_iterator transition_it;

    std::string_view::const_iterator tr_begin() const;
    std::string_view::const_iterator tr_end() const;

    void tr_reset();
    char tr_get_char();
    bool tr_next();
};
