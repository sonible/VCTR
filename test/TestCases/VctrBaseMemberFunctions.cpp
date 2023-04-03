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

TEST_CASE ("subSpan", "[VctrBaseMemberFunctions]")
{
    auto v = UnitTestValues<int32_t>::template vector<100, 0>();
    auto a = UnitTestValues<int32_t>::template array<100, 1>();
    const auto cv = UnitTestValues<int32_t>::template vector<100, 2>();
    const auto ca = UnitTestValues<int32_t>::template array<100, 3>();

    //==============================================================================
    // Span of a non-const Vector with compile time constant startIdx
    auto vStaticOffset1 = v.template subSpan<1>();
    static_assert (std::same_as<decltype (vStaticOffset1)::element_type, int32_t>);
    static_assert (vStaticOffset1.getExtent() == std::dynamic_extent);
    REQUIRE (vStaticOffset1.size() == 99);
    REQUIRE (vStaticOffset1[0] == v[1]);

    // Span of a non-const Vector with compile time constant startIdx and length
    auto vStaticOffset2 = v.template subSpan<1, 80>();
    static_assert (std::same_as<decltype (vStaticOffset2)::element_type, int32_t>);
    static_assert (vStaticOffset2.getExtent() == 80);
    REQUIRE (vStaticOffset2.size() == 80);
    REQUIRE (vStaticOffset2[0] == v[1]);

    // Span of a non-const Array with compile time constant startIdx
    auto aStaticOffset1 = a.template subSpan<1>();
    static_assert (std::same_as<decltype (aStaticOffset1)::element_type, int32_t>);
    static_assert (aStaticOffset1.getExtent() == 99);
    REQUIRE (aStaticOffset1.size() == 99);
    REQUIRE (aStaticOffset1[0] == a[1]);

    // Span of a non-const Array with compile time constant startIdx and length
    auto aStaticOffset2 = a.template subSpan<1, 80>();
    static_assert (std::same_as<decltype (aStaticOffset2)::element_type, int32_t>);
    static_assert (aStaticOffset2.getExtent() == 80);
    REQUIRE (aStaticOffset2.size() == 80);
    REQUIRE (aStaticOffset2[0] == a[1]);

    // Span of a const Vector with compile time constant startIdx
    auto cvStaticOffset1 = cv.template subSpan<1>();
    static_assert (std::same_as<decltype (cvStaticOffset1)::element_type, const int32_t>);
    static_assert (cvStaticOffset1.getExtent() == std::dynamic_extent);
    REQUIRE (cvStaticOffset1.size() == 99);
    REQUIRE (cvStaticOffset1[0] == cv[1]);

    // Span of a const Vector with compile time constant startIdx
    auto cvStaticOffset2 = cv.template subSpan<1, 80>();
    static_assert (std::same_as<decltype (cvStaticOffset2)::element_type, const int32_t>);
    static_assert (cvStaticOffset2.getExtent() == 80);
    REQUIRE (cvStaticOffset2.size() == 80);
    REQUIRE (cvStaticOffset2[0] == cv[1]);

    // Span of a const Array with compile time constant startIdx
    auto caStaticOffset1 = ca.template subSpan<1>();
    static_assert (std::same_as<decltype (caStaticOffset1)::element_type, const int32_t>);
    static_assert (caStaticOffset1.getExtent() == 99);
    REQUIRE (caStaticOffset1.size() == 99);
    REQUIRE (caStaticOffset1[0] == ca[1]);

    // Span of a const Array with compile time constant startIdx
    auto caStaticOffset2 = ca.template subSpan<1, 80>();
    static_assert (std::same_as<decltype (caStaticOffset2)::element_type, const int32_t>);
    static_assert (caStaticOffset2.getExtent() == 80);
    REQUIRE (caStaticOffset2.size() == 80);
    REQUIRE (caStaticOffset2[0] == ca[1]);

    //==============================================================================
    // Span of a non-const Vector with compile time constant startIdx
    auto vDynamicOffset1 = v.subSpan (1);
    static_assert (std::same_as<decltype (vDynamicOffset1)::element_type, int32_t>);
    static_assert (vDynamicOffset1.getExtent() == std::dynamic_extent);
    REQUIRE (vDynamicOffset1.size() == 99);
    REQUIRE (vDynamicOffset1[0] == v[1]);

    // Span of a non-const Vector with compile time constant startIdx and length
    auto vDynamicOffset2 = v.subSpan (1, 80);
    static_assert (std::same_as<decltype (vDynamicOffset2)::element_type, int32_t>);
    static_assert (vDynamicOffset2.getExtent() == std::dynamic_extent);
    REQUIRE (vDynamicOffset2.size() == 80);
    REQUIRE (vDynamicOffset2[0] == v[1]);

    // Span of a non-const Array with compile time constant startIdx
    auto aDynamicOffset1 = a.subSpan (1);
    static_assert (std::same_as<decltype (aDynamicOffset1)::element_type, int32_t>);
    static_assert (aDynamicOffset1.getExtent() == std::dynamic_extent);
    REQUIRE (aDynamicOffset1.size() == 99);
    REQUIRE (aDynamicOffset1[0] == a[1]);

    // Span of a non-const Array with compile time constant startIdx and length
    auto aDynamicOffset2 = a.subSpan (1, 80);
    static_assert (std::same_as<decltype (aDynamicOffset2)::element_type, int32_t>);
    static_assert (aDynamicOffset2.getExtent() == std::dynamic_extent);
    REQUIRE (aDynamicOffset2.size() == 80);
    REQUIRE (aDynamicOffset2[0] == a[1]);

    // Span of a const Vector with compile time constant startIdx
    auto cvDynamicOffset1 = cv.subSpan (1);
    static_assert (std::same_as<decltype (cvDynamicOffset1)::element_type, const int32_t>);
    static_assert (cvDynamicOffset1.getExtent() == std::dynamic_extent);
    REQUIRE (cvDynamicOffset1.size() == 99);
    REQUIRE (cvDynamicOffset1[0] == cv[1]);

    // Span of a const Vector with compile time constant startIdx
    auto cvDynamicOffset2 = cv.subSpan (1, 80);
    static_assert (std::same_as<decltype (cvDynamicOffset2)::element_type, const int32_t>);
    static_assert (cvDynamicOffset2.getExtent() == std::dynamic_extent);
    REQUIRE (cvDynamicOffset2.size() == 80);
    REQUIRE (cvDynamicOffset2[0] == cv[1]);

    // Span of a const Array with compile time constant startIdx
    auto caDynamicOffset1 = ca.subSpan (1);
    static_assert (std::same_as<decltype (caDynamicOffset1)::element_type, const int32_t>);
    static_assert (caDynamicOffset1.getExtent() == std::dynamic_extent);
    REQUIRE (caDynamicOffset1.size() == 99);
    REQUIRE (caDynamicOffset1[0] == ca[1]);

    // Span of a const Array with compile time constant startIdx
    auto caDynamicOffset2 = ca.subSpan (1, 80);
    static_assert (std::same_as<decltype (caDynamicOffset2)::element_type, const int32_t>);
    static_assert (caDynamicOffset2.getExtent() == std::dynamic_extent);
    REQUIRE (caDynamicOffset2.size() == 80);
    REQUIRE (caDynamicOffset2[0] == ca[1]);

    //==============================================================================
    // Spans of a spans
    const auto& s1 = vStaticOffset1;
    auto spanStaticOffset1 = s1.template subSpan<1>();
    static_assert (std::same_as<decltype (spanStaticOffset1)::element_type, int32_t>);
    static_assert (spanStaticOffset1.getExtent() == std::dynamic_extent);
    REQUIRE (spanStaticOffset1.size() == 98);
    REQUIRE (spanStaticOffset1[0] == vStaticOffset1[1]);

    const auto& s2 = vStaticOffset2;
    auto spanStaticOffset2 = s2.template subSpan<1>();
    static_assert (std::same_as<decltype (spanStaticOffset2)::element_type, int32_t>);
    static_assert (spanStaticOffset2.getExtent() == 79);
    REQUIRE (spanStaticOffset2.size() == 79);
    REQUIRE (spanStaticOffset2[0] == vStaticOffset2[1]);

    const auto& s3 = cvStaticOffset1;
    auto spanStaticOffset3 = s3.template subSpan<1>();
    static_assert (std::same_as<decltype (spanStaticOffset3)::element_type, const int32_t>);
    static_assert (spanStaticOffset3.getExtent() == std::dynamic_extent);
    REQUIRE (spanStaticOffset3.size() == 98);
    REQUIRE (spanStaticOffset3[0] == cvStaticOffset1[1]);

    //==============================================================================
    // Span on constexpr Array
    static constexpr vctr::Array<int, 4> cea { 1, 2, 3, 4 };
    static constexpr auto ces = cea.subSpan<0>();
    static_assert (cea.size() == ces.size());
    static_assert (cea[0] == ces[0]);
    static_assert (cea[1] == ces[1]);
    static_assert (cea[2] == ces[2]);
    static_assert (cea[3] == ces[3]);

    static constexpr auto ces2 = cea.subSpan<1, 2>();
    static_assert (ces2.size() == 2);
    static_assert (cea[1] == ces2[0]);
    static_assert (cea[2] == ces2[1]);
}

