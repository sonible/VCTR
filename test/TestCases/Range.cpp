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

TEMPLATE_TEST_CASE ("Range", "[range]", float, double, int)
{
    auto a = UnitTestValues<TestType>::template array<2, 0>();
    a.sort();

    vctr::Range r (a[0], a[1]);

    REQUIRE (r.getStart() == a[0]);
    REQUIRE (r.getEnd() == a[1]);
    REQUIRE (r.getLength() == a[1] - a[0]);

    a -= TestType (2);

    r.setStart (a[0]);
    r.setEnd (a[1]);

    REQUIRE (r.getStart() == a[0]);
    REQUIRE (r.getEnd() == a[1]);
    REQUIRE (r.getLength() == a[1] - a[0]);

    vctr::Range<TestType> empty;
    REQUIRE (empty.getStart() == TestType (0));
    REQUIRE (empty.getEnd() == TestType (0));
    REQUIRE (empty.getLength() == TestType (0));
}