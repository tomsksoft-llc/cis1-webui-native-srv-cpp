#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>

#include <cis1_proto_utils/transaction.h>

#include "session.h"

namespace cis
{

class session_manager
{
public:
    session_manager(boost::asio::io_context& ctx);

    std::shared_ptr<session> connect(const std::string& session_id);

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
    std::map<std::string, std::weak_ptr<session>> sessions_;
};

} // namespace cis
