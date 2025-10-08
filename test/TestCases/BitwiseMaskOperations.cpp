/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2025 by sonible GmbH.

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
#include <map>
#include <numbers>

template <class>
struct CompareOpHelper
{};

template <class T>
struct CompareOpHelper<std::less<T>>
{
    static constexpr auto compareOp = vctr::CompareOp::less;
    using ValueType = T;
};

template <class T>
struct CompareOpHelper<std::less_equal<T>>
{
    static constexpr auto compareOp = vctr::CompareOp::lessOrEqual;
    using ValueType = T;
};

template <class T>
struct CompareOpHelper<std::greater<T>>
{
    static constexpr auto compareOp = vctr::CompareOp::greater;
    using ValueType = T;
};

template <class T>
struct CompareOpHelper<std::greater_equal<T>>
{
    static constexpr auto compareOp = vctr::CompareOp::greaterOrEqual;
    using ValueType = T;
};

template <class T>
struct CompareOpHelper<std::equal_to<T>>
{
    static constexpr auto compareOp = vctr::CompareOp::equal;
    using ValueType = T;
};

template <class T>
struct CompareOpHelper<std::not_equal_to<T>>
{
    static constexpr auto compareOp = vctr::CompareOp::notEqual;
    using ValueType = T;
};

#if VCTR_ARM
template <class T>
using RegisterType = vctr::NeonRegister<T>;
#else
template <class T>
using RegisterType = vctr::AVXRegister<T>;
#if VCTR_GCC
// In contrast to other test cases where we test higher level VCTR functions, we call register member functions directly here.
// All the AVX functions are attributed with __attribute__ ((target ("avx"))). Directly taking the return value of those functions
// here leads to issues with GCC obviously not using the correct calling conventions when compiling the test case without explicitly
// setting the AVX target. This pragma enables it for the entire translation unit
#pragma GCC target ("avx")
#endif
#endif

TEMPLATE_PRODUCT_TEST_CASE ("BitwiseMaskOperations", "[VCTR][RegisterTypes][BitwiseMaskOperations]", (std::less, std::less_equal, std::greater, std::greater_equal, std::equal_to, std::not_equal_to), (float, double) )
{
    constexpr auto registerCompareOp = CompareOpHelper<TestType>::compareOp;
    constexpr auto scalarCompareOp = TestType();
    using T = typename CompareOpHelper<TestType>::ValueType;
    constexpr auto numElements = RegisterType<T>::numElements;

    constexpr size_t vecSize = 16;
    // compare max (0, sin (x)) against max (0, cos (x)) as these test all given operators
    constexpr auto zeroToTwoPi = vctr::generators::Linspace::makeArray<T, vecSize> (T (0), 2 * std::numbers::pi_v<T>, false);
    constexpr vctr::Array vecA = vctr::clampLowByConstant<0> << vctr::sin << zeroToTwoPi;
    constexpr vctr::Array vecB = vctr::clampLowByConstant<0> << vctr::cos << zeroToTwoPi;

    for (size_t registerStart = 0; registerStart < vecA.size(); registerStart += numElements)
    {
        auto aSrc = vecA.subSpan (registerStart, numElements);
        auto bSrc = vecB.subSpan (registerStart, numElements);

#if VCTR_ARM
        const auto a = aSrc.getNeon (0);
        const auto b = bSrc.getNeon (0);
#else
        const auto a = aSrc.getAVX (0);
        const auto b = bSrc.getAVX (0);
#endif
        const auto mask = RegisterType<T>::template compare<registerCompareOp> (a, b);

        // perform other bitwise operations
        const auto bitAnd = RegisterType<T>::bitwiseAnd (a, mask);
        const auto bitAndNot = RegisterType<T>::bitwiseAndNot (mask, a); // careful: First argument is negated, not the second one!
        const auto bitBlend = RegisterType<T>::bitwiseBlend (a, b, mask);

        // restore registers to vectors
        vctr::Array<T, numElements> outMask;
        vctr::Array<T, numElements> outBitAnd;
        vctr::Array<T, numElements> outBitAndNot;
        vctr::Array<T, numElements> outBitBlend;

#if VCTR_ARM
        mask.store (outMask.data());
        bitAnd.store (outBitAnd.data());
        bitAndNot.store (outBitAndNot.data());
        bitBlend.store (outBitBlend.data());
#else
        mask.storeAligned (outMask.data());
        bitAnd.storeAligned (outBitAnd.data());
        bitAndNot.storeAligned (outBitAndNot.data());
        bitBlend.storeAligned (outBitBlend.data());
#endif

        // run test for restored data
        for (size_t i = 0; i < numElements; ++i)
        {
            // scalarCompareOp is an instance of the comparison function object (e.g. std::less, std::greater, etc)
            // It evaluates the same comparison on individual elements as evaluated by RegisterType<T>::compare
            // above in order to check if the register wise operations generated reasonable per-element results
            if (scalarCompareOp (aSrc[i], bSrc[i]))
            {
                REQUIRE (std::isnan (outMask[i])); // 0xFFFFFFFF is -nan
                REQUIRE (outBitAnd[i] == aSrc[i]);
                REQUIRE (outBitAndNot[i] == 0.0f);
                REQUIRE (outBitBlend[i] == bSrc[i]);
            }
            else
            {
                VCTR_ASSERT (outMask[i] == 0.0f);
                REQUIRE (outBitAnd[i] == 0.0f);
                REQUIRE (outBitAndNot[i] == aSrc[i]);
                REQUIRE (outBitBlend[i] == aSrc[i]);
            }
        }
    }
}