#include "fs_cache.h"

#include <fstream>

fs_node::fs_node(std::chrono::nanoseconds invalidation_time,
        const std::filesystem::path& path,
        size_t caching_level)
    : state_(node_state::init)
    , invalidation_time_(invalidation_time)
    , last_updated_(std::chrono::system_clock::now())
    , dir_entry_(path)
    , caching_level_(caching_level)
{}

std::filesystem::path fs_node::path() const
{
    return dir_entry_.path();
}

std::string fs_node::name() const
{
    return dir_entry_.path().filename();
}

uint64_t fs_node::last_modified() const
{
    return dir_entry_.last_write_time().time_since_epoch().count();
}

std::chrono::time_point<std::chrono::system_clock> fs_node::last_updated() const
{
    return last_updated_;
}

const std::filesystem::directory_entry& fs_node::dir_entry() const
{
    return dir_entry_;
}

fs_type fs_node::type() const
{
    return dir_entry_.is_directory() ? fs_type::dir : fs_type::file;
}

fs_node::tree_t::iterator fs_node::begin()
{
    return childs_.begin();
}

fs_node::tree_t::iterator fs_node::end()
{
    return childs_.end();
}

void fs_node::update(bool force)
{

    auto now = std::chrono::system_clock::now();

    if(state_ == node_state::sync
    && !force
    && (now - last_updated_) < invalidation_time_)
    {
        return;
    }

    dir_entry_.refresh();
    last_updated_ = now;

    if(!dir_entry_.exists())
    {
        state_ = node_state::deleted;

        return;
    }

    load();
}

void fs_node::remove()
{
    std::error_code ec;

    std::filesystem::remove_all(dir_entry_.path(), ec);

    childs_.clear();
    childs_container_.clear();

    state_ = node_state::deleted;
}

void fs_node::invalidate()
{
    state_ = node_state::modified;
}

fs_node::node_state fs_node::state() const
{
    return state_;
}

bool fs_node::operator<(const fs_node& other) const
{
    return dir_entry_.path() < other.dir_entry_.path();
}

bool fs_node::operator==(const fs_node& other) const
{
    return dir_entry_.path() == other.dir_entry_.path();
}

void fs_node::load()
{
    if(!dir_entry_.is_directory() || caching_level_ == 1)
    {
        return;
    }

    std::vector<fs_node> childs_container;
    tree_t childs;

    std::error_code ec;

    for(auto it = std::filesystem::directory_iterator(dir_entry_, ec);
             it != std::filesystem::directory_iterator();
             it.increment(ec))
    {
        if(ec)
        {
            return;
        }

        auto& entry = *it;

        childs_container.emplace_back(
                invalidation_time_,
                entry.path(),
                caching_level_ > 0 ? caching_level_ - 1 : 0);
    }

    childs.insert_equal(
            childs_container.begin(),
            childs_container.end());

    auto old_it = childs_.begin();
    auto old_end = childs_.end();
    auto new_it = childs.begin();
    auto new_end = childs.end();

    while(old_it != old_end && new_it != new_end)
    {
        if(*old_it < *new_it)
        {
            ++old_it;
        }
        else if(*old_it == *new_it)
        {
            if(old_it->state_ == node_state::sync)
            {
                new_it->childs_ = std::move(old_it->childs_);
                new_it->childs_container_ = std::move(old_it->childs_container_);
                new_it->last_updated_ = old_it->last_updated_;
                new_it->state_ = old_it->state_;
            }

            ++old_it;
            ++new_it;
        }
        else
        {
            ++new_it;
        }
    }

    childs_.clear();
    childs_container_.clear();

    childs_ = std::move(childs);
    childs_container_ = std::move(childs_container);

    state_ = node_state::sync;
}

fs_node_functor_hook::hook_ptr
fs_node_functor_hook::to_hook_ptr(
        fs_node_functor_hook::value_type &value)
{
    return &value.hook_;
}

fs_node_functor_hook::const_hook_ptr
fs_node_functor_hook::to_hook_ptr(
        const fs_node_functor_hook::value_type &value)
{
    return &value.hook_;
}

fs_node_functor_hook::pointer
fs_node_functor_hook::to_value_ptr(
        fs_node_functor_hook::hook_ptr n)
{
    return boost::intrusive::get_parent_from_member<
            fs_node>(
                    n,
                    &fs_node::hook_);
}

fs_node_functor_hook::const_pointer
fs_node_functor_hook::to_value_ptr(
        fs_node_functor_hook::const_hook_ptr n)
{
    return boost::intrusive::get_parent_from_member<
            fs_node>(
                    n,
                    &fs_node::hook_);
}

