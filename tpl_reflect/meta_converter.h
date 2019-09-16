#pragma once

#include "tpl_helpers/ct_string.h"
#include "tpl_helpers/detect_idiom.h"

namespace reflect
{

template <class... T>
class fields_pack{};

template <class... T>
class validators_pack{};

template <class... CTStrings>
struct name_parts
{
    size_t size = sizeof...(CTStrings);
};

template <class... Args>
struct meta_converter;

template <class Type, class CTString>
struct ignored_field
{
    constexpr ignored_field(ignored_field&& f)
    {}

    constexpr ignored_field()
    {}

    constexpr ignored_field(std::tuple<>)
    {}

    template <class ProtocolEngine>
    static bool has(
            const typename ProtocolEngine::obj_type& protocol_obj)
    {
        return ProtocolEngine::has(
                protocol_obj,
                CTString::value);
    }

    template <class ProtocolEngine>
    static bool get(
            const typename ProtocolEngine::obj_type& protocol_obj,
            Type& obj)
    {
        return ProtocolEngine::get(
                protocol_obj,
                CTString::value);
    }

    template <class ProtocolEngine>
    static bool set(
            typename ProtocolEngine::obj_type& protocol_obj,
            const Type& obj,
            typename ProtocolEngine::set_context ctx)
    {
        ProtocolEngine::set(
                protocol_obj,
                CTString::value,
                ctx);
        return true; //just for paramater pack unfolding simplification
    }

