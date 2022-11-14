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

namespace vctr
{

/** Casts the size_t argument to an int. In a debug build, this checks for potential overflow issues */
inline int sizeToInt (size_t size)
{
    // You are evaluating an expression on a very big vector which size exceeds the maximum value of an integer.
    // The PlatformVectorOp that the implementation attempts to use in this context is however only implemented for int
    // size arguments, so it's likely to fail on this large vector.
    // To work around the problem, add a filter expression that avoids invoking the operation and forces the compiler
    // to choose a different strategy like vctr::useNeonOrAVX.
    VCTR_ASSERT (size < size_t (std::numeric_limits<int>::max()));

    return int (size);
}

} // namespace vctr
