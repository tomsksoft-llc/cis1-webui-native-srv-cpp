#pragma once

#include <optional>
#include <vector>
#include <map>
#include <variant>

#include <rapidjson/document.h>

#include "meta_converter.h"
#include "tpl_helpers/for_types.h"

namespace json
{

struct engine
{
    using obj_type = rapidjson::Value;

    using set_context = rapidjson::Document::AllocatorType&;

    template <class... CTStrings>
    static std::string name(reflect::name_parts<CTStrings...>);

    static bool has(
            const obj_type& protocol_obj,
            const char* str);

    template <class FieldType>
    static bool get(
            const obj_type& protocol_obj,
            const char* str,
            FieldType& obj_field);

    template <class FieldType>
    static bool get(
            const obj_type& protocol_obj,
            const char* str,
            std::optional<FieldType>& obj_field);

    template <class FieldType>
    static bool get(
            const obj_type& protocol_obj,
            const char* str,
            std::vector<FieldType>& obj_field);

    template <class FieldType>
    static bool get(
            const obj_type& protocol_obj,
            const char* str,
            std::map<std::string, FieldType>& obj_field);

    template <class... FieldTypes>
    static bool get(
            const obj_type& protocol_obj,
            const char* str,
            std::variant<std::monostate, FieldTypes...>& obj_field);

    static bool get(
            const obj_type& protocol_obj,
            const char* str);

    template <class FieldType>
    static void set(
            obj_type& protocol_obj,
            const char* str,
            const FieldType& obj_field,
            set_context ctx);

    template <class FieldType>
    static void set(
            obj_type& protocol_obj,
            const char* str,
            const std::optional<FieldType>& obj_field,
            set_context ctx);

    template <class FieldType>
    static void set(
            obj_type& protocol_obj,
            const char* str,
            const std::vector<FieldType>& obj_field,
            set_context ctx);

    template <class FieldType>
    static void set(
            obj_type& protocol_obj,
            const char* str,
            const std::map<std::string, FieldType>& obj_field,
            set_context ctx);

    template <class... FieldTypes>
    static void set(
            obj_type& protocol_obj,
            const char* str,
            const std::variant<std::monostate, FieldTypes...>& obj_field,
            set_context ctx);

