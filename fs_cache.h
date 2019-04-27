#pragma once

#include <filesystem>

class fs_cache;

class fs_cache_entry
{
public:
    class const_iterator
    {
    public:
        const_iterator(
                const fs_cache_entry* parent,
                const fs_cache_entry* pos);
        explicit const_iterator(
                const fs_cache_entry* parent);
        bool operator!=(const const_iterator& other) const;
        const_iterator& operator++();
        const fs_cache_entry& operator*() const;
        const fs_cache_entry* operator->() const;
    private:
        const fs_cache_entry* parent_entry_;
        const fs_cache_entry* entry_;
    };
    class iterator
    {
    public:
        iterator(
                fs_cache_entry* parent,
                fs_cache_entry* pos);
        explicit iterator(
                fs_cache_entry* parent);
        bool operator!=(const iterator& other) const;
        iterator& operator++();
        fs_cache_entry& operator*();
        fs_cache_entry* operator->();
        const fs_cache_entry& operator*() const;
        const fs_cache_entry* operator->() const;
    private:
        fs_cache_entry* parent_entry_;
        fs_cache_entry* entry_;
    };

    friend class fs_cache;

    explicit fs_cache_entry(std::filesystem::path dir);
    explicit fs_cache_entry(std::filesystem::directory_entry entry);

    void refresh();

    const std::filesystem::directory_entry& get() const;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
private:
    std::filesystem::directory_entry entry_;
    std::vector<fs_cache_entry> childs_;
    void load_childs();
};

class fs_cache
{
public:
    explicit fs_cache(std::filesystem::path root_dir);

    void refresh();

    fs_cache_entry::iterator begin();
    fs_cache_entry::iterator end();

    fs_cache_entry::const_iterator begin() const;
    fs_cache_entry::const_iterator end() const;
private:
    fs_cache_entry root_;
};
