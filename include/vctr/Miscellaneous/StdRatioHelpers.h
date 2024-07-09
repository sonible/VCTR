/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2024- by sonible GmbH.

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

namespace vctr::detail
{

template <class Ratio>
struct ReducedToLowestFormHelper
{
    static constexpr auto gcd = std::gcd (Ratio::num, Ratio::den);

    using Type = std::ratio<Ratio::num / gcd, Ratio::den / gcd>;
};

/** Reduces a std::ratio to its lowest form */
template <is::stdRatio Ratio>
using ReducedToLowestForm = typename ReducedToLowestFormHelper<Ratio>::Type;

/** Multiplies a std::ratio with an integer number.

    The return type is the type returned by std::div called for the signed
    equivalent of T.
 */
template <is::stdRatio Ratio, is::intNumber T>
constexpr auto multiplyBy (T valueToScale)
{
    using R = ReducedToLowestForm<Ratio>;
    using IntType = std::make_signed_t<T>;
    using DivType = decltype (std::div (std::declval<IntType>(), std::declval<IntType>()));
    auto res = IntType (valueToScale) * IntType (R::num);

    if (std::is_constant_evaluated())
    {
        DivType d { 0, 0 };
        d.quot = res / IntType (R::den);
        d.rem = res % IntType (R::den);
        return d;
    }

    return std::div (res, IntType (R::den));
}

/** Multiplies a std::ratio with a floating point number and returns the quotient. */
template <is::stdRatio Ratio, is::realOrComplexFloatNumber T>
constexpr T multiplyBy (T valueToScale)
{
    using R = ReducedToLowestForm<Ratio>;
    valueToScale *= T (R::num);
    return valueToScale / T (R::den);
}

/** Multiplies a std::ratio with an integer number and returns the quotient.

    The function expects the remainder of that operation to be zero. Otherwise
    an assertion error will be hit.
 */
template <is::stdRatio Ratio, is::intNumber T>
constexpr auto expectNoRemainderMultiplyBy (T valueToScale)
{
    auto res = multiplyBy<Ratio> (valueToScale);
    VCTR_ASSERT (res.rem == 0);
    return res.quot;
}
}
