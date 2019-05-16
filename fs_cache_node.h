#pragma once

#include <vector>
#include <filesystem>

#include <boost/intrusive/set_hook.hpp>
#include <boost/intrusive/rbtree.hpp>

#include "immutable_container_proxy.h"

template <class Notifier>
class fs_cache;

template <class Notifier>
class fs_cache_node;

template <class Notifier>
struct fs_cache_node_functor_hook
{
   //Required types
   using hook_type = boost::intrusive::set_member_hook<>;
   using hook_ptr = hook_type*;
   using const_hook_ptr = const hook_type*;
   using value_type = fs_cache_node<Notifier>;
   using pointer = value_type*;
   using const_pointer = const value_type*;

   static hook_ptr to_hook_ptr(value_type &value);
   static const_hook_ptr to_hook_ptr(const value_type &value);
   static pointer to_value_ptr(hook_ptr n);
   static const_pointer to_value_ptr(const_hook_ptr n);
};

template <class Notifier>
class fs_cache_node
{
public:
    struct comparator
    {
        template <class StringComparable>
        bool operator()(
                const fs_cache_node<Notifier>& lhs,
                const StringComparable& rhs) const;
        template <class StringComparable>
        bool operator()(
                const StringComparable& lhs,
                const fs_cache_node<Notifier>& rhs) const;
    };

    friend Notifier;
    friend class fs_cache<Notifier>;
    friend class fs_cache_node_functor_hook<Notifier>;

    fs_cache_node(
            std::filesystem::directory_entry key,
            size_t nesting,
            fs_cache_node* parent_arg = nullptr);

    /// parent ptr should be updated after move
    fs_cache_node(fs_cache_node&& other);
    fs_cache_node(const fs_cache_node& other) = delete;

    fs_cache_node& operator=(const fs_cache_node& other) = delete;
    fs_cache_node& operator=(fs_cache_node&& other) = delete;

    using tree_t = boost::intrusive::rbtree<
            fs_cache_node,
            boost::intrusive::function_hook<
                    fs_cache_node_functor_hook<Notifier>>>;

    void update();
    void remove();

    bool operator<(const fs_cache_node& other) const;

    const std::filesystem::directory_entry& dir_entry() const;
    std::string filename() const;
    fs_cache_node* parent() const;
    immutable_container_proxy<tree_t> childs();
    const tree_t& childs() const;
private:
    std::filesystem::directory_entry dir_entry_;
    boost::intrusive::set_member_hook<> hook_;
    size_t nesting_level_ = 0;
    fs_cache_node<Notifier>* parent_ = nullptr;
    Notifier data_;
    std::vector<fs_cache_node<Notifier>> childs_container_;
    tree_t childs_;
};

template <class Notifier>
inline typename fs_cache_node_functor_hook<Notifier>::hook_ptr
fs_cache_node_functor_hook<Notifier>::to_hook_ptr(
        fs_cache_node_functor_hook<Notifier>::value_type &value)
{
    return &value.hook_;
}

template <class Notifier>
inline typename fs_cache_node_functor_hook<Notifier>::const_hook_ptr
fs_cache_node_functor_hook<Notifier>::to_hook_ptr(
        const fs_cache_node_functor_hook<Notifier>::value_type &value)
{
    return &value.hook_;
}

template <class Notifier>
inline typename fs_cache_node_functor_hook<Notifier>::pointer
fs_cache_node_functor_hook<Notifier>::to_value_ptr(
        fs_cache_node_functor_hook<Notifier>::hook_ptr n)
{
    return boost::intrusive::get_parent_from_member<
            fs_cache_node<Notifier>>(
                    n,
                    &fs_cache_node<Notifier>::hook_);
}

template <class Notifier>
inline typename fs_cache_node_functor_hook<Notifier>::const_pointer
fs_cache_node_functor_hook<Notifier>::to_value_ptr(
        fs_cache_node_functor_hook<Notifier>::const_hook_ptr n)
{
    return boost::intrusive::get_parent_from_member<
            fs_cache_node<Notifier>>(
                    n,
                    &fs_cache_node<Notifier>::hook_);
}

template <class Notifier>
template <class StringComparable>
bool fs_cache_node<Notifier>::comparator::operator()(
        const fs_cache_node<Notifier>& lhs,
        const StringComparable& rhs) const
{
    return lhs.dir_entry_.path().filename() < rhs;
}

template <class Notifier>
template <class StringComparable>
bool fs_cache_node<Notifier>::comparator::operator()(
        const StringComparable& lhs,
        const fs_cache_node<Notifier>& rhs) const
{
    return lhs < rhs.dir_entry_.path().filename();
}

template <class Notifier>
fs_cache_node<Notifier>::fs_cache_node(
        std::filesystem::directory_entry key,
        size_t nesting,
        fs_cache_node* parent_arg)
    : dir_entry_(key)
    , hook_()
    , nesting_level_(nesting)
    , parent_(parent_arg)
{}

template <class Notifier>
fs_cache_node<Notifier>::fs_cache_node(fs_cache_node&& other)
    : dir_entry_(other.dir_entry_)
    , hook_(other.hook_)
    , nesting_level_(other.nesting_level_)
    , parent_(other.parent_)
    , data_(std::move(other.data_))
    , childs_container_(std::move(other.childs_container_))
{
    data_.update_self_ptr(this);
    childs_.insert_equal(
            childs_container_.begin(),
            childs_container_.end());
    for(auto& child : childs_container_)
    {
        child.parent_ = this;
    }
}

template <class Notifier>
void fs_cache_node<Notifier>::update()
{
    if(!dir_entry_.is_directory())
    {
        data_.init(this);
        return;
    }

    childs_.clear();
    childs_container_.clear();

    for(auto& entry : std::filesystem::directory_iterator(dir_entry_))
    {
        childs_container_.emplace_back(entry, nesting_level_ + 1, this);
    }
    childs_.insert_equal(
            childs_container_.begin(),
            childs_container_.end());

    data_.init(this);

    for(auto& child : childs_container_)
    {
        child.update();
    }
}

template <class Notifier>
void fs_cache_node<Notifier>::remove()
{
    //TODO fail if parent_ == nullptr?
    std::filesystem::remove_all(dir_entry_.path());
    parent_->update();
}

template <class Notifier>
bool fs_cache_node<Notifier>::operator<(const fs_cache_node& other) const
{
    return dir_entry_.path().filename() < other.dir_entry_.path().filename();
}

template <class Notifier>
const std::filesystem::directory_entry& fs_cache_node<
        Notifier>::dir_entry() const
{
    return dir_entry_;
}

template <class Notifier>
std::string fs_cache_node<Notifier>::filename() const
{
    return dir_entry_.path().filename();
}

template <class Notifier>
fs_cache_node<Notifier>* fs_cache_node<Notifier>::parent() const
{
    return parent_;
}

template <class Notifier>
immutable_container_proxy<typename fs_cache_node<
        Notifier>::tree_t> fs_cache_node<Notifier>::childs()
{
    return {childs_};
}

template <class Notifier>
const typename fs_cache_node<
        Notifier>::tree_t& fs_cache_node<Notifier>::childs() const
{
    return childs_;
}
