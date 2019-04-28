#pragma once

#include <stdexcept>

#include "fs_cache_node.h"

template <class Notifier>
class fs_cache
{
public:
    using comparator = typename fs_cache_node<Notifier>::comparator;

    template <class U>
    fs_cache(std::filesystem::path path, U* context);

    fs_cache_node<Notifier>& at(std::filesystem::path path);
    typename fs_cache_node<Notifier>::tree_t::iterator find(std::filesystem::path path);

    typename fs_cache_node<Notifier>::tree_t::iterator begin();
    typename fs_cache_node<Notifier>::tree_t::iterator end();
private:
    fs_cache_node<Notifier> root_;
};

template <class Notifier>
template <class U>
fs_cache<Notifier>::fs_cache(std::filesystem::path path, U* context)
    : root_(std::filesystem::directory_entry(path), 0)
{
    root_.data_.set_context(context);
    root_.update();
}

template <class Notifier>
fs_cache_node<Notifier>& fs_cache<Notifier>::at(std::filesystem::path path)
{
    fs_cache_node<Notifier>* current = &root_;
    for(auto& part : path)
    {
        if(part == "/")
        {
            continue;
        }
        auto it = current->childs_.find(part.filename().string(), comparator{});
        if(it == current->childs_.end())
        {
            throw std::out_of_range("Can't find sufficient child");
        }
        current = &(*it);
    }
    return *current;
}

template <class Notifier>
typename fs_cache_node<Notifier>::tree_t::iterator fs_cache<Notifier>::find(
        std::filesystem::path path)
{
    fs_cache_node<Notifier>* current = &root_;
    for(auto& part : path)
    {
        if(part == "/")
        {
            continue;
        }
        auto it = current->childs_.find(part.filename().string(), comparator{});
        if(it == current->childs_.end())
        {
            return end();
        }
        current = &(*it);
    }
    return current->parent_->childs_.iterator_to(*current);
}

template <class Notifier>
typename fs_cache_node<Notifier>::tree_t::iterator fs_cache<Notifier>::begin()
{
    return root_.childs_.begin();
}

template <class Notifier>
typename fs_cache_node<Notifier>::tree_t::iterator fs_cache<Notifier>::end()
{
    return root_.childs_.end();
}
