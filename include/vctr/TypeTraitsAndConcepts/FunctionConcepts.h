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

namespace vctr::is
{

/** Constrains a type to be a function with return type void that can take an argument by reference */
template <class T, class ElementType>
concept voidForEachFunction = requires (T&& fn, ElementType& e) { fn (e); } &&
                              std::is_void_v<std::invoke_result_t<T, ElementType&>>;

/** Constrains a type to be a function with return type void that can take an argument by const reference */
template <class T, class ElementType>
concept voidConstForEachFunction = requires (T&& fn, const ElementType& e) { fn (e); } &&
                                   std::is_void_v<std::invoke_result_t<T, ElementType&>>;

/** Constrains a type to be a function with return type ElementType that can take an argument by const reference */
template <class T, class ElementType>
concept returningForEachFunction = requires (T&& fn, const ElementType& e, ElementType r) { r = fn (e); };

/** Same as voidForEachFunction but with the option to specify additional function arguments */
template <class T, class ElementType, class... Args>
concept voidForEachFunctionWithArgs = requires (T&& fn, ElementType& e, Args&&... args) { fn (e, args...); } &&
                                      std::is_void_v<std::invoke_result_t<T, ElementType&, Args...>>;

/** Same as voidConstForEachFunction but with the option to specify additional function arguments */
template <class T, class ElementType, class... Args>
concept voidConstForEachFunctionWithArgs = requires (T&& fn, const ElementType& e, Args&&... args) { fn (e, args...); } &&
                                           std::is_void_v<std::invoke_result_t<T, ElementType&, Args...>>;

/** Same as returningForEachFunction but with the option to specify additional function arguments */
template <class T, class ElementType, class... Args>
concept returningForEachFunctionWithArgs = requires (T&& fn, const ElementType& e, ElementType r, Args&&... args) { r = fn (e, args...); };

/** Constrains a type to be a function that takes a const ElementType& argument and returns a bool */
template <class T, class ElementType>
concept elementPredicateFunction = requires (T&& fn, const ElementType& e, bool r) { r = fn (e); };

} // namespace vctr::is
