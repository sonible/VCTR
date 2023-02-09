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

#include <vctr_utils/compile_time_random_array.h>
#include <vctr/vctr.h>

/** A helper class to generate compile time generated arrays pre-populated with random values */
template <class T>
struct UnitTestValues
{
    // clang 15.0.2 crashes when this function is declared consteval instead of constexpr
    template <size_t size, int seedOffset, int min = -100, int max = 100>
    static constexpr auto array (bool removeZeros = false, bool forceZero = false)
    {
        constexpr auto r = vctr::Random::makeArrayWithUniformDistribution<size> (T (min), T (max), seedOffset);

        vctr::Array<T, size> a;

        for (size_t i = 0; i < size; ++i)
        {
            a[i] = r[i];

            if (removeZeros && a[i] == T (0))
                a[i] = T (1);
        }

        if (forceZero)
            a[size - 1] = T (0);

        return a;
    }

    template <size_t size, int seedOffset, int min = -100, int max = 100>
    static auto vector (bool removeZeros = false, bool forceZero = false)
    {
        return vctr::Vector (array<size, seedOffset, min, max> (removeZeros, forceZero));
    }
};

template <class T>
struct UnitTestValues<std::complex<T>>
{
    template <size_t size, int seedOffset, int min = -100, int max = 100>
    static constexpr auto array (bool removeZeros = false, bool forceZero = false)
    {
        auto realArray = vctr::Random::makeArrayWithUniformDistribution<2 * size> (T (min), T (max), seedOffset);

        vctr::Array<std::complex<T>, size> cplxArray;

        for (size_t i = 0; i < size; ++i)
        {
            cplxArray[i] = std::complex (realArray[i * 2], realArray[i * 2 + 1]);

            if (removeZeros && cplxArray[i] == std::complex (T (0), T (0)))
                cplxArray[i] += T (1);
        }

        if (forceZero)
            cplxArray[size - 1] = T (0);

        return cplxArray;
    }

    template <size_t size, int seedOffset, int min = -100, int max = 100>
    static auto vector (bool removeZeros = false, bool forceZero = false)
    {
        return vctr::Vector (array<size, seedOffset, min, max> (removeZeros, forceZero));
    }
};

/** Specialisation for std::string_view, the array holds up to 100 words from "Lorem ipsum" */
template <>
class UnitTestValues<std::string_view>
{
private:
    static constexpr std::array<std::string_view, 100> loremIpsum { "Lorem", "ipsum", "dolor", "sit", "amet", "consetetur", "sadipscing", "elitr", "sed", "diam", "nonumy", "eirmod", "tempor", "invidunt", "ut", "labore", "et", "dolore", "magna", "aliquyam", "erat", "sed", "diam", "voluptua", ".", "At", "vero", "eos", "et", "accusam", "et", "justo", "duo", "dolores", "et", "ea", "rebum", ".", "Stet", "clita", "kasd", "gubergren", "no", "sea", "takimata", "sanctus", "est", "Lorem", "ipsum", "dolor", "sit", "amet", ".", "Lorem", "ipsum", "dolor", "sit", "amet", "consetetur", "sadipscing", "elitr,", "sed", "diam", "nonumy", "ut", "labore", "et", "dolore", "magna", "aliquyam", "erat,", "sed", "diam", "voluptua", ".", "At", "vero", "eos", "et", "accusam", "et", "justo", "duo", "dolores", "et", "ea", "rebum.", "Stet", "clita", "kasd", "gubergren,", "no", "takimata", "sanctus", "est", "Lorem", "ipsum", "dolor", "sit", "amet." };

    template <size_t size, size_t... i>
    static constexpr vctr::Array<std::string_view, size> getSubArray (std::index_sequence<i...>)
    {
        return { loremIpsum[i]... };
    }

public:
    template <size_t size, int seedOffset>
    requires (size <= loremIpsum.size())
    static constexpr auto array()
    {
        // todo: Rotate by seedOffset once rotation is implemented
        return getSubArray<size> (std::make_index_sequence<size>());
    }

    template <size_t size, int seedOffset>
    static auto vector()
    {
        return vctr::Vector (array<size, seedOffset>());
    }
};

