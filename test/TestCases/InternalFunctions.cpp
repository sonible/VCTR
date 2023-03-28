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

TEST_CASE ("detail::previousMultipleOf", "[internal]")
{
    REQUIRE (vctr::detail::previousMultipleOf<2> (0) == 0);
    REQUIRE (vctr::detail::previousMultipleOf<2> (1) == 0);
    REQUIRE (vctr::detail::previousMultipleOf<2> (2) == 2);
    REQUIRE (vctr::detail::previousMultipleOf<2> (3) == 2);

    REQUIRE (vctr::detail::previousMultipleOf<128> (0) == 0);
    REQUIRE (vctr::detail::previousMultipleOf<128> (1) == 0);
    REQUIRE (vctr::detail::previousMultipleOf<128> (50) == 0);
    REQUIRE (vctr::detail::previousMultipleOf<128> (128) == 128);
    REQUIRE (vctr::detail::previousMultipleOf<128> (150) == 128);
    REQUIRE (vctr::detail::previousMultipleOf<128> (257) == 256);
}

TEST_CASE ("detail::nextMultipleOf", "[internal]")
{
    REQUIRE (vctr::detail::nextMultipleOf<2> (0) == 0);
    REQUIRE (vctr::detail::nextMultipleOf<2> (1) == 2);
    REQUIRE (vctr::detail::nextMultipleOf<2> (2) == 2);
    REQUIRE (vctr::detail::nextMultipleOf<2> (3) == 4);

    REQUIRE (vctr::detail::nextMultipleOf<128> (0) == 0);
    REQUIRE (vctr::detail::nextMultipleOf<128> (1) == 128);
    REQUIRE (vctr::detail::nextMultipleOf<128> (50) == 128);
    REQUIRE (vctr::detail::nextMultipleOf<128> (150) == 256);
}