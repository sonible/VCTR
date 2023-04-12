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

namespace vctr
{

/** A simple range class.

    The interface is intended to be compatible with the juce::Range class.
 */
template <is::realNumber ValueType>
class Range
{
public:
    using value_type = ValueType;

    //==============================================================================
    /** Constructs a range with given start and end values.

        The end value must not be smaller than the start value.
     */
    constexpr Range (ValueType startValue, ValueType endValue) noexcept
        : start (startValue),
          end (endValue)
    {
        VCTR_ASSERT (end >= start);
    }

    /** Constructs an empty range. */
    constexpr Range() = default;

    /** Copy constructor. */
    constexpr Range (const Range&) = default;

    /** Move constructor. */
    constexpr Range (Range&&) = default;

    //==============================================================================
    /** Sets a new start value. */
    constexpr void setStart (ValueType newStart) noexcept
    {
        start = newStart;
        VCTR_ASSERT (start <= end);
    }

    /** Sets a new end value. */
    constexpr void setEnd (ValueType newEnd) noexcept
    {
        end = newEnd;
        VCTR_ASSERT (start <= end);
    }

    //==============================================================================
    /** Returns the start of the range. */
    constexpr ValueType getStart() const noexcept          { return start; }

    /** Returns the length of the range. */
    constexpr ValueType getLength() const noexcept         { return end - start; }

    /** Returns the end of the range. */
    constexpr ValueType getEnd() const noexcept            { return end; }

private:
    //==============================================================================
    ValueType start { 0 };
    ValueType end { 0 };
};

}
