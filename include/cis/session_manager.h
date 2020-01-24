/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

#include <cis1_proto_utils/transaction.h>

#include "session_interface.h"

namespace cis
{

class session_manager
{
public:
    session_manager(
            boost::asio::io_context& ctx,
            const std::filesystem::path& sessions_root);

    std::shared_ptr<session_interface> connect(
            const std::string& session_id);

    void subscribe(
            const std::string& session_id,
            uint64_t ws_session_id,
            std::shared_ptr<subscriber_interface> subscriber);

    std::shared_ptr<subscriber_interface> get_subscriber(
            const std::string& session_id,
            uint64_t ws_session_id);

    void finish_session(const std::string& session_id);

private:
    boost::asio::io_context& ctx_;
    std::filesystem::path sessions_root_;
    std::map<std::string, std::weak_ptr<session_interface>> sessions_;
};

} // namespace cis
