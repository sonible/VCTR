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

#include <list>
#include <string>
#include <vctr_test_utils/vctr_test_common.h>

TEMPLATE_TEST_CASE ("Array Constructors", "[Array][Constructor]", float, int64_t, std::string)
{
    const auto v = UnitTestValues<TestType>::template vector<100, 0>();
    const auto a = UnitTestValues<TestType>::template array<100, 0>();

    // Default constructor
    vctr::Array<TestType, 42> uninitializedWithSize;
    REQUIRE (uninitializedWithSize.size() == 42);

    // Initial value constructor
    vctr::Array<TestType, 43> initializedWithSize (v[0]);
    REQUIRE (initializedWithSize.size() == 43);
    REQUIRE_THAT (initializedWithSize, vctr::AllElementsEqualTo (v[0]));

    // Initializer list
    vctr::Array il { v[0], v[1], v[2], v[3] };
    static_assert (std::same_as<TestType, typename decltype (il)::value_type>);
    REQUIRE (il.size() == 4);
    REQUIRE (il[0] == v[0]);
    REQUIRE (il[1] == v[1]);
    REQUIRE (il[2] == v[2]);
    REQUIRE (il[3] == v[3]);

    vctr::Array singleElementInitializerList { v[0] };
    static_assert (std::same_as<TestType, typename decltype (singleElementInitializerList)::value_type>);
    REQUIRE (singleElementInitializerList.size() == 1);
    REQUIRE (singleElementInitializerList[0] == v[0]);

    // Copy constructor
    vctr::Array aCopied (a);
    REQUIRE_THAT (a, vctr::Equals (aCopied));

    // Move constructor
    vctr::Array aMoved (std::move (aCopied));
    REQUIRE_THAT (a, vctr::Equals (aMoved));

    // Moving a std::array into an Array
    std::array sa { v[0], v[1], v[2] };
    vctr::Array saCopied (sa);
    REQUIRE_THAT (sa, vctr::Equals (saCopied));
    vctr::Array saMoved (std::move (sa));
    REQUIRE (saMoved.size() == 3);
    REQUIRE (il[0] == v[0]);
    REQUIRE (il[1] == v[1]);
    REQUIRE (il[2] == v[2]);

    // Copying from a different container type
    const std::list<TestType> l { v[0], v[1], v[2], v[3] };
    std::vector<TestType> sv { v[0], v[1], v[2], v[3] };
    vctr::Array<TestType, 4> lCopied (l);
    REQUIRE_THAT (l, vctr::Equals (lCopied));
    vctr::Array<TestType, 100> vCopied (v);
    REQUIRE_THAT (v, vctr::Equals (vCopied));
    vctr::Array<TestType, 4> svCopied (sv);
    REQUIRE_THAT (sv, vctr::Equals (svCopied));

    // Constructing a vector from a pointer and size pair
    vctr::Array<TestType, 100> fromPtr (v.data(), v.size());
    REQUIRE_THAT (a, vctr::Equals (fromPtr));

    // Constructing a vector from explicitly given iterators
    vctr::Array<TestType, 4> fromIterators (l.begin(), l.end());
    REQUIRE_THAT (l, vctr::Equals (fromIterators));
}

TEST_CASE ("Array Constructor with initializer function", "[Array][Constructor]")
{
    auto floatInitializerFn = [] (size_t i) { return std::sqrt (float (i)); };
    auto identityInitializerFn = [] (size_t i) { return i; };
    auto toStringInitializerFn = [] (size_t i) { return std::to_string (i); };

    const vctr::Array<float, 3> f (floatInitializerFn);
    REQUIRE (f[0] == 0.0f);
    REQUIRE (f[1] == 1.0f);
    REQUIRE (f[2] == std::sqrt (2.0f));

    const vctr::Array<size_t, 3> i (identityInitializerFn);
    REQUIRE (i[0] == 0);
    REQUIRE (i[1] == 1);
    REQUIRE (i[2] == 2);

    const vctr::Array<std::string, 3> s (toStringInitializerFn);
    REQUIRE_THAT (s[0], Catch::Matchers::Equals ("0"));
    REQUIRE_THAT (s[1], Catch::Matchers::Equals ("1"));
    REQUIRE_THAT (s[2], Catch::Matchers::Equals ("2"));
}

TEST_CASE ("OwnedArray Constructors", "[Array][OwnedArray][Constructor]")
{
    auto* i1 = new int32_t (1);
    auto* i2 = new int32_t (2);

    vctr::OwnedArray oa1 { i1, i2 };
    static_assert (std::same_as<std::unique_ptr<int32_t>, decltype (oa1)::value_type>);
    REQUIRE (oa1.size() == 2);
    REQUIRE (*oa1[0] == 1);
    REQUIRE (*oa1[1] == 2);

    vctr::OwnedVector oa2 { std::make_unique<int32_t> (3), std::make_unique<int32_t> (4) };
    static_assert (std::same_as<std::unique_ptr<int32_t>, decltype (oa2)::value_type>);
    REQUIRE (oa2.size() == 2);
    REQUIRE (*oa2[0] == 3);
    REQUIRE (*oa2[1] == 4);

    vctr::OwnedVector<char> oa3;
    static_assert (std::same_as<std::unique_ptr<char>, decltype (oa3)::value_type>);
    REQUIRE (oa3.size() == 0);
}
