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

#include "detail/ElementwiseEqualityDetails.h"

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
        const auto actualSize = size_t (std::distance (vec.begin(), vec.end()));
        const auto referenceSize = size_t (std::distance (reference.begin(), reference.end()));

        // Query actual/expected elements by advancing these iterators rather than via operator[], so that this
        // also works for containers that only provide forward iterators (e.g. no random access).
        auto itA = vec.begin();
        auto itB = reference.begin();

        // decltype (auto) rather than auto& here, since some iterators (e.g. std::vector<bool>::iterator) return
        // a proxy object by value from operator*, which cannot bind to a non-const lvalue reference.
        return detail::computeElementwiseEqualityDetails<SrcValueType> (
            details, actualSize, referenceSize, margin, epsilon,
            [&itA] (size_t) -> decltype (auto) { decltype (auto) v = *itA; ++itA; return v; },
            [&itB] (size_t) -> decltype (auto) { decltype (auto) v = *itB; ++itB; return v; });
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
        std::ostringstream s;
        s << "Equals: " << Catch::StringMaker<ReferenceVec>::convert (reference);
        details.appendDescriptionTo (s);
        return s.str();
    }

private:
    ReferenceVec reference;
    mutable detail::ElementwiseEqualityDetails details;

    double margin = 0.0;
    double epsilon = 0.0;
};

template <class T>
Equals (std::initializer_list<T>) -> Equals<Vector<T>>;

} // namespace vctr