fs_iterator::fs_iterator(
        fs_cache& root,
        const fs_node::tree_t::iterator& it)
    : root_(&root)
    , it_(it)
{}

fs_iterator::fs_iterator(
        fs_cache& root,
        const std::filesystem::directory_iterator& it)
    : root_(&root)
    , it_(it)
{}

fs_iterator& fs_iterator::operator=(const fs_iterator& other)
{
    root_ = other.root_;
    it_ = other.it_;

    return *this;
}

const std::filesystem::directory_entry& fs_iterator::operator*() const
{
    return std::visit(
            meta::overloaded{
                    [](const fs_node::tree_t::iterator& it)
                        -> const std::filesystem::directory_entry&
                    {
                        return it->dir_entry();
                    },
                    [](const std::filesystem::directory_iterator& it)
                        -> const std::filesystem::directory_entry&
                    {
                        return *it;
                    }},
            it_);
}

const std::filesystem::directory_entry* fs_iterator::operator->() const
{
    return std::visit(
            meta::overloaded{
                    [](const fs_node::tree_t::iterator& it)
                        -> const std::filesystem::directory_entry*
                    {
                        return &(it->dir_entry());
                    },
                    [](const std::filesystem::directory_iterator& it)
                        -> const std::filesystem::directory_entry*
                    {
                        return &(*it);
                    }},
            it_);
}

fs_cache& fs_iterator::root()
{
    return *root_;
}

fs_iterator fs_iterator::begin()
{
    return std::visit(
            meta::overloaded{
                    [&](const fs_node::tree_t::iterator& it)
                        -> fs_iterator
                    {
                        if(it->caching_level_ > 1)
                        {
                            it->update();

                            auto new_it = it->begin();
                            
                            if(new_it->state_ == fs_node::node_state::deleted)
                            {
                                ++new_it;
                            }

                            return {*root_, new_it};
                        }
                        else
                        {
                            std::error_code ec;

                            std::filesystem::directory_iterator dir_it(
                                    it->path(),
                                    ec);

                            return {*root_, dir_it};
                        }
                    },
                    [&](const std::filesystem::directory_iterator& it)
                        -> fs_iterator
                    {
                        std::error_code ec;

                        std::filesystem::directory_iterator dir_it(*it, ec);

                        return {*root_, dir_it};
                    }},
            it_);
}

fs_iterator fs_iterator::end()
{
    return std::visit(
            meta::overloaded{
                    [&](const fs_node::tree_t::iterator& it)
                        -> fs_iterator
                    {
                        if(it->caching_level_ > 1)
                        {
                            return {*root_, it->end()};
                        }
                        else
                        {
                            return {*root_, std::filesystem::directory_iterator()};
                        }
                    },
                    [&](const std::filesystem::directory_iterator& it)
                        -> fs_iterator
                    {
                        return {*root_, std::filesystem::directory_iterator()};
                    }},
            it_);
}

fs_iterator fs_iterator::find(const std::string& name)
{
    return std::visit(
            meta::overloaded{
                    [&](const fs_node::tree_t::iterator& it)
                        -> fs_iterator
                    {
                        if(it->caching_level_ > 1)
                        {
                            it->update();

                            auto tree_it = it->childs_.find(
                                            name,
                                            fs_node::comparator{});

                            if(tree_it != it->childs_.end()
                            && tree_it->state_ == fs_node::node_state::deleted)
                            {
                                tree_it = it->childs_.end();
                            }

                            return {*root_,
                                    tree_it};
                        }
                        else
                        {
                            auto it = begin();

                            for(; it != end(); ++it)
                            {
                                if(it->path().filename() == name)
                                {
                                    break;
                                }
                            }

                            return it;
                        }
                    },
                    [&](const std::filesystem::directory_iterator&)
                        -> fs_iterator
                    {
                        auto it = begin();

                        for(; it != end(); ++it)
                        {
                            if(it->path().filename() == name)
                            {
                                break;
                            }
                        }

                        return it;
                    }},
            it_);
}

void fs_iterator::remove()
{
    std::visit(
            meta::overloaded{
                    [](fs_node::tree_t::iterator& it)
                    {
                        it->remove();
                    },
                    [](std::filesystem::directory_iterator& it)
                    {
                        std::error_code ec;

                        std::filesystem::remove_all(it->path(), ec);
                    }},
            it_);
}

void fs_iterator::update()
{
    std::visit(
            meta::overloaded{
                    [](fs_node::tree_t::iterator& it)
                    {
                        it->update();
                    },
                    [](auto& it)
                    {}},
            it_);
}

