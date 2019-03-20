#include "publisher.h"

void publisher::publish(const std::string& topic, const std::any& data)
{
    auto it = subscribers_.find(topic);
    if(it != subscribers_.end())
    {
        for(auto sub : it->second)
        {
            sub->notify(topic, data);
        }
    }
}

void publisher::subscribe(const std::string& topic, subscriber* sub)
{
    subscribers_[topic].insert(sub);
}

void publisher::unsubscribe(const std::string& topic, subscriber* sub)
{
    subscribers_[topic].erase(sub);
}

subscriber::subscriber(publisher* pub)
    : publisher_(pub)
{}

subscriber::~subscriber()
{
    for(auto&& [topic, fn] : subscriptions_)
    {
        publisher_->unsubscribe(topic, this);
    }
}

void subscriber::subscribe(const std::string& topic, const std::function<void(const std::any&)>& cb)
{
    subscriptions_[topic] = cb;
    publisher_->subscribe(topic, this);
}

void subscriber::unsubscribe(const std::string& topic)
{
    publisher_->unsubscribe(topic, this);
    subscriptions_.erase(topic);
}

void subscriber::notify(const std::string& topic, const std::any& data)
{
    subscriptions_[topic](data);
}