TEST_CASE ("forEach", "[VctrBaseMemberFunctions]")
{
    const auto loremIpsum = UnitTestValues<std::string>::template array<4, 1>();

    auto strings = loremIpsum;

    // In place for each function
    strings.forEach ([] (std::string& s) { for (auto& c : s) c = std::toupper (c); });
    REQUIRE (strings[0] == "LOREM");
    REQUIRE (strings[1] == "IPSUM");
    REQUIRE (strings[2] == "DOLOR");
    REQUIRE (strings[3] == "SIT");

    // Returning for each function
    strings.forEach ([] (const std::string& s) { return s + "."; });
    REQUIRE (strings[0] == "LOREM.");
    REQUIRE (strings[1] == "IPSUM.");
    REQUIRE (strings[2] == "DOLOR.");
    REQUIRE (strings[3] == "SIT.");

    // In place for each function with index
    strings.forEach ([] (std::string& s, size_t idx) { s = std::to_string (idx); });
    REQUIRE (strings[0] == "0");
    REQUIRE (strings[1] == "1");
    REQUIRE (strings[2] == "2");
    REQUIRE (strings[3] == "3");

    // Returning for each function with index
    strings.forEach ([] (const std::string& s, size_t idx) { return s + std::string (idx, '.'); });
    REQUIRE (strings[0] == "0");
    REQUIRE (strings[1] == "1.");
    REQUIRE (strings[2] == "2..");
    REQUIRE (strings[3] == "3...");

    // In place for each function with arguments
    strings.forEach ([] (std::string& s, size_t pos, size_t n) { s = s.substr (pos, n); }, 1, 2);
    REQUIRE (strings[0].empty());
    REQUIRE (strings[1] == ".");
    REQUIRE (strings[2] == "..");
    REQUIRE (strings[3] == "..");

    // Returning for each function with argument
    strings.forEach (std::plus(), "x");
    REQUIRE (strings[0] == "x");
    REQUIRE (strings[1] == ".x");
    REQUIRE (strings[2] == "..x");
    REQUIRE (strings[3] == "..x");

    const auto cv = UnitTestValues<double>::template vector<100, 2>();

    // Non mutating for each function
    double sum1 = 0.0;
    cv.forEach ([&] (double e) { sum1 += e; });

    double sum2 = 0.0;
    for (auto& e : cv)
        sum2 += e;

    REQUIRE (sum1 == sum2);

    sum1 = 0.0;
    sum2 = 0.0;

    // Non mutating for each function with idx
    cv.forEach ([&] (double e, size_t idx) { sum1 += e + double (idx); });
    for (size_t i = 0; i < cv.size(); ++i)
        sum2 += cv[i] + double (i);

    REQUIRE (sum1 == sum2);

    sum1 = 0.0;
    sum2 = 0.0;

    // Non mutating for each function with arg
    cv.forEach ([&] (double e, double x) { sum1 += e + x; }, -1.0);
    for (auto& e : cv)
        sum2 += e - 1.0;

    REQUIRE (sum1 == sum2);
}

