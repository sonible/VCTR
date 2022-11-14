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

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
template <size_t spanExtent>
constexpr auto VctrBase<ElementType, StorageType, extent, StorageInfoType>::constCorrectSpan (ElementType* data, size_t spanSize)
{
    return Span<ElementType, spanExtent> (data, spanSize);
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
template <size_t spanExtent>
constexpr auto VctrBase<ElementType, StorageType, extent, StorageInfoType>::constCorrectSpan (const ElementType* data, size_t spanSize) const
requires (! is::stdSpan<StorageType>)
{
    return Span<const ElementType, spanExtent> (data, spanSize);
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
template <size_t spanExtent>
constexpr auto VctrBase<ElementType, StorageType, extent, StorageInfoType>::constCorrectSpan (ElementType* data, size_t spanSize) const
requires is::stdSpan<StorageType>
{
    return Span<ElementType, spanExtent> (data, spanSize);
}

//==============================================================================
template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
template <is::anyVctrOrExpression V>
void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator*= (const V& v)
{
    const auto& self = *this;
    assignExpressionTemplate (MultiplyVectors<extent, decltype (self), const V&> (self, v));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator*= (value_type c)
{
    const auto& self = *this;
    assignExpressionTemplate (MultiplyVecBySingle<extent, decltype (self)> (c, self));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
template <is::anyVctrOrExpression V>
void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator/= (const V& v)
{
    const auto& self = *this;
    assignExpressionTemplate (DivideVectors<extent, decltype (self), const V&> (self, v));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator/= (value_type c)
{
    const auto& self = *this;
    assignExpressionTemplate (DivideVecBySingle<extent, decltype (self)> (self, c));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
template <is::anyVctrOrExpression V>
void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator+= (const V& v)
{
    const auto& self = *this;
    assignExpressionTemplate (AddVectors<extent, decltype (self), const V&> (self, v));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator+= (value_type c)
{
    const auto& self = *this;
    assignExpressionTemplate (AddSingleToVec<extent, decltype (self)> (c, self));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
template <is::anyVctrOrExpression V>
void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator-= (const V& v)
{
    const auto& self = *this;
    assignExpressionTemplate (SubtractVectors<extent, decltype (self), const V&> (self, v));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator-= (value_type c)
{
    const auto& self = *this;
    assignExpressionTemplate (SubtractSingleFromVec<extent, decltype (self)> (self, c));
}


} // namespace vctr
