#pragma once

#include <filesystem>
#include <vector>
#include <thread>
#include <variant>

#include <boost/intrusive/set_hook.hpp>
#include <boost/intrusive/rbtree.hpp>

#include <tpl_helpers/overloaded.h>

enum class fs_type
{
    dir,
    file,
};

class fs_cache;

class fs_node;

struct fs_node_functor_hook
{
   //Required types
   using hook_type = boost::intrusive::set_member_hook<>;
   using hook_ptr = hook_type*;
   using const_hook_ptr = const hook_type*;
   using value_type = fs_node;
   using pointer = value_type*;
   using const_pointer = const value_type*;

   static hook_ptr to_hook_ptr(value_type &value);
   static const_hook_ptr to_hook_ptr(const value_type &value);
   static pointer to_value_ptr(hook_ptr n);
   static const_pointer to_value_ptr(const_hook_ptr n);
};

class fs_node
{
public:
    friend class fs_cache;
    friend class fs_iterator;
    friend class fs_node_functor_hook;

    using tree_t = boost::intrusive::rbtree<
            fs_node,
            boost::intrusive::function_hook<
                    fs_node_functor_hook>>;

    struct comparator
    {
        template <class StringComparable>
        bool operator()(
                const fs_node& lhs,
                const StringComparable& rhs) const;

        template <class StringComparable>
        bool operator()(
                const StringComparable& lhs,
                const fs_node& rhs) const;
    };

    enum class node_state
    {
        init,
        deleted,
        sync,
        modified,
    };

    fs_node(std::chrono::nanoseconds invalidation_time,
            const std::filesystem::path& path,
            size_t caching_level);

    std::filesystem::path path() const;

    std::string name() const;

    uint64_t last_modified() const;

    std::chrono::time_point<std::chrono::system_clock> last_updated() const;

    const std::filesystem::directory_entry& dir_entry() const;

    fs_type type() const;

    tree_t::iterator begin();

    tree_t::iterator end();

    void update(bool force = false);

    void remove();

    void invalidate();

    node_state state() const;

    bool operator<(const fs_node& other) const;

    bool operator==(const fs_node& other) const;

private:
    node_state state_;
    std::chrono::time_point<std::chrono::system_clock> last_updated_;
    std::filesystem::directory_entry dir_entry_;
    std::chrono::nanoseconds invalidation_time_;
    size_t caching_level_;
    boost::intrusive::set_member_hook<> hook_;
    std::vector<fs_node> childs_container_;
    tree_t childs_;

    void load();
};

template <class StringComparable>
bool fs_node::comparator::operator()(
        const fs_node& lhs,
        const StringComparable& rhs) const
{
    return lhs.dir_entry_.path().filename().string() < rhs;
}

template <class StringComparable>
bool fs_node::comparator::operator()(
        const StringComparable& lhs,
        const fs_node& rhs) const
{
    return lhs < rhs.dir_entry_.path().filename().string();
}

class fs_iterator
{
public:
    fs_iterator(
            fs_cache& root,
            const fs_node::tree_t::iterator& it);

    fs_iterator(
            fs_cache& root,
            const std::filesystem::directory_iterator& it);

    fs_iterator& operator=(const fs_iterator& other);

    const std::filesystem::directory_entry& operator*() const;

    const std::filesystem::directory_entry* operator->() const;

    fs_cache& root();

    fs_iterator begin();

    fs_iterator end();

    fs_iterator find(const std::string& filename);

    void remove();

    void update();

    void invalidate();

    bool operator==(const fs_iterator& other);

    bool operator!=(const fs_iterator& other);

    fs_iterator& operator++();

private:
    fs_cache* root_;
    std::variant<
        fs_node::tree_t::iterator,
        std::filesystem::directory_iterator> it_;
};

class fs_cache
{
public:
    using comparator = fs_node::comparator;

    fs_cache(
            const std::filesystem::path& path,
            size_t max_caching_level,
            std::chrono::nanoseconds invalidation_time);

    fs_node& root();

    void preload();

    fs_iterator begin();

    fs_iterator end();

    fs_iterator find(const std::filesystem::path& path);

    void move_entry(
            const std::filesystem::path& old_path,
            const std::filesystem::path& new_path,
            std::error_code& ec);

    void create_directory(
            const std::filesystem::path& path,
            std::error_code& ec);

    std::unique_ptr<std::ostream> create_file_w(
            const std::filesystem::path& path,
            std::error_code& ec);

private:
    fs_node root_;
    size_t max_caching_level_;
    std::chrono::nanoseconds invalidation_time_;
};
