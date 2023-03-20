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

namespace vctr::has
{

/** Constrains the type to define an operator += */
template <class Lhs, class Rhs = Lhs>
concept operatorPlusEquals = requires (Lhs l, Rhs r) { l += r; };

template <class T>
concept init = requires (T& t, void* ptr, size_t s) { t.init (ptr, s); };

}

namespace vctr::detail
{

// clang-format off
template <class T>
struct IsStdUniquePtr : std::false_type {};

template <class T, class D>
struct IsStdUniquePtr<std::unique_ptr<T, D>> : std::true_type {};

template <class T>
struct IsStdTuple : std::false_type {};

template <class... T>
struct IsStdTuple<std::tuple<T...>> : std::true_type {};
// clang-format on

} // namespace vctr::detail

namespace vctr::is
{

// clang-format off

/** Constrains a type to be trivially copyable */
template <class T>
concept triviallyCopyable = std::is_trivially_copyable_v<T>;

/** Constrains a type to be non const */
template <class T>
concept nonConst = ! std::is_const_v<T>;

/** Constrains a type to be a pointer of a reference to a pointer */
template <class T>
concept pointer = std::is_pointer_v<std::remove_cvref_t<T>>;

/** Constrains a type to be no pointer of a reference to a pointer */
template <class T>
concept noPointer = ! std::is_pointer_v<std::remove_cvref_t<T>>;

/** Constrains the type to be any instance of std::unique_ptr */
template <class T>
concept uniquePtr = detail::IsStdUniquePtr<T>::value;

/** Constrains the type to be any instance of std::tuple */
template <class T>
concept stdTuple = detail::IsStdTuple<T>::value;

template <class T, class... TupleTypes>
concept stdTupleWithTypes = std::same_as<T, std::tuple<TupleTypes...>>;

/** Constrains the type to be a class that defines a static const value field.

    Suitable classes are e.g. std::integral_constant and vctr::Constant.
 */
template <class C>
concept constant = requires (const void* ptr) { ptr = &C::value; };

/** Constrains the type to be a class that defines a static const value field of a certain type.

    Suitable classes are e.g. std::integral_constant and vctr::Constant.
 */
template <class C, class T>
concept constantWithType = std::same_as<T, std::remove_cvref_t<decltype (C::value)>>;

// clang-format on

} // namespace vctr::is

namespace vctr::are
{

/** Constrains a pack of elements to be the same type */
template <class T, class... U>
concept same = (std::same_as<T, U> || ...);

} // namespace vctr::are
