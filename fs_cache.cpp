#include "fs_cache.h"

#include <functional>

// fs_cache

fs_cache::fs_cache(std::filesystem::path root_dir)
    : root_(root_dir)
{
}

void fs_cache::refresh()
{
    root_.refresh();
}

fs_cache_entry::iterator fs_cache::begin()
{
    return root_.begin();
}

fs_cache_entry::iterator fs_cache::end()
{
    return root_.end();
}

fs_cache_entry::const_iterator fs_cache::begin() const
{
    return root_.begin();
}

fs_cache_entry::const_iterator fs_cache::end() const
{
    return root_.end();
}

// fs_cache_entry::iterator

fs_cache_entry::iterator::iterator(
        fs_cache_entry* parent,
        fs_cache_entry* pos)
    : parent_entry_(parent)
    , entry_(pos)
{}

fs_cache_entry::iterator::iterator(
        fs_cache_entry* parent)
    : parent_entry_(parent)
    , entry_(parent->childs_.data() + parent->childs_.size())
{}

bool fs_cache_entry::iterator::operator!=(const fs_cache_entry::iterator& other) const
{
    return entry_ != other.entry_;
}

fs_cache_entry::iterator& fs_cache_entry::iterator::operator++()
{
    ++entry_;
    return *this;
}

fs_cache_entry& fs_cache_entry::iterator::operator*()
{
    return *entry_;
}

fs_cache_entry* fs_cache_entry::iterator::operator->()
{
    return entry_;
}

const fs_cache_entry& fs_cache_entry::iterator::operator*() const
{
    return *entry_;
}

const fs_cache_entry* fs_cache_entry::iterator::operator->() const
{
    return entry_;
}

// fs_cache_entry::const_iterator

fs_cache_entry::const_iterator::const_iterator(
        const fs_cache_entry* parent,
        const fs_cache_entry* pos)
    : parent_entry_(parent)
    , entry_(pos)
{}

fs_cache_entry::const_iterator::const_iterator(
        const fs_cache_entry* parent)
    : parent_entry_(parent)
    , entry_(parent->childs_.data() + parent->childs_.size())
{}

bool fs_cache_entry::const_iterator::operator!=(
        const fs_cache_entry::const_iterator& other) const
{
    return entry_ != other.entry_;
}

fs_cache_entry::const_iterator& fs_cache_entry::const_iterator::operator++()
{
    ++entry_;
    return *this;
}

const fs_cache_entry& fs_cache_entry::const_iterator::operator*() const
{
    return *entry_;
}

const fs_cache_entry* fs_cache_entry::const_iterator::operator->() const
{
    return entry_;
}

// fs_cache_entry

fs_cache_entry::fs_cache_entry(std::filesystem::path dir)
    : entry_(dir)
    , recursive_last_write_time_(entry_.last_write_time())
{
    load_childs();
}

fs_cache_entry::fs_cache_entry(std::filesystem::directory_entry entry)
    : entry_(entry)
    , recursive_last_write_time_(entry_.last_write_time())
{}

void fs_cache_entry::refresh()
{
    entry_.refresh();
    recursive_last_write_time_ = entry_.last_write_time();
    load_childs();
}

void fs_cache_entry::load_childs()
{
    if(!entry_.is_directory())
    {
        return;
    }
    childs_.clear();
    for(auto& entry: std::filesystem::directory_iterator(entry_))
    {
        if(entry.is_regular_file() || entry.is_directory())
        {
            childs_.emplace_back(entry);
        }
    }
    for(auto& child : childs_)
    {
        child.load_childs();
        if(child.recursive_last_write_time_ > recursive_last_write_time_)
        {
            recursive_last_write_time_ = child.recursive_last_write_time_;
        }
    }
};

std::filesystem::file_time_type fs_cache_entry::recursive_last_write_time() const
{
    return recursive_last_write_time_;
}

const std::filesystem::directory_entry& fs_cache_entry::get() const
{
    return entry_;
}

fs_cache_entry::iterator fs_cache_entry::begin()
{
    return fs_cache_entry::iterator{this, childs_.data()};
}

fs_cache_entry::iterator fs_cache_entry::end()
{
    return fs_cache_entry::iterator{this};
}

fs_cache_entry::const_iterator fs_cache_entry::begin() const
{
    return fs_cache_entry::const_iterator{this, childs_.data()};
}

fs_cache_entry::const_iterator fs_cache_entry::end() const
{
    return fs_cache_entry::const_iterator{this};
}
