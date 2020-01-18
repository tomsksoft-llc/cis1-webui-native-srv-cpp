#include "cis/offline_session.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <iomanip>

namespace cis
{

const char* const combined_log_regex_expr =
        R"((\d{4}-\d{2}-\d{2}-\d{2}-\d{2}-\d{2}))"  //date-time
        R"(\s+\|\s+)"                               //delimeter
        R"((\d{1,8}))"                              //ppid
        R"(\s+\|\s+)"                               //delimeter
        R"((\d{1,8}))"                              //pid
        R"(\s+\|)"                                  //delimeter
        R"((.*))";                                  //message

const std::regex combined_log_regex(combined_log_regex_expr);

offline_session::offline_session(
        const std::filesystem::path& sessions_root,
        const std::string& session_id)
{
    std::ifstream combined_log(
            sessions_root / (session_id + ".combined.log"));

    std::string line;
    while(std::getline(combined_log, line))
    {
        if(     std::smatch smatch;
                std::regex_match(line, smatch, combined_log_regex))
        {
            cis1::cwu::log_entry entry;

            std::stringstream ss(smatch[1]);

            std::tm t = {};

            ss >> std::get_time(&t, "%Y-%m-%d-%H-%M-%S");

            entry.time = std::chrono::system_clock::from_time_t(
                    std::mktime(&t));

            entry.message = smatch[4];

            log(entry);
        }
    }
}

void offline_session::establish()
{
    //may happen if session_id of new session equals session_id of old session
    //shouldn't happen in real world
    //may cause unfroseen consequences
}

void offline_session::log(const cis1::cwu::log_entry& dto)
{
    received_logs_.push_back(dto);

    for(auto& [key, subscriber] : subscribers_)
    {
        if(auto s = subscriber.lock(); s)
        {
            s->log_entry(dto);
        }
    }
}

void offline_session::subscribe(
        uint64_t ws_session_id,
        std::shared_ptr<subscriber_interface> subscriber)
{
    for(auto& log_entry : received_logs_)
    {
        subscriber->log_entry(log_entry);
    }

    subscribers_.emplace(ws_session_id, subscriber);

    subscriber->session_closed();
}

std::shared_ptr<subscriber_interface> offline_session::get_subscriber(
        uint64_t ws_session_id)
{
    if(auto it = subscribers_.find(ws_session_id); it != subscribers_.end())
    {
        return it->second.lock();
    }

    return nullptr;
}

void offline_session::finish()
{}

} // namespace cis
