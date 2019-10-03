#pragma once

#include <memory>
#include <type_traits>

#include <transaction.h>

template <class T>
class is_smart_ptr
    : public std::false_type
{};

template <class T>
class is_smart_ptr<std::unique_ptr<T>>
    : public std::true_type
{};

template <class T>
class is_smart_ptr<std::shared_ptr<T>>
    : public std::true_type
{};

template <class T>
class is_transaction
    : public std::false_type
{};

template <>
class is_transaction<transaction>
    : public std::true_type
{};

namespace websocket
{

template <class T, class Enable = void>
class transaction_handle;

template <class T>
class transaction_handle<
        T,
        typename std::enable_if<is_smart_ptr<T>::value
                             && is_transaction<typename std::decay<decltype(
                                        *std::declval<T>())>::type>::value>::type>
{
public:
    template <class U>
    transaction_handle(U&& resource)
        : handle_(std::forward<U>(resource))
    {}

    template <class Payload>
    void send(const Payload& p) const
    {
        handle_->send(p);
    }

    template <class Payload>
    void send_error(const Payload& p, const std::string& err) const
    {
        handle_->send_error(p, err);
    }

    void send_error(const std::string& err) const
    {
        handle_->send_error(err);
    }

    std::optional<boost::asio::executor> get_executor() const
    {
        return handle_->get_executor();
    }

private:
    T handle_;
};

template <class T>
class transaction_handle<
        T,
        typename std::enable_if<!is_smart_ptr<T>::value
                             && is_transaction<T>::value>::type>
{
public:
    template <class U>
    transaction_handle(U&& resource)
        : handle_(std::forward<U>(resource))
    {}

    template <class Payload>
    void send(const Payload& p) const
    {
        handle_.send(p);
    }

    template <class Payload>
    void send_error(const Payload& p, const std::string& err) const
    {
        handle_.send_error(p, err);
    }

    void send_error(const std::string& err) const
    {
        handle_.send_error(err);
    }

    std::optional<boost::asio::executor> get_executor() const
    {
        return handle_.get_executor();
    }

private:
    T handle_;
};

template <class U>
transaction_handle(U&& resource) -> transaction_handle<U>;

} // namespace websocket
