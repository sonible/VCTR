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
#include <string>

TEMPLATE_TEST_CASE ("Span Constructors", "[Span][Constructor]", float, int64_t, std::string)
{
    auto v = UnitTestValues<TestType>::template vector<100, 0>();
    const auto cv = UnitTestValues<TestType>::template vector<100, 0>();

    // Default constructor
    vctr::Span<TestType> uninitialized;
    REQUIRE (uninitialized.data() == nullptr);
    REQUIRE (uninitialized.size() == 0);
    REQUIRE (uninitialized.getExtent() == std::dynamic_extent);

    // Manually created from pointer and size
    vctr::Span fromPtrAndSize (v.data(), v.size());
    REQUIRE (fromPtrAndSize.data() == v.data());
    REQUIRE (fromPtrAndSize.size() == v.size());
    REQUIRE_THAT (fromPtrAndSize, vctr::Equals (v));
    if constexpr (vctr::is::number<TestType>) // if v is numeric, we know it's aligned
        REQUIRE (fromPtrAndSize.getStorageInfo().dataIsSIMDAligned == true);

    vctr::Span fromUnalignedPtrAndSize (v.data() + 1, v.size() - 1);
    static_assert (std::same_as<TestType, typename decltype (fromUnalignedPtrAndSize)::value_type>);
    if constexpr (vctr::is::number<TestType>) // if v is numeric, we know it will not be aligned
        REQUIRE (fromUnalignedPtrAndSize.getStorageInfo().dataIsSIMDAligned == false);

    if constexpr (vctr::is::number<TestType>)
    {
        // Specify alignment explicitly. This should lead to a fully compile time specified storage info
        vctr::Span fromAlignedPtrAndSize (v.data(), v.size(), vctr::simdAlignedSpanStorageInfo<TestType>());
        auto storageInfo = fromAlignedPtrAndSize.getStorageInfo();

        REQUIRE (storageInfo.dataIsSIMDAligned == true);
        static_assert (std::same_as<TestType, typename decltype (fromAlignedPtrAndSize)::element_type>);
    }

    // Automatically converting a container with data and size interface
    vctr::Span fromMutableVector (v);

    static_assert (std::same_as<TestType, typename decltype (fromMutableVector)::element_type>);
    REQUIRE (fromMutableVector.data() == v.data());
    REQUIRE (fromMutableVector.size() == v.size());
    REQUIRE_THAT (fromMutableVector, vctr::Equals (v));
    fromMutableVector[2] = cv[0];
    REQUIRE (v[2] == cv[0]);

    vctr::Span fromConstVector (cv);
    static_assert (std::same_as<const TestType, typename decltype (fromConstVector)::element_type>);
    REQUIRE (fromConstVector.data() == cv.data());
    REQUIRE (fromConstVector.size() == cv.size());
    REQUIRE_THAT (fromConstVector, vctr::Equals (cv));

    auto fn = [&] (const vctr::Span<TestType> s) { s[4] = cv[1]; };
    fn (v);
    REQUIRE (v[4] == cv[1]);
}
