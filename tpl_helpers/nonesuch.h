#pragma once

namespace meta
{

struct nonesuch {
    ~nonesuch() = delete;
    nonesuch(nonesuch const&) = delete;
    void operator=(nonesuch const&) = delete;
};

} // namespace meta
