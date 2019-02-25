#pragma once

#include <functional>
#include <map>
#include <set>
#include <any>

class subscriber;

class publisher
{
public:
    void publish(const std::string& topic, const std::any& data);
private:
    friend class subscriber;
    void subscribe(const std::string& topic, subscriber* sub);
    void unsubscribe(const std::string& topic, subscriber* sub);
    std::map<std::string, std::set<subscriber*>> subscribers_;
};

class subscriber
{
public:
    subscriber(publisher* pub);
    ~subscriber();
    void subscribe(const std::string& topic, std::function<void(const std::any&)> cb);
    void unsubscribe(const std::string& topic);
    void notify(const std::string& topic, const std::any& data);
private:
    publisher* publisher_;
    std::map<std::string, std::function<void(const std::any&)>> subscriptions_;
};