TEST_CASE ("empty", "[VctrBaseMemberFunctions]")
{
    auto a = UnitTestValues<int32_t>::template array<10, 2>();
    auto ae = vctr::Array<int32_t, 0>();
    vctr::Span spanFromA (a);
    vctr::Span spanFromAe (ae);

    REQUIRE_FALSE (a.empty());
    REQUIRE_FALSE (spanFromA.empty());
    REQUIRE (ae.empty());
    REQUIRE (spanFromAe.empty());

    auto v = UnitTestValues<int32_t>::template vector<10, 1>();

    REQUIRE_FALSE (v.empty());
    REQUIRE_FALSE (vctr::Span (v).empty());

    v.clear();

    REQUIRE (v.empty());
    REQUIRE (vctr::Span (v).empty());
}

TEST_CASE ("sizeInBytes", "[VctrBaseMemberFunctions]")
{
    auto v = UnitTestValues<int32_t>::template vector<100, 1>();
    auto a = UnitTestValues<int32_t>::template array<100, 2>();
    vctr::Span spanFromV (v);
    vctr::Span spanFromA (a);

    constexpr auto sizeOfA = a.sizeInBytes();
    constexpr auto sizeOfSpanFromA = spanFromA.sizeInBytes();

    REQUIRE (v.sizeInBytes() == 400);
    REQUIRE (spanFromV.sizeInBytes() == 400);
    REQUIRE (sizeOfA == 400);
    REQUIRE (sizeOfSpanFromA == std::span (a).size_bytes());
}

