/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "bound_task_chain.h"

bound_task_chain<> make_async_chain(boost::asio::executor ex)
{
    return bound_task_chain{ex};
}
