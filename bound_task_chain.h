#pragma once

#include <memory>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/executor.hpp>

#include "tpl_helpers/detect_idiom.h"

template <typename T>
using get_executor_t = decltype(std::declval<T>().get_executor());

template <typename T>
constexpr bool has_bound_executor = meta::is_detected<get_executor_t, T>::value;

template <class... Args>
class bound_task_chain_impl
    : public std::enable_shared_from_this<bound_task_chain_impl<Args...>>
{
public:
    bound_task_chain_impl(
            std::tuple<Args...>&& fn_chain,
            std::function<void(const std::error_code&)>&& error_handler)
        : fn_chain_(std::move(fn_chain))
        , error_handler_(std::move(error_handler))
    {}

    bound_task_chain_impl(const bound_task_chain_impl&) = delete;
    bound_task_chain_impl(bound_task_chain_impl&&) = delete;

    template <size_t N, class... ContinuationArgs>
    void call(ContinuationArgs&&... args);
private:
    std::tuple<Args...> fn_chain_;
    std::function<void(const std::error_code&)> error_handler_;
};

template <class... Args>
template <size_t N, class... ContinuationArgs>
void bound_task_chain_impl<Args...>::call(ContinuationArgs&&... args)
{
    auto e = std::get<N>(fn_chain_).get_executor();
    post(e,
            [
            &,
            self = this->shared_from_this(),
            packed_args = std::make_tuple(std::forward<ContinuationArgs>(args)...)
            ]() mutable
            {
                auto& bound_fn = std::get<N>(fn_chain_).get();
                try
                {
                    if constexpr(!std::is_same<
                            decltype(std::apply(bound_fn, std::move(packed_args))),
                            void>::value)
                    {
                        if constexpr(N > 0)
                        {
                            call<N - 1>(std::apply(bound_fn, std::move(packed_args)));
                        }
                        else
                        {
                            std::apply(bound_fn, std::move(packed_args));
                        }
                    }
                    else
                    {
                        std::apply(bound_fn, std::move(packed_args));

                        if constexpr(N > 0)
                        {
                            call<N - 1>();
                        }
                    }
                }
                catch(std::error_code ec)
                {
                    return error_handler_(ec);
                }
            });
}

template <class... Args>
class bound_task_chain
{
public:
    bound_task_chain(boost::asio::executor ex, Args... args)
        : ex_(ex)
        , fn_chain_(args...)
    {}

    bound_task_chain(const bound_task_chain&) = delete;

    template <class F, typename Indices = std::make_index_sequence<sizeof...(Args)>>
    [[nodiscard]] auto then(F&& f)
    {
        if constexpr(has_bound_executor<F>)
        {
            return then_impl(std::forward<F>(f), Indices{});
        }
        else
        {
            return then_impl(boost::asio::bind_executor(ex_, std::forward<F>(f)), Indices{});
        }
    }

    template <class F>
    bound_task_chain& on_error(F&& f)
    {
        error_handler_ = std::forward<F>(f);
        return *this;
    }

    template <class... InitialArgs>
    void run(InitialArgs&&... args)
    {
        static_assert(sizeof...(Args) > 0, "chain should have at least one element to run");
        std::make_shared<bound_task_chain_impl<Args...>>(
                std::move(fn_chain_),
                std::move(error_handler_))->template call<sizeof...(Args) - 1, InitialArgs...>(
                        std::forward<InitialArgs>(args)...);
    }

private:
    boost::asio::executor ex_;
    std::tuple<Args...> fn_chain_;
    std::function<void(const std::error_code&)> error_handler_ =
            [](const std::error_code& ec){throw ec;};

    template <class F, std::size_t... Is>
    bound_task_chain<F, Args...> then_impl(F&& f, std::index_sequence<Is...>)
    {
        return {ex_, std::forward<F>(f), std::get<Is>(fn_chain_)...};
    }
};

auto make_async_chain(boost::asio::executor ex)
{
    return bound_task_chain{ex};
}
