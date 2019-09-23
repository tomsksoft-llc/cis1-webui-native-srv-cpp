#pragma once

template <class Container>
class immutable_container_proxy
{
public:
    immutable_container_proxy(Container& c)
        : c_(c)
    {}

    template <class Key>
    typename Container::reference at(const Key& key)
    {
        return c_.at(key);
    }
    template <class Key>
    typename Container::const_reference at(const Key& key) const
    {
        return c_.at(key);
    }

    typename Container::size_type size()
    {
        return c_.size();
    }
    typename Container::size_type empty()
    {
        return c_.empty();
    }

    template <class Key>
    typename Container::iterator find(const Key& key)
    {
        return c_.find(key);
    }
    template <class Key>
    typename Container::const_iterator find(const Key& key) const
    {
        return c_.find(key);
    }

    template <class Key, class Comparator>
    typename Container::iterator find(const Key& key, Comparator cmp)
    {
        return c_.find(key, cmp);
    }
    template <class Key, class Comparator>
    typename Container::const_iterator find(const Key& key, Comparator cmp) const
    {
        return c_.find(key, cmp);
    }
    //TODO add specific methods
    //map
    //  count
    //  equal_range
    //  lower_bound
    //  upper_bound
    //vec
    //  front
    //  back
    //  data
    //  operator[]

    typename Container::iterator begin()
    {
        return c_.begin();
    }
    typename Container::const_iterator begin() const
    {
        return c_.begin();
    }

    typename Container::iterator end()
    {
        return c_.end();
    }
    typename Container::const_iterator end() const
    {
        return c_.end();
    }
    operator const Container&() const
    {
        return c_;
    }
private:
    Container& c_;
};
