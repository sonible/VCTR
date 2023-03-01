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

namespace vctr::generators
{
/** Collection of static methods for generating evenly spaced numbers. */
struct Linspace
{
    /** Returns a Vector with evenly spaced numbers between start and stop.

        Properties:
        - If start and stop are identical, the container will be filled with that value.
        - Ranges can have a negative increment, i.e., start > stop is explicitly allowed.

        @tparam ElementType Element type contained in the vector.

        @param start        First value of the interval; always included.
        @param stop         Last value of the interval; only included if includeEnd is true.
        @param num          Number of elements (length of the vector), must be > 0.
        @param includeEnd If this is true, both start and stop are included; otherwise
                          only start is included.
     */
    template <is::realNumber ElementType>
    static Vector<ElementType> makeVector (ElementType start, ElementType stop, size_t num, bool includeEnd = true)
    {
        Vector<ElementType> dst (num);
        dst.fillLinspace (start, stop, includeEnd);
        return dst;
    }

    /** Like makeVector, but returns an Array. */
    template <is::realNumber ElementType, size_t num>
    static constexpr Array<ElementType, num> makeArray (ElementType start, ElementType stop, bool includeEnd = true)
    {
        Array<ElementType, num> dst;
        dst.fillLinspace (start, stop, includeEnd);
        return dst;
    }

    /** Returns a Vector with values between start and stop and a step of 1.

        Properties:
        - The number of elements is stop - start + 1.
        - If start and stop are identical, the container will be filled with that value.
        - Ranges can have a negative increment, i.e., start > stop is explicitly allowed.

        @Note: stop - start must not be fractional.

        @tparam ElementType Element type contained in the vector.

        @param start        First value of the interval; always included.
        @param stop         Last value of the interval; only included if includeEnd is true.
        @param includeEnd   If this is true, the numbers lie on the closed interval
                            [start, stop]; otherwise the interval is half-open [start, stop).
     */
    template <is::realNumber ElementType>
    static Vector<ElementType> makeVector (ElementType start, ElementType stop, bool includeEnd = true)
    {
        const auto distance = stop - start;

        VCTR_ASSERT (distance - std::floor (distance) == 0);

        const auto num = std::abs (int (distance)) + 1;
        return makeVector (start, stop, num, includeEnd);
    }
};
} // namespace vctr::generators
