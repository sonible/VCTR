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

template <vctr::is::realNumber T>
int16_t toInt16Plus1 (T in) { return static_cast<int16_t> (in + T (1)); }

template <vctr::is::realNumber T>
double toDoublePlus1 (T in) { return static_cast<double> (in + T (1)); }

template <vctr::is::realNumber T>
std::string toStringPlus1 (T in) { return std::to_string (in + T (1)); }

TEST_CASE ("TransformedByDynamicCast", "[VCTR][Expressions][transformation]")
{
    struct A
    {
        virtual ~A() = default;
        int a;
    };

    struct B
    {
        virtual ~B() = default;
        int b;
    };

    struct C : A, B
    {
        C (int x, int y)
        {
            a = x;
            b = y;
        }

        ~C() override = default;
    };

    vctr::Array<B*, 2> bs { new C (1, 2), new C (3, 4) };

    vctr::Array aPtrs = vctr::transformedByDynamicCastTo<A*> << bs;
    vctr::Array aVals = vctr::transformedByStaticCastTo<A> << vctr::transformedByDynamicCastTo<A&> << vctr::transformedBy ([] (B* b)-> B& { return *b; }) << bs;

    // The cast should have succeeded for all
    REQUIRE (aPtrs.all ([] (A* a) { return a != nullptr; }));

    REQUIRE (aPtrs[0]->a == 1);
    REQUIRE (aPtrs[1]->a == 3);
    REQUIRE (aVals[0].a == 1);
    REQUIRE (aVals[1].a == 3);

    // This cast should fail. Suppress assertion outputs for that case
    vctr::Array invalid = vctr::transformedByDynamicCastTo<std::string*, false> << bs;
    REQUIRE (invalid.allElementsEqual (nullptr));

    bs.forEach ([] (B* b) { delete b; });
}

TEMPLATE_PRODUCT_TEST_CASE ("TransformedByStaticCast", "[VCTR][Expressions][transformation]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int16_t, int32_t, int64_t))
{
    VCTR_TEST_DEFINES (10)

    // Testing the cast on an expression to verify that the loop based vector op implementation works
    vctr::Vector castedToInt16 = filter << vctr::transformedByStaticCastTo<int16_t> << (srcA + ElementType (1));
    vctr::Vector castedToDouble = filter << vctr::transformedByStaticCastTo<double> << (srcA + ElementType (1));

    REQUIRE_THAT (castedToInt16, vctr::EqualsTransformedBy<toInt16Plus1> (srcA));
    REQUIRE_THAT (castedToDouble, vctr::EqualsTransformedBy<toDoublePlus1> (srcA));
}

TEMPLATE_PRODUCT_TEST_CASE ("TransformedBy", "[VCTR][Expressions][transformation]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, int64_t))
{
    VCTR_TEST_DEFINES (10)

    int counter = 0;

    // There are different code paths for copyable and non-copyable function objects.
    // These two variations cover both cases.
    vctr::Vector convertedToString          = filter << vctr::transformedBy ([] (auto n) { return std::to_string (n); }) << (srcA + ElementType (1));
    vctr::Vector convertedToStringWithCount = filter << vctr::transformedBy ([&] (auto n) { ++counter; return std::to_string (n); }) << (srcA + ElementType (1));

    REQUIRE_THAT (convertedToString, vctr::EqualsTransformedBy<toStringPlus1> (srcA));
    REQUIRE_THAT (convertedToStringWithCount, vctr::EqualsTransformedBy<toStringPlus1> (srcA));

    REQUIRE (counter == 10);
}