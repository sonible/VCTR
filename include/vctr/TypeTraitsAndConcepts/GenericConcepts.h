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

// clang-format off
template <class T>
struct IsStdUniquePtr : std::false_type {};

template <class T, class D>
struct IsStdUniquePtr<std::unique_ptr<T, D>> : std::true_type {};
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

/** Constrains the type to be a wrapper class that defines a static const value field */
template <class T>
concept constantWrapper = requires (T, std::any a) { a = T::value; };

// clang-format on

} // namespace vctr::is

namespace vctr::are
{

/** Constrains a pack of elements to be the same type */
template <class T, class... U>
concept same = (std::same_as<T, U> || ...);

} // namespace vctr::are