TEST_CASE ("reverse, rotate", "[VctrBaseMemberFunctions]")
{
    auto loremIpsum = UnitTestValues<std::string>::template array<5, 1>();

    loremIpsum.reverse();
    REQUIRE (loremIpsum[0] == "amet");
    REQUIRE (loremIpsum[1] == "sit");
    REQUIRE (loremIpsum[2] == "dolor");
    REQUIRE (loremIpsum[3] == "ipsum");
    REQUIRE (loremIpsum[4] == "Lorem");

    loremIpsum.rotate (2);
    REQUIRE (loremIpsum[0] == "dolor");
    REQUIRE (loremIpsum[1] == "ipsum");
    REQUIRE (loremIpsum[2] == "Lorem");
    REQUIRE (loremIpsum[3] == "amet");
    REQUIRE (loremIpsum[4] == "sit");
}

TEST_CASE ("shift", "[VctrBaseMemberFunctions]")
{
    vctr::Array a { 0, 1, 2, 3, 4 };

    SECTION ("shiftRight, don't clear")
    {
        a.shiftRight (2, false);
        vctr::Array shifted { 0, 1, 0, 1, 2 };
        REQUIRE_THAT (shifted, vctr::Equals (a));
    }

    SECTION ("shiftRight, clear")
    {
        a.shiftRight (2, true);
        vctr::Array shifted { 0, 0, 0, 1, 2 };
        REQUIRE_THAT (shifted, vctr::Equals (a));
    }

    SECTION ("shiftLeft, don't clear")
    {
        a.shiftLeft (3, false);
        vctr::Array shifted { 3, 4, 2, 3, 4 };
        REQUIRE_THAT (shifted, vctr::Equals (a));
    }

    SECTION ("shiftLeft, clear")
    {
        a.shiftLeft (3, true);
        vctr::Array shifted { 3, 4, 0, 0, 0 };
        REQUIRE_THAT (shifted, vctr::Equals (a));
    }

    SECTION ("shift by entire size, don't clear")
    {
        auto aCopy = a;
        a.shiftRight (a.size(), false);
        REQUIRE_THAT (aCopy, vctr::Equals (a));
    }

    SECTION ("shift by entire size, clear")
    {
        a.shiftRight (a.size(), true);
        vctr::Array zeros { 0, 0, 0, 0, 0 };
        REQUIRE_THAT (zeros, vctr::Equals (a));
    }
}