/** Specialisation for std::string. Same behaviour as std::string_view but not consteval */
template <>
class UnitTestValues<std::string>
{
private:
    template <size_t size, int seedOffset, size_t... i>
    static vctr::Array<std::string, size> fillArray (std::index_sequence<i...>)
    {
        constexpr auto loremIpsum = UnitTestValues<std::string_view>::array<size, seedOffset>();

        return { std::string (loremIpsum[i])... };
    }

public:
    template <size_t size, size_t seedOffset>
    static auto array()
    {
        return fillArray<size, seedOffset> (std::make_index_sequence<size>());
    }

    template <size_t size, size_t seedOffset>
    static auto vector()
    {
        return vctr::Vector (array<size, seedOffset>());
    }
};

template <class T>
struct PlatformVectorOps
{
    static constexpr auto filter = vctr::usePlatformVectorOps;
    using ElementType = T;
};

template <class T>
struct AVX
{
    static constexpr auto filter = vctr::useAVX;
    using ElementType = T;
};

template <class T>
struct SSE
{
    static constexpr auto filter = vctr::useSSE;
    using ElementType = T;
};

template <class T>
struct Neon
{
    static constexpr auto filter = vctr::useNeon;
    using ElementType = T;
};

#if VCTR_ARM
#define VCTR_NATIVE_SIMD Neon
#else
#define VCTR_NATIVE_SIMD AVX, SSE
#endif

#define VCTR_TEST_DEFINES_BASE(testVectorSize, start, end, avoidZeros, forceZero)                                             \
    using ElementType = typename TestType::ElementType;                                                                       \
    const auto& filter = TestType::filter;                                                                                    \
    constexpr auto srcA = UnitTestValues<ElementType>::template array<testVectorSize, 0, start, end> (avoidZeros, forceZero); \
    constexpr auto srcB = UnitTestValues<ElementType>::template array<testVectorSize, 1, start, end> (avoidZeros, forceZero); \
    const auto srcC = UnitTestValues<ElementType>::template vector<testVectorSize, 2, start, end> (avoidZeros, forceZero);    \
    const auto srcD = UnitTestValues<ElementType>::template vector<testVectorSize, 3, start, end> (avoidZeros, forceZero);    \
    const vctr::Span srcUnaligned = srcD.template subSpan<1>();

/** Defines a few handy things, needed in nearly every numerical unit test case:

    - ElementType: The element type of the test type
    - filter: The implementation filter instance of the test type
    - srcA, srcB: Two constexpr Arrays sized testVectorSize, prepopulated with random data
    - srcC, srcD: Two const Vectors sized testVectorSize, prepopulated with random data
    - srcUnaligned: A Span pointing to srcD with a start offset of 1. Therefore unaligned

    All values are in the range -100 to 100
 */
#define VCTR_TEST_DEFINES(testVectorSize) VCTR_TEST_DEFINES_BASE (testVectorSize, -100, 100, false, false)

/** Like VCTR_TEST_DEFINES, but it ensures that there are only nonzero values */
#define VCTR_TEST_DEFINES_NO_ZEROS(testVectorSize) VCTR_TEST_DEFINES_BASE (testVectorSize, -100, 100, true, false)

/** Like VCTR_TEST_DEFINES, but it ensures that the last value of each vector contains a zero */
#define VCTR_TEST_DEFINES_WITH_TRAILING_ZERO(testVectorSize) VCTR_TEST_DEFINES_BASE (testVectorSize, -100, 100, false, true)

/** Like VCTR_TEST_DEFINES, but it ensures that the values are all between start and end */
#define VCTR_TEST_DEFINES_IN_RANGE(start, end, testVectorSize) VCTR_TEST_DEFINES_BASE (testVectorSize, start, end, false, false)

/** Like VCTR_TEST_DEFINES_NO_ZEROS, but it ensures that the values are all between start and end */
#define VCTR_TEST_DEFINES_NO_ZEROS_IN_RANGE(start, end, testVectorSize) VCTR_TEST_DEFINES_BASE (testVectorSize, start, end, true, false)

/** Like VCTR_TEST_DEFINES_WITH_TRAILING_ZERO, but it ensures that the values are all between start and end */
#define VCTR_TEST_DEFINES_WITH_TRAILING_ZERO_IN_RANGE(start, end, testVectorSize) VCTR_TEST_DEFINES_BASE (testVectorSize, start, end, false, true)

#include <vctr_test_utils/vctr_catch_matchers.h>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_template_test_macros.hpp>
