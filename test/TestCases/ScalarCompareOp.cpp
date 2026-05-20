/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2026 - by sonible GmbH.

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

TEST_CASE ("ScalarCompareOp", "[VCTR][ScalarCompareOp]")
{
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::less> (-1, 1) == true);
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::less> (1, -1) == false);
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::less> (-1, -1) == false);

    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::lessOrEqual> (-1, 1) == true);
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::lessOrEqual> (1, -1) == false);
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::lessOrEqual> (-1, -1) == true);

    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::greater> (-1, 1) == false);
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::greater> (1, -1) == true);
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::greater> (-1, -1) == false);

    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::greaterOrEqual> (-1, 1) == false);
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::greaterOrEqual> (1, -1) == true);
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::greaterOrEqual> (-1, -1) == true);

    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::equal> (-1, 1) == false);
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::equal> (1, -1) == false);
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::equal> (-1, -1) == true);

    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::notEqual> (-1, 1) == true);
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::notEqual> (1, -1) == true);
    STATIC_REQUIRE (vctr::scalarCompare<vctr::CompareOp::notEqual> (-1, -1) == false);
}