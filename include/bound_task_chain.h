#pragma once

#include <memory>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/post.hpp>

#include "tpl_helpers/detect_idiom.h"

namespace thennable
{

struct callback_arg_t{};

constexpr const callback_arg_t callback;

} // namespace thennable

template <typename T>
using get_executor_t = decltype(std::declval<T>().get_executor());

template <typename T>
constexpr bool has_bound_executor = meta::is_detected<get_executor_t, T>::value;

template <typename T>
using is_async_t = decltype(std::declval<T>().is_async_task());

template <typename T>
constexpr bool is_wrapped_async = meta::is_detected<is_async_t, T>::value;

template <class T>
struct async_task_wrapper
{
    async_task_wrapper(T&& t, boost::asio::executor ex)
        : t_(std::move(t))
        , ex_(ex)
    {}

    auto& get()
    {
        return t_;
    }

    auto get_executor() const
    {
        return ex_;
    }

    constexpr bool is_async_task() const
    {
        return true;
    }
private:
    T t_;
    boost::asio::executor ex_;
};

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
    boost::asio::post(e,
            [
            &,
            self = this->shared_from_this(),
            packed_args = std::make_tuple(std::forward<ContinuationArgs>(args)...)
            ]() mutable
            {
                auto& bound_fn = std::get<N>(fn_chain_).get();
                try
                {
                    if constexpr(is_wrapped_async<decltype(std::get<N>(fn_chain_))>)
                    {
                        if constexpr(N > 0)
                        {
                            std::apply(
                                    [&](auto&&... args1)
                                    {
                                        bound_fn(std::forward<decltype(args1)>(
                                                args1)...,
                                                [&, self = self->shared_from_this()]
                                                (auto&&... args2)
                                                {
                                                    this->template call<N-1>(
                                                            std::forward<decltype(args2)>(
                                                                    args2)...);
                                                });
                                    },
                                    std::move(packed_args));
                        }
                        else
                        {
                            std::apply(
                                    [&](auto&&... args1)
                                    {
                                        bound_fn(std::forward<decltype(args1)>(
                                                args1)...,
                                                [self = this->shared_from_this()]
                                                (auto&&... args2){});
                                    },
                                    std::move(packed_args));
                        }
                    }
                    else if constexpr(!std::is_same<
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
    [[nodiscard]] auto then(F&& f, thennable::callback_arg_t)
    {
        if constexpr(has_bound_executor<F>)
        {
            return then_impl(
                    async_task_wrapper{
                            std::move(f.get()),
                            f.get_executor()},
                    Indices{});
        }
        else
        {
            return then_impl(
                    async_task_wrapper{
                            std::move(f),
                            ex_},
                    Indices{});
        }
    }

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

bound_task_chain<> make_async_chain(boost::asio::executor ex);

template <class Continuation>
struct make_async_task_t
{
    using continuation = Continuation;
    using cb = void(std::function<continuation>&&);
    using task = async_task_wrapper<std::function<cb>>;
};
