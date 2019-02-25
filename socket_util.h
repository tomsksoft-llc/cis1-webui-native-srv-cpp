#pragma once

#include <boost/system/error_code.hpp>

namespace detail
{
    void set_cloexec_impl(int fd, boost::system::error_code& ec);
}

template <class T>
void set_cloexec(T& handle, boost::system::error_code& ec)
{
#ifdef __linux__
    int native_handle = handle.native_handle();
    detail::set_cloexec_impl(native_handle, ec);
#endif
}