void fs_iterator::invalidate()
{
    std::visit(
            meta::overloaded{
                    [](fs_node::tree_t::iterator& it)
                    {
                        it->invalidate();
                    },
                    [](auto& it)
                    {}},
            it_);
}

bool fs_iterator::operator==(const fs_iterator& other)
{
    return (root_ != other.root_) ||
        std::visit(
                [&](const auto& it)
                {
                    using T = typename std::decay<decltype(it)>::type;
                    if(std::holds_alternative<T>(other.it_))
                    {
                        return it == std::get<T>(other.it_);
                    }
                    else
                    {
                        return false;
                    }
                },
                it_);
}

bool fs_iterator::operator!=(const fs_iterator& other)
{
    return !(*this == other);
}

fs_iterator& fs_iterator::operator++()
{
    std::visit(
            meta::overloaded{
                    [](fs_node::tree_t::iterator& it)
                    {
                        auto end = fs_node::tree_t::container_from_iterator(it).end();

                        do {

                        ++it;

                        } while(it != end && it->state_ == fs_node::node_state::deleted);
                    },
                    [&](std::filesystem::directory_iterator& it)
                    {
                        std::error_code ec;

                        it.increment(ec);

                        if(ec)
                        {
                            it_ = std::filesystem::directory_iterator();
                        }
                    }
            },
            it_);

    return *this;
}

fs_cache::fs_cache(
        const std::filesystem::path& path,
        size_t max_caching_level,
        std::chrono::nanoseconds invalidation_time)
    : root_(invalidation_time, path, max_caching_level)
    , max_caching_level_(max_caching_level)
    , invalidation_time_(invalidation_time)
{}

fs_node& fs_cache::root()
{
    return root_;
}

fs_iterator fs_cache::begin()
{
    root_.update();

    return {*this, root_.begin()};
}

fs_iterator fs_cache::end()
{
    return {*this, root_.end()};
}

fs_iterator fs_cache::find(const std::filesystem::path& path)
{
    auto now = std::chrono::system_clock::now();
    size_t depth = 0;

    fs_node* parent = &root_;
    fs_node* current = &root_;

    for(auto& part : path)
    {
        if(part == "/")
        {
            continue;
        }

        ++depth;

        bool updated = false;

        if(now - current->last_updated() > invalidation_time_
        && depth <= max_caching_level_
        || current->state() == fs_node::node_state::init)
        {
            current->update();
            updated = true;
        }

        if(depth == max_caching_level_)
        {
            auto tmp_path = root_.dir_entry().path();
            tmp_path += path;
            tmp_path.remove_filename();

            std::error_code ec;
            std::filesystem::directory_iterator dir_it(tmp_path, ec);

            if(ec)
            {
                return end();
            }

            for(; dir_it != std::filesystem::directory_iterator(); ++dir_it)
            {
                if(dir_it->path().filename() == path.filename())
                {
                    break;
                }
            }

            if(dir_it == std::filesystem::directory_iterator())
            {
                return end();
            }

            return fs_iterator{*this, dir_it};
        }


        auto it = current->childs_.find(
                part.filename().string(),
                comparator{});

        if(it == current->childs_.end() && !updated)
        {
            current->update();
            updated = true;
            it = current->childs_.find(
                    part.filename().string(),
                    comparator{});
        }

        if(it == current->childs_.end())
        {
            return end();
        }

        parent = current;
        current = &(*it);
    }

    return {*this, parent->childs_.iterator_to(*current)};
}

void fs_cache::move_entry(
        const std::filesystem::path& old_path,
        const std::filesystem::path& new_path,
        std::error_code& ec)
{
    std::filesystem::rename(
            root_.path() += old_path,
            root_.path() += new_path,
            ec);

    if(!ec)
    {
        auto old_it = find(old_path.parent_path());

        if(old_it != end())
        {
            old_it.invalidate();
        }

        auto new_it = find(new_path.parent_path());

        if(new_it != end())
        {
            new_it.invalidate();
        }
    }
}

void fs_cache::create_directory(
        const std::filesystem::path& path,
        std::error_code& ec)
{
    std::filesystem::create_directories(
            root_.path() += path,
            ec);

    if(!ec)
    {
        auto it = find(path.parent_path());

        if(it != end())
        {
            it.invalidate();
        }
    }
}

std::unique_ptr<std::ostream> fs_cache::create_file_w(
        const std::filesystem::path& path,
        std::error_code& ec)
{
    auto file = std::make_unique<std::ofstream>(
            root_.path() += path,
            std::ios_base::out);

    if(!file->is_open())
    {
        ec.assign(1, ec.category());

        return nullptr;
    }

    auto it = find(path.parent_path());

    if(it != end())
    {
        it.invalidate();
    }

    return file;
}