    bool validate(
            const Type& obj) const
    {
        return true;
    }
};

template <
        class Type,
        class FieldType,
        FieldType Type::* Ptr,
        class CTString,
        class... Validators>
struct field
    : private Validators...
{
    constexpr field(field&& f)
        : Validators(std::move(static_cast<Validators&>(f)))...
    {}

	template <class Tuple>
    constexpr field(Tuple tup)
        : field(tup, std::make_index_sequence<std::tuple_size<Tuple>::value>{})
    {}

    template <class Tuple, size_t... Is>
    constexpr field(Tuple tup, std::index_sequence<Is...>)
        : Validators(std::get<Is>(tup))...
    {}

    template <class ProtocolEngine>
    static bool has(
            const typename ProtocolEngine::obj_type& protocol_obj)
    {
        return ProtocolEngine::has(
                protocol_obj,
                CTString::value);
    }

    template <class ProtocolEngine>
    static bool get(
            const typename ProtocolEngine::obj_type& protocol_obj,
            Type& obj)
    {
        auto& obj_field = obj.*Ptr;
        return ProtocolEngine::get(
                protocol_obj,
                CTString::value,
                obj_field);
    }

    template <class ProtocolEngine>
    static bool set(
            typename ProtocolEngine::obj_type& protocol_obj,
            const Type& obj,
            typename ProtocolEngine::set_context ctx)
    {
        const auto& obj_field = obj.*Ptr;
        ProtocolEngine::set(
                protocol_obj,
                CTString::value,
                obj_field,
                ctx);
        return true; //just for paramater pack unfolding simplification
    }

    bool validate(
            const Type& obj) const
    {
        return ((static_cast<const Validators*>(this)->operator()(obj.*Ptr)) && ...);
    }
};

template <
        class Type,
        class FnType,
        auto... Ptrs>
struct validator
    : private FnType
{
    constexpr validator(validator&& v)
        : FnType(std::move(static_cast<FnType&>(v)))
    {}

    constexpr validator(FnType fn)
        : FnType(fn)
    {}

    bool validate(
            const Type& obj) const
    {
        return FnType::operator()(obj.*Ptrs...);
    }
};

template <class Type, class... Fields, class... Validators, class NameParts>
struct meta_converter<
        Type,
        fields_pack<Fields...>,
        validators_pack<Validators...>,
        NameParts>
        : private Fields...
        , private Validators...
        , private NameParts
{
    constexpr meta_converter(
            Fields&&... field_args,
            Validators&&... validator_args)
		: Fields(std::move(field_args))...
        , Validators(std::move(validator_args))...
    {}

    template <
			class FieldsArgsTuple,
			class... ValidatorArgs>
    constexpr meta_converter(
            FieldsArgsTuple fields_args,
            ValidatorArgs... validator_args)
		: meta_converter(
				fields_args,
				std::make_index_sequence<std::tuple_size<FieldsArgsTuple>::value>{},
				validator_args...)
    {}

    template <
			class FieldsArgsTuple,
			size_t... Is,
			class... ValidatorArgs>
    constexpr meta_converter(
            FieldsArgsTuple fields_args,
            std::index_sequence<Is...>,
			ValidatorArgs... validator_args)
		: Fields(std::get<Is>(fields_args))...
        , Validators(validator_args)...
    {}

    template <class ProtocolEngine>
    static auto name()
    {
        return ProtocolEngine::name(NameParts{});
    }

    template <class ProtocolEngine>
    bool has(
            const typename ProtocolEngine::obj_type& protocol_obj) const
    {
        return (Fields::template has<ProtocolEngine>(protocol_obj) && ...);
    }

    template <class ProtocolEngine>
    bool get(
            const typename ProtocolEngine::obj_type& protocol_obj,
            Type& obj) const
    {
        return (Fields::template get<ProtocolEngine>(protocol_obj, obj) && ...);
    }

    template <class ProtocolEngine>
    bool set(
            typename ProtocolEngine::obj_type& protocol_obj,
            const Type& obj,
            typename ProtocolEngine::set_context ctx) const
    {
        return (Fields::template set<ProtocolEngine>(protocol_obj, obj, ctx) && ...);
    }

    bool validate(const Type& obj) const
    {
        return ((static_cast<const Fields*>(this)->validate(obj)) && ...)
            && ((static_cast<const Validators*>(this)->validate(obj)) && ...);
    }
};

template<typename T>
using converter_t = decltype( T::get_converter() );

template<typename T>
constexpr bool has_converter = meta::is_detected<converter_t, T>::value;

namespace detail
{

template <auto c>
struct ptr_v{};

template <class... Args>
struct meta_converter_impl;

template <class Type, class... Fields, class... Validators, class NameParts>
struct meta_converter_impl<
        Type,
        fields_pack<Fields...>,
        validators_pack<Validators...>,
        NameParts>
        : public Fields...
        , public Validators...
{
    constexpr meta_converter_impl(){}

    template <class... PrevFields, class... PrevValidators, class Entry>
    constexpr meta_converter_impl(
            meta_converter_impl<
                    Type,
                    fields_pack<PrevFields...>,
                    validators_pack<PrevValidators...>,
                    NameParts>&& prev,
            Entry&& entry)
        : PrevFields(std::move(static_cast<PrevFields&>(prev)))...
        , PrevValidators(std::move(static_cast<PrevValidators&>(prev)))...
        , Entry(std::move(entry))
    {}

    template <class PrevNameParts>
    constexpr meta_converter_impl(
            meta_converter_impl<
                    Type,
                    fields_pack<Fields...>,
                    validators_pack<Validators...>,
                    PrevNameParts>&& prev)
        : Fields(std::move(static_cast<Fields&>(prev)))...
        , Validators(std::move(static_cast<Validators&>(prev)))...
    {}

    template <class... CTStrings>
    constexpr meta_converter_impl<
            Type,
            fields_pack<Fields...>,
            validators_pack<Validators...>,
            name_parts<CTStrings...>> set_name(CTStrings...)
    {
        return {std::move(*this)};
    }

    template <
            class CTString,
            class FieldType,
            FieldType Type::* Ptr,
            class... FieldValidators>
    constexpr meta_converter_impl<
            Type,
            fields_pack<Fields...,
                    field<Type, FieldType, Ptr, CTString, FieldValidators...>>,
            validators_pack<Validators...>,
            NameParts> add_field(
            CTString ct_string,
            ptr_v<Ptr> ptr,
            FieldValidators... field_validators)
    {
        field<Type, FieldType, Ptr, CTString, FieldValidators...> f(
                std::make_tuple(field_validators...));
        return {std::move(*this), std::move(f)};
    }

    template <class CTString>
    constexpr meta_converter_impl<
            Type,
            fields_pack<Fields...,
                    ignored_field<Type, CTString>>,
            validators_pack<Validators...>,
            NameParts> add_field(
            CTString ct_string)
    {
        return {std::move(*this), ignored_field<Type, CTString>{}};
    }

    template <
            class Validator,
            auto Type::*... Ptrs>
    constexpr meta_converter_impl<
            Type,
            fields_pack<Fields...>,
            validators_pack<Validators...,
                    validator<Type, Validator, Ptrs...>
                    >,
            NameParts> add_validator(
            Validator fn,
            ptr_v<Ptrs>... ptrs)
    {
        /*
        static_assert(
                is_applicable<Validator, Args2...>::value,
                "meta_converter_impl::add_validator():"
                " argument types doesn't match field types");
        */
        validator<Type, Validator, Ptrs...> v(fn);
        return {std::move(*this), std::move(v)};
    }

    constexpr meta_converter<
            Type,
            fields_pack<Fields...>,
            validators_pack<Validators...>,
            NameParts> done()
    {
        return {std::move(static_cast<Fields&>(*this))...,
                std::move(static_cast<Validators&>(*this))...};
    }
};

} // namespace detail

template <auto c>
using ptr_v = detail::ptr_v<c>;

template <class T>
constexpr auto make_meta_converter()
{
    return detail::meta_converter_impl<
            T,
            fields_pack<>,
            validators_pack<>,
            name_parts<>>{};
}

} // namespace reflect
