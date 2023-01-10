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
struct IsNumber : std::bool_constant<std::is_arithmetic_v<T>> {};

template <class T>
struct IsNumber<std::complex<T>> : std::bool_constant<std::is_arithmetic_v<T>> {};

template <class T>
struct IsComplex : std::false_type {};

template <class T>
struct IsComplex<std::complex<T>> : std::true_type {};
// clang-format on
} // namespace vctr::detail

namespace vctr::is
{

/** Constrains a type to represent a real valued or std::complex number type */
template <class T>
concept number = detail::IsNumber<T>::value;

/** Constrains a type to represent a real valued floating point number */
template <class T>
concept floatNumber = std::is_floating_point_v<std::remove_cvref_t<T>>;

/** Constrains a type to represent a real valued integer number */
template <class T>
concept intNumber = std::is_integral_v<std::remove_cvref_t<T>>;

/** Constrains a type to represent a real valued 32 bit integer number */
template <class T>
concept int32Number = intNumber<T> && sizeof (T) == 4;

/** Constrains a type to represent a real valued 64 bit integer number */
template <class T>
concept int64Number = intNumber<T> && sizeof (T) == 8;

/** Constrains a type to represent a real valued signed number (e.g. floatNumber or signedIntNumber) */
template <class T>
concept signedNumber = std::is_signed_v<std::remove_cvref_t<T>>;

/** Constrains a type to represent a real valued signed integer number */
template <class T>
concept signedIntNumber = intNumber<T> && signedNumber<T>;

/** Constrains a type to represent a real valued unsigned integer number */
// clang-format off
template <class T>
concept unsignedIntNumber = intNumber<T> && ! signedNumber<T>;
// clang-format on

/** Constrains a type to represent a real valued number */
template <class T>
concept realNumber = std::is_arithmetic_v<T>;

/** Constrains a type to represent a complex valued number (that means, any instance of std::complex) */
template <class T>
concept complexNumber = detail::IsComplex<std::remove_cvref_t<T>>::value;

/** Constrains a type to represent a complex valued floating point number (e.g. std::complex<float> or std::complex<double>) */
template <class T>
concept complexFloatNumber = complexNumber<T> && floatNumber<typename T::value_type>;

/** Constrains a type to represent a complex valued integer number */
template <class T>
concept complexIntNumber = complexNumber<T> && intNumber<typename T::value_type>;

/** Constrains a type to be of the type DisabledConstant */
template <class T>
concept disabledConstant = std::same_as<DisabledConstant, T>;

} // namespace vctr::is
