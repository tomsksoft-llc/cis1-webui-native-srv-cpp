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

#include "knuth_morris_pratt_searcher.h"

class multipart_stream_parser
{
public:
    enum class event
    {
        none,
        message_begin,
        message_end,
    };
    void init(
            const std::string& boundary);
    std::pair<size_t, event> handle_char(char c, boost::system::error_code& ec);
private:
    enum class state
    {
        start,
        preamble,
        first_message,
        maybe_end_or_encapsulation,
        maybe_end,
        maybe_encapsulation,
        encapsulation,
        epilogue,
    };

    state parser_state_ = state::start;

    knuth_morris_pratt_searcher kmp_;

    std::string preamble_delim_;
    std::string encapsulation_delim_;

    const std::string& tr_delim() const;
    size_t last_matched_ = 0;

    void tr_init();
    void tr_reset();
    std::pair<size_t, bool> tr_check(char c);
};
