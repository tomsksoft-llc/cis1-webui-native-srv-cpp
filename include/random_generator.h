/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <random>

class random_generator
{
private:
    random_generator();
public:
    random_generator(const random_generator&) = delete;
    random_generator& operator=(const random_generator &) = delete;
    random_generator(random_generator &&) = delete;
    random_generator & operator=(random_generator &&) = delete;

    static std::mt19937& instance();
private:
    std::mt19937 gen_;
};
