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

template <is::anyVctrOrExpression V>
std::ostream& operator<< (std::ostream& s, const V& vec)
{
    if (vec.empty())
    {
        s << "[ ]";
        return s;
    }
  
    s << "[ ";

    const auto n = vec.size() - 1;

    size_t i = 0;
    for (; i < n; ++i)
        s << vec[i] << ", ";

    s << vec[i] << " ]";

    return s;
}

} // namespace vctr
