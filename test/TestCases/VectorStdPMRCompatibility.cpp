/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2023- by sonible GmbH.

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
#if __has_include (<memory_resource>)

#include <memory_resource>
#include <string>
#include <vctr_test_utils/vctr_test_common.h>

TEMPLATE_TEST_CASE ("Vector with a std::pmr::polymorphic_allocator", "[Vector][Constructor]", float, int64_t, std::string)
{
    const auto a = UnitTestValues<TestType>::template array<10, 0>();

    std::pmr::monotonic_buffer_resource resource;
    vctr::Vector<TestType, std::pmr::polymorphic_allocator> pmrVec (&resource);

    pmrVec.push_back (a.back());
    pmrVec.pop_back();
    pmrVec.append (a);
    REQUIRE_THAT (a, vctr::Equals (pmrVec));
}

#endif
