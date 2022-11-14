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

TEMPLATE_TEST_CASE ("Assign VCTR to containers with other allocator", "[ConversionOperators][Allocators]", float, int64_t, std::string)
{
    const auto v = UnitTestValues<TestType>::template vector<10, 0>();

    const std::vector<TestType> stdVec = v;
    const vctr::Vector<TestType, std::allocator> stdAllocatorV = v;

    REQUIRE_THAT (stdVec, vctr::Equals (v));
    REQUIRE_THAT (stdAllocatorV, vctr::Equals (v));
}
