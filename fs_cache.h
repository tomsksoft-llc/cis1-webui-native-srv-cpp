#pragma once

#include <system_error>
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
    typename fs_cache_node<Notifier>::tree_t::iterator find(
            std::filesystem::path path);

    typename fs_cache_node<Notifier>::tree_t::iterator begin();
    typename fs_cache_node<Notifier>::tree_t::iterator end();

    void create_directory(std::filesystem::path path, std::error_code& ec);
    void move_entry(
            std::filesystem::path old_path,
            std::filesystem::path new_path,
            std::error_code& ec);
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

template <class Notifier>
void fs_cache<Notifier>::create_directory(
        std::filesystem::path path,
        std::error_code& ec)
{
    auto parent_dir_it = find(path.parent_path());
    if(parent_dir_it == end() || !parent_dir_it->dir_entry().is_directory())
    {
        //FIXME errorsystem
        ec.assign(1, ec.category());
        return;
    }

    if(auto child_it = parent_dir_it->childs().find(
                path.filename(),
                comparator{});
            child_it != parent_dir_it->childs().end())
    {
        //FIXME errorsystem
        ec.assign(1, ec.category());
        return;
    }

    std::filesystem::create_directory(
            parent_dir_it->dir_entry().path() / path.filename(), ec);

    if(ec)
    {
        ec.assign(1, ec.category());
        //FIXME errorsystem
        return;
    }

    parent_dir_it->update();

    ec.assign(0, ec.category());
}

template <class Notifier>
void fs_cache<Notifier>::move_entry(
        std::filesystem::path old_path,
        std::filesystem::path new_path,
        std::error_code& ec)
{
    auto old_parent_dir_it = find(old_path.parent_path());
    auto new_parent_dir_it = find(new_path.parent_path());

    if(old_parent_dir_it == end() || new_parent_dir_it == end())
    {
        ec.assign(1, ec.category());
        //FIXME errorsystem
        return;
    }

    if(auto source_it = old_parent_dir_it->childs().find(
                old_path.filename(),
                comparator{});
        source_it == old_parent_dir_it->childs().end())
    {
        ec.assign(1, ec.category());
        //FIXME errorsystem
        return;
    }

    if(auto dest_it = new_parent_dir_it->childs().find(
                new_path.filename(),
                comparator{});
        dest_it != new_parent_dir_it->childs().end())
    {
        ec.assign(1, ec.category());
        //FIXME errorsystem
        return;
    }

    std::filesystem::rename(
            old_parent_dir_it->dir_entry().path() / old_path.filename(),
            new_parent_dir_it->dir_entry().path() / new_path.filename(),
            ec);
    if(ec)
    {
        return;
    }

    old_parent_dir_it->update();
    new_parent_dir_it->update();
}
