#include "random_generator.h"

#include <ctime>

random_generator::random_generator()
{
    gen_.seed(std::time(nullptr));
}

std::mt19937& random_generator::instance()
{
        static random_generator inst;
        return inst.gen_;
}
