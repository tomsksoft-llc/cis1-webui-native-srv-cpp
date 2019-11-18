/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "beast_ext/knuth_morris_pratt_searcher.h"

void knuth_morris_pratt_searcher::init(const std::string& pattern)
{
    if(pattern.size() < 2)
    {
        std::terminate();
    }
    if(pattern != pattern_)
    {
        table_.clear();
        pattern_ = pattern;
        pattern_sv_ = pattern_;
        build_table(pattern_);
    }
    reset();
}

void knuth_morris_pratt_searcher::reset()
{
    m_ = 0;
    i_ = 0;
}

size_t knuth_morris_pratt_searcher::partial_search(char c)
{
    size_t offset = m_ + i_;

    while(m_ + i_ - offset < 1)
    {
        if(pattern_sv_[i_] == c)
        {
            if(i_ == pattern_sv_.length() - 1)
            {
                return i_ + 1;
            }
            i_ += 1;
        }
        else
        {
            m_ += i_ - table_[i_];
            if(table_[i_] > -1)
            {
                i_ = table_[i_];
            }
            else
            {
                i_ = 0;
            }
        }
    }
    return i_;
}

void knuth_morris_pratt_searcher::build_table(const std::string& pattern_sv_)
{
    int pos = 2;
    int condition = 0;

    table_.resize(pattern_sv_.size() + 1);

    if(pattern_sv_.length() > 2)
    {
        table_[pattern_sv_.length()] = 0;
    }
    else
    {
        table_[2] = 0;
    }

    table_[0] = -1;
    table_[1] = 0;

    while(pos < pattern_sv_.length())
    {
        if(pattern_sv_.substr(pos - 1,1) == pattern_sv_.substr(condition, 1))
        {
            condition += 1;
            table_[pos] = condition;
            pos += 1;
        }
        else if(condition > 0)
        {
            condition = table_[condition];
        }
        else
        {
            table_[pos] = 0;
            pos += 1;
        }
    }
}
