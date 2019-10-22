#pragma once

#include <optional>

#include "sqlite_orm/sqlite_orm.h"

namespace sqlite_orm
{
    template<class T>
    struct type_printer<std::optional<T>> : public type_printer<T> {};

    template<>
    struct statement_binder<std::nullopt_t>
    {
        int bind(sqlite3_stmt *stmt, int index, const std::nullopt_t &)
        {
            return sqlite3_bind_null(stmt, index);
        }
    };

    template<class T>
    struct statement_binder<std::optional<T>>
    {
        int bind(sqlite3_stmt *stmt, int index, const std::optional<T> &value)
        {
            if(value)
            {
                return statement_binder<T>().bind(stmt, index, value.value());
            }
            else
            {
                return statement_binder<std::nullopt_t>().bind(stmt, index, std::nullopt);
            }
        }
    };

    template<>
    struct field_printer<std::nullopt_t>
    {
        std::string operator()(const std::nullopt_t &) const
        {
            return "null";
        }
    };

    template<class T>
    struct field_printer<std::optional<T>>
    {
        std::string operator()(const std::optional<T> &t) const
        {
            if(t)
            {
                return field_printer<T>()(*t);
            }
            else
            {
                return field_printer<std::nullopt_t>()(std::nullopt);
            }
        }
    };

    template<class T>
    struct row_extractor<std::optional<T>>
    {
        std::optional<T> extract(const char *row_value)
        {
            if(row_value)
            {
                return row_extractor<T>().extract(row_value);
            }
            else
            {
                return std::nullopt;
            }
        }

        std::optional<T> extract(sqlite3_stmt *stmt, int columnIndex)
        {
            auto str = sqlite3_column_text(stmt, columnIndex);
            return this->extract((const char*)str);
        }
    };

    template<class T>
    struct type_is_nullable<std::optional<T>>
        : public std::true_type
    {
        bool operator()(const std::optional<T> &t) const
        {
            return static_cast<bool>(t);
        }
    };
}

