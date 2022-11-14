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

/** Checks if all elements in the vector it matches against equal the reference value */
template <class ElementType>
struct AllElementsEqualTo : Catch::Matchers::MatcherGenericBase
{
    AllElementsEqualTo (const ElementType& r)
        : reference (r)
    {}

    template <class Vec>
    bool match (const Vec& vec) const
    {
        return std::all_of (vec.begin(), vec.end(), [&] (auto& e) { return e == reference; });
    }

    std::string describe() const override
    {
        return "All elements equal to: " + Catch::StringMaker<ElementType>::convert (reference);
    }

private:
    const ElementType& reference;
};
} // namespace vctr
