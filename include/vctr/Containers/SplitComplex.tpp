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

// The compound assignment operators build the split-complex expressions defined in SplitComplexMath.h and assign
// them to this container. The assignment evaluates the real and the imaginary parts in a single pass, so it is safe
// that this container is also a source of the expression.

//==============================================================================
template <is::anyVctrWithRealFloatValueType ContainerType>
template <is::anySplitComplexVectorOrExpression V>
constexpr void SplitComplexBase<ContainerType>::operator*= (const V& v) requires isMutable
{
    const auto& self = *this;
    *this = self * v;
}

template <is::anyVctrWithRealFloatValueType ContainerType>
constexpr void SplitComplexBase<ContainerType>::operator*= (value_type c) requires isMutable
{
    const auto& self = *this;
    *this = self * c;
}

template <is::anyVctrWithRealFloatValueType ContainerType>
constexpr void SplitComplexBase<ContainerType>::operator*= (complex_type c) requires isMutable
{
    const auto& self = *this;
    *this = self * c;
}

//==============================================================================
template <is::anyVctrWithRealFloatValueType ContainerType>
template <is::anySplitComplexVectorOrExpression V>
constexpr void SplitComplexBase<ContainerType>::operator/= (const V& v) requires isMutable
{
    const auto& self = *this;
    *this = self / v;
}

template <is::anyVctrWithRealFloatValueType ContainerType>
constexpr void SplitComplexBase<ContainerType>::operator/= (value_type c) requires isMutable
{
    const auto& self = *this;
    *this = self / c;
}

template <is::anyVctrWithRealFloatValueType ContainerType>
constexpr void SplitComplexBase<ContainerType>::operator/= (complex_type c) requires isMutable
{
    const auto& self = *this;
    *this = self / c;
}

//==============================================================================
template <is::anyVctrWithRealFloatValueType ContainerType>
template <is::anySplitComplexVectorOrExpression V>
constexpr void SplitComplexBase<ContainerType>::operator+= (const V& v) requires isMutable
{
    const auto& self = *this;
    *this = self + v;
}

template <is::anyVctrWithRealFloatValueType ContainerType>
constexpr void SplitComplexBase<ContainerType>::operator+= (value_type c) requires isMutable
{
    const auto& self = *this;
    *this = self + complex_type (c);
}

template <is::anyVctrWithRealFloatValueType ContainerType>
constexpr void SplitComplexBase<ContainerType>::operator+= (complex_type c) requires isMutable
{
    const auto& self = *this;
    *this = self + c;
}

//==============================================================================
template <is::anyVctrWithRealFloatValueType ContainerType>
template <is::anySplitComplexVectorOrExpression V>
constexpr void SplitComplexBase<ContainerType>::operator-= (const V& v) requires isMutable
{
    const auto& self = *this;
    *this = self - v;
}

template <is::anyVctrWithRealFloatValueType ContainerType>
constexpr void SplitComplexBase<ContainerType>::operator-= (value_type c) requires isMutable
{
    const auto& self = *this;
    *this = self - complex_type (c);
}

template <is::anyVctrWithRealFloatValueType ContainerType>
constexpr void SplitComplexBase<ContainerType>::operator-= (complex_type c) requires isMutable
{
    const auto& self = *this;
    *this = self - c;
}

} // namespace vctr
