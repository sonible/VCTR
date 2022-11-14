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

/** Checks if a reference vector contains the same element values as the vector it is matched against */
template <is::anyVctr ReferenceVec>
struct Equals : Catch::Matchers::MatcherGenericBase
{
    using SrcValueType = ValueType<ReferenceVec>;

    Equals (const ReferenceVec& vec)
        : reference (vec)
    {}

    template <class Vec>
    bool match (const Vec& vec) const
    {
        if constexpr (is::number<SrcValueType>)
        {
            return std::equal (vec.begin(), vec.end(), reference.begin(), reference.end(), [&] (auto a, auto b) { return a == Catch::Approx (b).margin (margin).epsilon (epsilon); });
        }
        else
        {
            return std::equal (vec.begin(), vec.end(), reference.begin(), reference.end());
        }
    }

    auto withMargin (double m) &&
    {
        margin = m;
        return std::move (*this);
    }

    auto withEpsilon (double e) &&
    {
        epsilon = e;
        return std::move (*this);
    }

    std::string describe() const override
    {
        return "Equals: " + Catch::StringMaker<ReferenceVec>::convert (reference);
    }

private:
    const ReferenceVec& reference;

    double margin = 0.0;
    double epsilon = 0.0;
};
} // namespace vctr
