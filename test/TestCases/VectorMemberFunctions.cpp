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

TEST_CASE ("capacity, shrink_to_fit, reserve, clear", "[VectorMemberFunctions]")
{
    auto v = UnitTestValues<int32_t>::template vector<100, 0>();

    REQUIRE (v.capacity() >= 100);

    v.shrink_to_fit();
    // shrink_to_fit() is a non-binding request, so we cannot check == 100 here
    REQUIRE (v.capacity() >= 100);

    v.push_back (42);
    REQUIRE (v.capacity() >= 100);

    v.reserve (200);
    REQUIRE (v.capacity() >= 200);

    v.clear();
    REQUIRE (v.capacity() >= 0);

    v.shrink_to_fit();
    // shrink_to_fit() is a non-binding request, so we cannot check == 0 here
    REQUIRE (v.capacity() >= 0);
}
