/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2022- by sonible GmbH.

    This file is part of VCTR - Versatile Container Templates Reconceptualized.

    VCTR is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    VCTR is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License version 3 for more details.

    You should have received a copy of the GNU Lesser General Public License
    version 3 along with VCTR.  If not, see <https://www.gnu.org/licenses/>.
  ==============================================================================
*/

#pragma once

#include <string_view>
#include <cstdint>

//==============================================================================
// Todo: There is quite a bit of repeated, non-generic code in here. It works for
//  our purpose like this, but we would be happy cleaning this up and making it
//  more generic. Contributions that address this are highly welcome!
//==============================================================================

namespace vctr
{
/** Returns a string containing the templates type name */
// clang-format off
template <class T> consteval std::string_view typeName();
template <>        consteval std::string_view typeName<void>()     { return "void"; }
template <>        consteval std::string_view typeName<int8_t>()   { return "int8_t"; }
template <>        consteval std::string_view typeName<uint8_t>()  { return "uint8_t"; }
template <>        consteval std::string_view typeName<int16_t>()  { return "int16_t"; }
template <>        consteval std::string_view typeName<uint16_t>() { return "uint16_t"; }
template <>        consteval std::string_view typeName<int32_t>()  { return "int32_t"; }
template <>        consteval std::string_view typeName<uint32_t>() { return "uint32_t"; }
template <>        consteval std::string_view typeName<int64_t>()  { return "int64_t"; }
template <>        consteval std::string_view typeName<uint64_t>() { return "uint64_t"; }
// clang-format on

namespace detail
{

template <class T>
consteval std::string_view typeNameWrappedInFunctionTemplateName()
{
#if VCTR_MSVC
    return __FUNCSIG__;
#else
    return __PRETTY_FUNCTION__;
#endif
}

template <class R, class Arg, R (*t) (Arg)>
consteval std::string_view functionNameWrappedInFunctionTemplateName()
{
#if VCTR_MSVC
    return __FUNCSIG__;
#else
    return __PRETTY_FUNCTION__;
#endif
}

template <class R, class ArgA, class ArgB, R (*t) (ArgA, ArgB)>
consteval std::string_view functionNameWrappedInFunctionTemplateName()
{
#if VCTR_MSVC
    return __FUNCSIG__;
#else
    return __PRETTY_FUNCTION__;
#endif
}

consteval size_t wrappedTypeNamePrefixLength()
{
    // We take void as a test type to wrap in the function and see where we actually
    // find it in the generated string
    return typeNameWrappedInFunctionTemplateName<void>().find (typeName<void>());
}

consteval size_t wrappedTypeNameSuffixLength()
{
    return typeNameWrappedInFunctionTemplateName<void>().length() - wrappedTypeNamePrefixLength() - typeName<void>().length();
}

} // namespace detail

template <class T>
consteval std::string_view typeName()
{
    constexpr auto wrappedName = detail::typeNameWrappedInFunctionTemplateName<T>();
    constexpr auto prefixLength = detail::wrappedTypeNamePrefixLength();
    constexpr auto suffixLength = detail::wrappedTypeNameSuffixLength();
    constexpr auto typeNameLength = wrappedName.length() - prefixLength - suffixLength;

    return wrappedName.substr (prefixLength, typeNameLength);
}

template <class T>
consteval std::string_view typeName (const T&)
{
    return typeName<T>();
}

/** Returns the name for a function with a single argument */
template <class R, class Arg, R (*fn) (Arg)>
consteval std::string_view functionName()
{
    constexpr auto wrappedName = detail::functionNameWrappedInFunctionTemplateName<R, Arg, fn>();
#if VCTR_MSVC
    constexpr auto prefixLen = wrappedName.find ("functionNameWrappedInFunctionTemplateName<") + 42;
    constexpr auto postfixLen = wrappedName.rfind (">(void)");
    constexpr auto onlyTemplateArgs = wrappedName.substr (prefixLen, postfixLen - prefixLen);

    constexpr auto returnTypeName = typeName<R>();
    constexpr auto returnTypeLen = onlyTemplateArgs.find (returnTypeName) + returnTypeName.size() + 1;
    constexpr auto onlyTemplateArgsNoReturnType = onlyTemplateArgs.substr (returnTypeLen);

    constexpr auto argTypeName = typeName<Arg>();
    constexpr auto argTypeLen = onlyTemplateArgsNoReturnType.find (argTypeName) + argTypeName.size() + 1;
    constexpr auto onlyFunctionSignature = onlyTemplateArgsNoReturnType.substr (argTypeLen);

    constexpr auto argPos = onlyFunctionSignature.find ("(");
    constexpr auto functionNameLen = argPos - returnTypeLen;

    return onlyFunctionSignature.substr (returnTypeLen, functionNameLen);
#else
    constexpr auto prefixLength = wrappedName.find ("&") + 1;
    constexpr auto postfixPos = wrappedName.rfind ("]");
    constexpr auto functionNameLength = postfixPos - prefixLength;
    return wrappedName.substr (prefixLength, functionNameLength);
#endif
}

/** Returns the name for a function with two arguments */
template <class R, class ArgA, class ArgB, R (*fn) (ArgA, ArgB)>
consteval std::string_view functionName()
{
    constexpr auto wrappedName = detail::functionNameWrappedInFunctionTemplateName<R, ArgA, ArgB, fn>();

#if VCTR_MSVC
    constexpr auto prefixLen = wrappedName.find ("functionNameWrappedInFunctionTemplateName<") + 42;
    constexpr auto postfixLen = wrappedName.rfind (">(void)");
    constexpr auto onlyTemplateArgs = wrappedName.substr (prefixLen, postfixLen - prefixLen);

    constexpr auto returnTypeName = typeName<R>();
    constexpr auto returnTypeLen = onlyTemplateArgs.find (returnTypeName) + returnTypeName.size() + 1;
    constexpr auto onlyTemplateArgsNoReturnType = onlyTemplateArgs.substr (returnTypeLen);

    constexpr auto argATypeName = typeName<ArgA>();
    constexpr auto argATypeLen = onlyTemplateArgsNoReturnType.find (argATypeName) + argATypeName.size() + 1;
    constexpr auto onlyTemplateArgsNoArgA = onlyTemplateArgsNoReturnType.substr (argATypeLen);

    constexpr auto argBTypeName = typeName<ArgB>();
    constexpr auto argBTypeLen = onlyTemplateArgsNoReturnType.find (argBTypeName) + argBTypeName.size() + 1;
    constexpr auto onlyFunctionSignature = onlyTemplateArgsNoReturnType.substr (argBTypeLen);

    constexpr auto argPos = onlyFunctionSignature.find ("(");
    constexpr auto functionNameLen = argPos - returnTypeLen;

    return onlyFunctionSignature.substr (returnTypeLen, functionNameLen);
#else
    constexpr auto prefixLength = wrappedName.find ("&") + 1;
    constexpr auto postfixPos = wrappedName.rfind ("]");
    constexpr auto functionNameLength = postfixPos - prefixLength;
    return wrappedName.substr (prefixLength, functionNameLength);
#endif
}

} // namespace vctr
