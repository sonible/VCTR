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

#include <vctr_test_utils/vctr_test_common.h>

TEMPLATE_PRODUCT_TEST_CASE ("Mean", "[mean]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>, int32_t, int64_t, uint32_t, uint64_t) )
{
    VCTR_TEST_DEFINES (10)

    const auto mean = vctr::mean << filter << srcA;
    const auto meanU = vctr::mean << filter << srcUnaligned;

    const auto meanMemFn = srcA.mean();
    const auto ref = std::reduce (srcA.begin(), srcA.end()) / ElementType (srcA.size());
    const auto refU = std::reduce (srcUnaligned.begin(), srcUnaligned.end()) / ElementType (srcUnaligned.size());

    const auto eps = vctr::RealType<ElementType> (0.00001);

    if constexpr (vctr::is::complexFloatNumber<ElementType>)
    {
        REQUIRE_THAT (mean.real(), Catch::Matchers::WithinRel (meanMemFn.real(), eps));
        REQUIRE_THAT (mean.imag(), Catch::Matchers::WithinRel (meanMemFn.imag(), eps));
        REQUIRE_THAT (mean.real(), Catch::Matchers::WithinRel (ref.real(), eps));
        REQUIRE_THAT (mean.imag(), Catch::Matchers::WithinRel (ref.imag(), eps));
        REQUIRE_THAT (meanU.real(), Catch::Matchers::WithinRel (refU.real(), eps));
        REQUIRE_THAT (meanU.imag(), Catch::Matchers::WithinRel (refU.imag(), eps));
    }
    else if constexpr (vctr::is::floatNumber<ElementType>)
    {
        REQUIRE_THAT (mean, Catch::Matchers::WithinRel (meanMemFn, eps));
        REQUIRE_THAT (mean, Catch::Matchers::WithinRel (ref, eps));
        REQUIRE_THAT (meanU, Catch::Matchers::WithinRel (refU, eps));
    }
    else
    {
        REQUIRE (mean == meanMemFn);
        REQUIRE (mean == ref);
        REQUIRE (meanU == refU);
    }
}

TEMPLATE_PRODUCT_TEST_CASE ("MeanSquare", "[meanSquare]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>, int32_t, int64_t, uint32_t, uint64_t) )
{
    VCTR_TEST_DEFINES (10)

    const auto meanSquare = vctr::meanSquare << filter << srcA;
    const auto meanSquareU = vctr::meanSquare << filter << srcUnaligned;

    vctr::Vector srcASquared = vctr::square << srcA;
    vctr::Vector srcUSquared = vctr::square << srcUnaligned;

    const auto meanSquareMemFn = srcA.meanSquare();
    const auto ref = std::reduce (srcASquared.begin(), srcASquared.end()) / ElementType (srcASquared.size());
    const auto refU = std::reduce (srcUSquared.begin(), srcUSquared.end()) / ElementType (srcUSquared.size());

    const auto eps = vctr::RealType<ElementType> (0.00001);

    if constexpr (vctr::is::complexFloatNumber<ElementType>)
    {
        REQUIRE_THAT (meanSquare.real(), Catch::Matchers::WithinRel (meanSquareMemFn.real(), eps));
        REQUIRE_THAT (meanSquare.imag(), Catch::Matchers::WithinRel (meanSquareMemFn.imag(), eps));
        REQUIRE_THAT (meanSquare.real(), Catch::Matchers::WithinRel (ref.real(), eps));
        REQUIRE_THAT (meanSquare.imag(), Catch::Matchers::WithinRel (ref.imag(), eps));
        REQUIRE_THAT (meanSquareU.real(), Catch::Matchers::WithinRel (refU.real(), eps));
        REQUIRE_THAT (meanSquareU.imag(), Catch::Matchers::WithinRel (refU.imag(), eps));
    }
    else if constexpr (vctr::is::floatNumber<ElementType>)
    {
        REQUIRE_THAT (meanSquare, Catch::Matchers::WithinRel (meanSquareMemFn, eps));
        REQUIRE_THAT (meanSquare, Catch::Matchers::WithinRel (ref, eps));
        REQUIRE_THAT (meanSquareU, Catch::Matchers::WithinRel (refU, eps));
    }
    else
    {
        REQUIRE (meanSquare == meanSquareMemFn);
        REQUIRE (meanSquare == ref);
        REQUIRE (meanSquareU == refU);
    }
}

TEMPLATE_PRODUCT_TEST_CASE ("RMS", "[rms]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>, int32_t, int64_t, uint32_t, uint64_t) )
{
    VCTR_TEST_DEFINES (10)

    const auto rms = vctr::rms << filter << srcA;
    const auto rmsU = vctr::rms << filter << srcUnaligned;

    vctr::Vector srcASquared = vctr::square << srcA;
    vctr::Vector srcUSquared = vctr::square << srcUnaligned;

    const auto rmsMemFn = srcA.rms();
    const auto ref = ElementType (std::sqrt (std::reduce (srcASquared.begin(), srcASquared.end()) / ElementType (srcASquared.size())));
    const auto refU = ElementType (std::sqrt (std::reduce (srcUSquared.begin(), srcUSquared.end()) / ElementType (srcUSquared.size())));

    const auto eps = vctr::RealType<ElementType> (0.00001);

    if constexpr (vctr::is::complexFloatNumber<ElementType>)
    {
        REQUIRE_THAT (rms.real(), Catch::Matchers::WithinRel (rmsMemFn.real(), eps));
        REQUIRE_THAT (rms.imag(), Catch::Matchers::WithinRel (rmsMemFn.imag(), eps));
        REQUIRE_THAT (rms.real(), Catch::Matchers::WithinRel (ref.real(), eps));
        REQUIRE_THAT (rms.imag(), Catch::Matchers::WithinRel (ref.imag(), eps));
        REQUIRE_THAT (rmsU.real(), Catch::Matchers::WithinRel (refU.real(), eps));
        REQUIRE_THAT (rmsU.imag(), Catch::Matchers::WithinRel (refU.imag(), eps));
    }
    else if constexpr (vctr::is::floatNumber<ElementType>)
    {
        REQUIRE_THAT (rms, Catch::Matchers::WithinRel (rmsMemFn, eps));
        REQUIRE_THAT (rms, Catch::Matchers::WithinRel (ref, eps));
        REQUIRE_THAT (rmsU, Catch::Matchers::WithinRel (refU, eps));
    }
    else
    {
        REQUIRE (rms == rmsMemFn);
        REQUIRE (rms == ref);
        REQUIRE (rmsU == refU);
    }
}