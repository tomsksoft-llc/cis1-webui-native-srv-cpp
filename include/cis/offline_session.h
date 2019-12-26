#pragma once

#include <vector>
#include <map>
#include <filesystem>

#include "session_interface.h"

namespace cis
{

class offline_session
    : public session_interface
{
public:
    offline_session(
            const std::filesystem::path& sessions_root,
            const std::string& session_id);

    void establish() override;

    void log(const cis1::cwu::log_entry& dto) override;

    void subscribe(
            uint64_t ws_session_id,
            std::shared_ptr<subscriber_interface> subscriber) override;

    std::shared_ptr<subscriber_interface> get_subscriber(
            uint64_t ws_session_id) override;

    void finish() override;

private:
    std::vector<cis1::cwu::log_entry> received_logs_;
    std::map<uint64_t, std::weak_ptr<subscriber_interface>> subscribers_;
};

} // namespace cis
