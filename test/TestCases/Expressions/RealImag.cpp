/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2023 by sonible GmbH.

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

#include <vctr_test_utils/vctr_test_common.h>

template <std::floating_point T>
T getRealPart (std::complex<T> src) { return src.real(); }

template <std::floating_point T>
T getImagPart (std::complex<T> src) { return src.imag(); }

TEMPLATE_PRODUCT_TEST_CASE ("Real", "[VCTR][Expressions][Complex][real]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES (10)

    const vctr::Vector real = filter << vctr::real << srcA;
    const vctr::Vector realU = filter << vctr::real << srcUnaligned;

    // std::is_arithmetic is not true for complex values, so this checks if the return type is always real valued, even for
    // complex input
    static_assert (std::is_arithmetic_v<typename decltype (real)::value_type>);
    static_assert (std::is_arithmetic_v<typename decltype (realU)::value_type>);

    REQUIRE_THAT (real, vctr::EqualsTransformedBy<getRealPart> (srcA));
    REQUIRE_THAT (realU, vctr::EqualsTransformedBy<getRealPart> (srcUnaligned));
}

TEMPLATE_PRODUCT_TEST_CASE ("Imag", "[VCTR][Expressions][Complex][imag]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES (10)

    const vctr::Vector imag = filter << vctr::imag << srcA;
    const vctr::Vector imagU = filter << vctr::imag << srcUnaligned;

    // std::is_arithmetic is not true for complex values, so this checks if the return type is always real valued, even for
    // complex input
    static_assert (std::is_arithmetic_v<typename decltype (imag)::value_type>);
    static_assert (std::is_arithmetic_v<typename decltype (imagU)::value_type>);

    REQUIRE_THAT (imag, vctr::EqualsTransformedBy<getImagPart> (srcA));
    REQUIRE_THAT (imagU, vctr::EqualsTransformedBy<getImagPart> (srcUnaligned));
}

TEMPLATE_PRODUCT_TEST_CASE ("RealToComplex", "[VCTR][Expressions][Complex][realToComplex]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double) )
{
    VCTR_TEST_DEFINES (10)

    const vctr::Vector cplx = filter << vctr::realToComplex << srcA;
    const vctr::Vector cplxU = filter << vctr::realToComplex << srcUnaligned;
    
    static_assert (vctr::is::complexFloatNumber<typename decltype (cplx)::value_type>);
    static_assert (vctr::is::complexFloatNumber<typename decltype (cplxU)::value_type>);

    REQUIRE_THAT (srcA, vctr::EqualsTransformedBy<getRealPart> (cplx));
    REQUIRE_THAT (srcUnaligned, vctr::EqualsTransformedBy<getRealPart> (cplxU));

    REQUIRE (vctr::Vector (vctr::imag << cplx).allElementsEqual (ElementType (0)));
    REQUIRE (vctr::Vector (vctr::imag << cplxU).allElementsEqual (ElementType (0)));
}