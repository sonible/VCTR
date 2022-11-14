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
#include <list>
#include <string>

TEMPLATE_TEST_CASE ("Vector Constructors", "[Vector][Constructor]", float, int64_t, std::string)
{
    const auto v = UnitTestValues<TestType>::template vector<100, 0>();
    const auto a = UnitTestValues<TestType>::template array<100, 0>();

    // Default constructor
    vctr::Vector<TestType> empty;
    REQUIRE (empty.size() == 0);

    // Constructing a vector with a given size
    vctr::Vector<TestType> uninitialisedWithSize (42);
    REQUIRE (uninitialisedWithSize.size() == 42);

    vctr::Vector initialisedWithSize (43, v[0]);
    REQUIRE (initialisedWithSize.size() == 43);
    REQUIRE_THAT (initialisedWithSize, vctr::AllElementsEqualTo (v[0]));

    // Initializer list
    vctr::Vector il { v[0], v[1], v[2], v[3] };
    REQUIRE (il.size() == 4);
    REQUIRE (il[0] == v[0]);
    REQUIRE (il[1] == v[1]);
    REQUIRE (il[2] == v[2]);
    REQUIRE (il[3] == v[3]);

    // Copy constructor
    vctr::Vector vCopied (v);
    REQUIRE_THAT (v, vctr::Equals (vCopied));

    // Move constructor
    vctr::Vector vMoved (std::move (vCopied));
    REQUIRE_THAT (v, vctr::Equals (vMoved));

    // Copying from a different container type
    const std::list<TestType> l { v[0], v[1], v[2], v[3] };
    vctr::Vector lCopied (l);
    REQUIRE_THAT (l, vctr::Equals (lCopied));
    vctr::Vector aCopied (a);
    REQUIRE_THAT (a, vctr::Equals (aCopied));

    // Constructing a vector from a pointer and size pair
    vctr::Vector fromPtr (a.data(), a.size());
    REQUIRE_THAT (a, vctr::Equals (fromPtr));

    // Constructing a vector from explicitly given iterators
    vctr::Vector fromIterators (l.begin(), l.end());
    REQUIRE_THAT (l, vctr::Equals (fromIterators));
}

TEST_CASE ("Vector Constructor with single size_t argument", "[Vector][Constructor]")
{
    vctr::Vector singleElementInitializerList { size_t (2) };
    static_assert (std::same_as<size_t, decltype (singleElementInitializerList)::value_type>);
    REQUIRE (singleElementInitializerList.size() == 1);
    REQUIRE (singleElementInitializerList[0] == 2);
}

TEST_CASE ("Vector Constructor with initializer function", "[Vector][Constructor]")
{
    auto floatInitializerFn = [] (size_t i) { return std::sqrt (float (i)); };
    auto identityInitializerFn = [] (size_t i) { return i; };
    auto toStringInitializerFn = [] (size_t i) { return std::to_string (i); };

    const vctr::Vector f (3, floatInitializerFn);
    static_assert (std::same_as<float, decltype (f)::value_type>);
    REQUIRE (f[0] == 0.0f);
    REQUIRE (f[1] == 1.0f);
    REQUIRE (f[2] == std::sqrt (2.0f));

    const vctr::Vector i (3, identityInitializerFn);
    static_assert (std::same_as<size_t, decltype (i)::value_type>);
    REQUIRE (i[0] == 0);
    REQUIRE (i[1] == 1);
    REQUIRE (i[2] == 2);

    const vctr::Vector s (3, toStringInitializerFn);
    static_assert (std::same_as<std::string, decltype (s)::value_type>);
    REQUIRE_THAT (s[0], Catch::Matchers::Equals ("0"));
    REQUIRE_THAT (s[1], Catch::Matchers::Equals ("1"));
    REQUIRE_THAT (s[2], Catch::Matchers::Equals ("2"));
}

TEST_CASE ("OwnedVector Constructors", "[Vector][OwnedVector][Constructor]")
{
    auto* i1 = new int32_t (1);
    auto* i2 = new int32_t (2);

    vctr::OwnedVector ov1 { i1, i2 };
    static_assert (std::same_as<std::unique_ptr<int32_t>, decltype (ov1)::value_type>);
    REQUIRE (ov1.size() == 2);
    REQUIRE (*ov1[0] == 1);
    REQUIRE (*ov1[1] == 2);

    vctr::OwnedVector ov2 { std::make_unique<int32_t> (3), std::make_unique<int32_t> (4) };
    static_assert (std::same_as<std::unique_ptr<int32_t>, decltype (ov2)::value_type>);
    REQUIRE (ov2.size() == 2);
    REQUIRE (*ov2[0] == 3);
    REQUIRE (*ov2[1] == 4);

    vctr::OwnedVector<char> ov3;
    static_assert (std::same_as<std::unique_ptr<char>, decltype (ov3)::value_type>);
    REQUIRE (ov3.size() == 0);
}
