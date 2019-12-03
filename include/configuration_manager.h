/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>
#include <any>
#include <map>
#include <optional>

#include "error_code.h"

class configuration_manager
{
public:
    template <class T>
    bool add_entry(
            const std::string& name,
            T entry)
    {
        auto [it, res] = entries_.emplace(name, entry);

        return res;
    }

    template <class T>
    T* get_entry(
            const std::string& name,
            std::error_code& ec)
    {
        if(auto it = entries_.find(name);
                it != entries_.end())
        {
            std::any* a = &(it->second);
            T* entry = std::any_cast<T>(a);

            if(entry != nullptr)
            {
                return entry;
            }

            ec.assign(1, ec.category());

            return nullptr;
        }

        return nullptr;
    }

    template <class T>
    T* get_entry(
            const std::string& name)
    {
        if(auto it = entries_.find(name);
                it != entries_.end())
        {
            std::any* a = &(it->second);
            T* entry = std::any_cast<T>(a);

            if(entry != nullptr)
            {
                return entry;
            }

            return nullptr;
        }

        return nullptr;
    }

    void remove_entry(
            const std::string& name)
    {
        entries_.erase(name);
    }

    bool has_entry(
            const std::string& name)
    {
        if(auto it = entries_.find(name);
                it != entries_.end())
        {
            return true;
        }

        return false;
    }

private:
    std::map<std::string, std::any> entries_;
};