TEST_CASE ("assign", "[VctrBaseMemberFunctions]")
{
    vctr::Vector<int> v (3, 42);
    REQUIRE (v.size() == 3);

    // Assigning values resizes a Vector and copies the values to the container
    v.assign ({ -2, -1, 0, 1, 2 });
    REQUIRE (v.size() == 5);
    REQUIRE_THAT (v, vctr::Equals ({ -2, -1, 0, 1, 2 }));

    // Assign an initializer list to a span viewing the vector should alter the viewed Vector
    vctr::Span s = v;
    s.assign ({ 100, 101, 102, 103, 104 });
    REQUIRE_THAT (v, vctr::Equals ({ 100, 101, 102, 103, 104 }));
}

TEST_CASE ("fill", "[VctrBaseMemberFunctions]")
{
    constexpr int fillValue = 5;

    vctr::Array a { 0, 1, 2, 3, 4 };
    a.fill (fillValue);
    REQUIRE (a.allElementsEqual (fillValue));

    vctr::Vector b { 0, 1, 2, 3, 4 };
    b.fill (fillValue);
    REQUIRE (b.allElementsEqual (fillValue));
}

TEST_CASE ("init", "[VctrBaseMemberFunctions]")
{
    constexpr int fillValue = 13;

    SECTION ("Reduce size")
    {
        vctr::Vector a { 0, 1, 2, 3, 4 };
        a.init (3, fillValue);
        REQUIRE (a.size() == 3);
        REQUIRE (a.allElementsEqual (fillValue));
    }

    SECTION ("Increase size")
    {
        vctr::Vector a { 0, 1, 2, 3, 4 };
        a.init (30, fillValue);
        REQUIRE (a.size() == 30);
        REQUIRE (a.allElementsEqual (fillValue));
    }
}

TEMPLATE_TEST_CASE ("allElementsAreFinite", "[VctrBaseMemberFunctions]", float, double)
{
    vctr::Vector<TestType> real { 1.0f, -1.0f };
    REQUIRE (real.allElementsAreFinite());

    real.push_back (std::numeric_limits<TestType>::infinity());
    REQUIRE_FALSE (real.allElementsAreFinite());

    vctr::Vector<std::complex<TestType>> cplx { { 0.001f, 1e20f }, { -0.001f, -1e20f } };
    REQUIRE (cplx.allElementsAreFinite());

    cplx.push_back ({ - std::numeric_limits<TestType>::infinity(), std::numeric_limits<TestType>::infinity()});
    REQUIRE_FALSE (cplx.allElementsAreFinite());
}

TEMPLATE_TEST_CASE ("anyElementIsNaN", "[VctrBaseMemberFunctions]", float, double)
{
    vctr::Vector<TestType> real { 1.0f, -1.0f };
    REQUIRE_FALSE (real.anyElementIsNaN());

    real.push_back (std::numeric_limits<TestType>::quiet_NaN());
    REQUIRE (real.anyElementIsNaN());

    vctr::Vector<std::complex<TestType>> cplx { { 0.001f, 1e20f }, { -0.001f, -1e20f } };
    REQUIRE_FALSE (cplx.anyElementIsNaN());

    cplx.push_back ({ 0.0f, std::numeric_limits<TestType>::quiet_NaN()});
    REQUIRE (cplx.anyElementIsNaN());
}