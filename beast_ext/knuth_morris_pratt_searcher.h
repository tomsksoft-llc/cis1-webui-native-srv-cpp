#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <string_view>

class knuth_morris_pratt_searcher
{
public:
	void init(const std::string& pattern);

	void reset();

    //returns count of matched symbols
	size_t partial_search(char c);
private:
    size_t m_;
    size_t i_;
    std::string pattern_;
    std::string_view pattern_sv_; //prevent copy on substr
    std::vector<int> table_;

	void build_table(std::string pattern_sv_);
};
