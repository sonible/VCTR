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

namespace vctr::detail
{

template <class, class>
struct FunctionWithSignature : std::false_type
{
};

template <class Fn, class Ret, class... Args>
struct FunctionWithSignature<Fn, Ret (Args...)>
{
    static constexpr bool value = requires (Fn&& t, Ret (*fp) (Args...)) { fp = &t; };
};

template <class, class>
struct FunctionWithSignatureOrImplicitlyConvertible : std::false_type
{
};

template <class Fn, class Ret, class... Args>
struct FunctionWithSignatureOrImplicitlyConvertible<Fn, Ret (Args...)>
{
    static constexpr bool value = requires (Fn&& t, Args&&... args) { { t (args...) } -> std::convertible_to<Ret>; };
};

} // namespace vctr::detail

namespace vctr::is
{

/** Constrains Fn to be a function with an exact function signature.

    Example:
    @code
    template <is::functionWithSignature<bool (int)> Fn>
    void foo (Fn&& fn)
    @endcode

    The example code above will only accept callable types that have the exact
    signature specified. This means, e.g. these functions
    @code
    bool fn1 (const int&);
    const bool& fn2 (int);
    @endcode
    would not satisfy the constraint in the example above.
 */
template <class Fn, class Signature>
concept functionWithSignature = detail::FunctionWithSignature<Fn, Signature>::value;

/** Constrains Fn to be a function with the specified function signature or some signature with
    implicitly convertible alternatives to the specified arguments.

    Example:
    @code
    template <is::functionWithSignatureOrImplicitlyConvertible<bool (const int&)> Fn>
    void foo (Fn&& fn)
    @endcode

    The example code above will accept callable types like those
    @code
    bool fn1 (int);
    bool fn2 (const int&);
    const bool& fn3 (int);
    @endcode
 */
template <class Fn, class Signature>
concept functionWithSignatureOrImplicitlyConvertible = detail::FunctionWithSignatureOrImplicitlyConvertible<Fn, Signature>::value;

/** Constrains a type to be a function that takes a const ElementType& argument and returns a bool */
template <class T, class ElementType>
concept elementPredicateFunction = requires (T&& fn, const ElementType& e, bool r) { r = fn (e); };

} // namespace vctr::is