    static void set(
            obj_type& protocol_obj,
            const char* str,
            set_context ctx);
};

template <class T>
bool get(const rapidjson::Value& from, T& to)
{
	auto conv = to.get_converter();
	return conv.template get<engine>(from, to);
}

template <class T>
void set(
        rapidjson::Value& to,
        const T& from,
        engine::set_context ctx)
{
	to.SetObject();
	from.get_converter().template set<engine>(to, from, ctx);
}

template <class T>
void set(
        rapidjson::Value& to,
        const char* field_name,
        const T& from,
        engine::set_context ctx)
{
	rapidjson::Value v;
    set(v, from, ctx);
    to.AddMember(
            rapidjson::Value().SetString(
                    field_name,
                    strlen(field_name),
                    ctx),
            v.Move(),
            ctx);
}

template <class... CTStrings>
std::string engine::name(reflect::name_parts<CTStrings...>)
{
    std::string str;

    str.reserve((CTStrings::size + ...) + sizeof...(CTStrings));

    ((str.append(CTStrings::value), str.append(".")), ...);

    str.pop_back();

    return str;
}

template <class FieldType>
bool engine::get(
        const obj_type& protocol_obj,
        const char* str,
        FieldType& obj_field)
{
    if(!has(protocol_obj, str))
    {
        return false;
    }

    auto& field = protocol_obj[str];
    return json::get(field, obj_field);
}

template <class FieldType>
bool engine::get(
        const obj_type& protocol_obj,
        const char* str,
        std::optional<FieldType>& obj_field)
{
    if(!has(protocol_obj, str))
    {
        return false;
    }

    auto& field = protocol_obj[str];
    
    if(field.IsNull())
    {
        return true;
    }
    else
    {
        return json::get(field, obj_field);
    }
}

template <class FieldType>
bool engine::get(
        const obj_type& protocol_obj,
        const char* str,
        std::vector<FieldType>& obj_field)
{
    if(protocol_obj[str].IsArray())
    {
        for(const auto& param : protocol_obj[str].GetArray())
        {
            FieldType arr_obj;

            if constexpr (reflect::has_converter<FieldType>)
            {
                if(!param.IsObject() || !FieldType::get_converter().template get<engine>(param, arr_obj))
                {
                    return false;
                }
            }
            else if (!json::get(param, arr_obj))
            {
                return false;
            }

            obj_field.push_back(arr_obj);
        }
        return true;
    }

    return false;
}

template <class FieldType>
bool engine::get(
        const obj_type& protocol_obj,
        const char* str,
        std::map<std::string, FieldType>& obj_field)
{
    if(protocol_obj[str].IsObject())
    {
        for(const auto& [key, value] : protocol_obj[str].GetObject())
        {
            if(key.IsString())
            {
                FieldType map_obj;
                if constexpr (reflect::has_converter<FieldType>)
                {
                    if(!value.IsObject() || !FieldType::get_converter().template get<engine>(value, map_obj))
                    {
                        return false;
                    }
                }
                else if(!json::get(value, map_obj))
                {
                    return false;
                }

                obj_field.insert({key.GetString(), map_obj});
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    return false;
}

template <class... FieldTypes>
bool engine::get(
        const obj_type& protocol_obj,
        const char* str,
        std::variant<std::monostate, FieldTypes...>& obj_field)
{
    std::string type_str = str;
    type_str += "_type";

    if(!has(protocol_obj, str) || !has(protocol_obj, type_str.c_str()))
    {
        return false;
    }

    auto& type_field = protocol_obj[type_str.c_str()];
    
    std::string type;

    if(!json::get(type_field, type))
    {
        return false;
    }

    auto& field = protocol_obj[str];

    if(type == "none")
    {
        return field.IsNull();
    }

    if(!field.IsObject())
    {
        return false;
    }
    
    //TODO add support to basic types

    bool result = false;
    meta::for_types<FieldTypes...>(
            [&](auto type_tag)
            {
                using FieldType = typename decltype(type_tag)::type;

                if(FieldType::get_name().value == type)
                {
                    obj_field = FieldType{};
                    result = FieldType::get_converter().template get<engine>(
                            field,
                            std::get<FieldType>(obj_field));

                    return false;
                }

                return true;
            });

    return result;
}

template <class FieldType>
void engine::set(
        obj_type& protocol_obj,
        const char* str,
        const FieldType& obj_field,
        set_context ctx)
{
    return json::set(protocol_obj, str, obj_field, ctx);
}

template <class FieldType>
void engine::set(
        obj_type& protocol_obj,
        const char* str,
        const std::optional<FieldType>& obj_field,
        set_context ctx)
{
    if(obj_field)
    {
        return json::set(protocol_obj, str, obj_field.value(), ctx);
    }
    else
    {
        rapidjson::Value v;
        v.SetNull();
        protocol_obj.AddMember(
                rapidjson::Value().SetString(
                        str,
                        strlen(str),
                        ctx),
                v.Move(),
                ctx);
    }
}

template <class FieldType>
void engine::set(
        obj_type& protocol_obj,
        const char* str,
        const std::vector<FieldType>& obj_field,
        set_context ctx)
{
    rapidjson::Value v;
    v.SetArray();
    for(const auto& obj : obj_field)
    {
        rapidjson::Value vv;
        if constexpr (reflect::has_converter<FieldType>)
        {
            vv.SetObject();
            FieldType::get_converter().template set<engine>(vv, obj, ctx);
        }
        else
        {
            json::set(vv, obj, ctx);
        }
        v.PushBack(vv.Move(), ctx);
    }
    protocol_obj.AddMember(
            rapidjson::Value().SetString(
                    str,
                    strlen(str),
                    ctx),
            v.Move(),
            ctx);
}

template <class FieldType>
void engine::set(
        obj_type& protocol_obj,
        const char* str,
        const std::map<std::string, FieldType>& obj_field,
        set_context ctx)
{
    rapidjson::Value v;
    v.SetObject();
    for(const auto& [key, value] : obj_field)
    {
        rapidjson::Value vv;
        if constexpr(reflect::has_converter<FieldType>)
        {
            vv.SetObject();
            FieldType::get_converter().template set<engine>(vv, value, ctx);
        }
        else
        {
            json::set(vv, value, ctx);
        }
        v.AddMember(
                rapidjson::Value().SetString(
                        key.c_str(),
                        key.length(),
                        ctx),
                vv.Move(),
                ctx);
    }
    protocol_obj.AddMember(
            rapidjson::Value().SetString(
                    str,
                    strlen(str),
                    ctx),
            v.Move(),
            ctx);
}

template <class... FieldTypes>
void engine::set(
        obj_type& protocol_obj,
        const char* str,
        const std::variant<std::monostate, FieldTypes...>& obj_field,
        set_context ctx)
{
    std::string type_str = str; 
    type_str += "_type";
    
    rapidjson::Value type;

    rapidjson::Value v;
    std::visit(
            [&](auto&& arg)
            {
                using FieldType = std::decay_t<decltype(arg)>;

                if constexpr(std::is_same_v<FieldType, std::monostate>)
                {
                    type.SetString("none");
                    v.SetNull();
                }
                else if(reflect::has_converter<FieldType>)
                {
                    type.SetString(FieldType::get_name().value);
                    v.SetObject();
                    FieldType::get_converter().template set<engine>(v, arg, ctx);
                }
                else
                {
                    //TODO add support to basic types
                }
            },
            obj_field);
    
    protocol_obj.AddMember(
        rapidjson::Value().SetString(
                type_str.c_str(),
                type_str.length(),
                ctx),
        type.Move(),
        ctx);

    protocol_obj.AddMember(
        rapidjson::Value().SetString(
                str,
                strlen(str),
                ctx),
        v.Move(),
        ctx);
}

template <>
bool get<bool>(const rapidjson::Value& from, bool& to);

template <>
bool get<int32_t>(const rapidjson::Value& from, int32_t& to);

template <>
bool get<uint64_t>(const rapidjson::Value& from, uint64_t& to);

template <>
bool get<std::string>(const rapidjson::Value& from, std::string& to);

template <>
void set<bool>(
        rapidjson::Value& to,
        const bool& from,
        engine::set_context allocator);

template <>
void set<int32_t>(
        rapidjson::Value& to,
        const int32_t& from,
        engine::set_context allocator);

template <>
void set<uint64_t>(
        rapidjson::Value& to,
        const uint64_t& from,
        engine::set_context allocator);

template <>
void set<std::string>(
        rapidjson::Value& to,
        const std::string& from,
        engine::set_context allocator);

} // namespace json
