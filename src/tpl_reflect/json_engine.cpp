#include "tpl_reflect/json_engine.h"

namespace json
{

bool engine::has(
        const obj_type& protocol_obj,
        const char* str)
{
    return protocol_obj.HasMember(str);
}

void engine::set(
        obj_type& protocol_obj,
        const char* str,
        set_context ctx)
{
    protocol_obj.AddMember(
            rapidjson::Value().SetString(
                    str,
                    strlen(str),
                    ctx),
            rapidjson::Value().SetObject(),
            ctx);
}

bool engine::get(
        const obj_type& protocol_obj,
        const char* str)
{
    return protocol_obj[str].IsObject();
}

template <>
bool get<bool>(const rapidjson::Value& from, bool& to)
{
    if(from.IsBool())
    {
        to = from.GetBool();
        return true;
    }

    return false;
}

template <>
bool get<int32_t>(const rapidjson::Value& from, int32_t& to)
{
    if(from.IsInt())
    {
        to = from.GetInt();
        return true;
    }

    return false;
}

template <>
bool get<uint64_t>(const rapidjson::Value& from, uint64_t& to)
{
    if(from.IsUint64())
    {
        to = from.GetUint64();
        return true;
    }

    return false;
}

template <>
bool get<std::string>(const rapidjson::Value& from, std::string& to)
{
    if(from.IsString())
    {
        to = from.GetString();
        return true;
    }

    return false;
}

template <>
void set<bool>(
        rapidjson::Value& to,
        const bool& from,
        engine::set_context allocator)
{
    to.SetBool(from);
}

template <>
void set<int32_t>(
        rapidjson::Value& to,
        const int32_t& from,
        engine::set_context allocator)
{
    to.SetInt(from);
}

template <>
void set<uint64_t>(
        rapidjson::Value& to,
        const uint64_t& from,
        engine::set_context allocator)
{
    to.SetUint64(from);
}

template <>
void set<std::string>(
        rapidjson::Value& to,
        const std::string& from,
        engine::set_context allocator)
{
    to.SetString(
            from.c_str(),
            from.length(),
            allocator);
}

} // namespace json
