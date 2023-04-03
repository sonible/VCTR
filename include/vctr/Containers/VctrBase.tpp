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
constexpr void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator*= (const V& v)
{
    const auto& self = *this;
    assignExpressionTemplate (Expressions::MultiplyVectors<extent, decltype (self), const V&> (self, v));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
constexpr void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator*= (value_type c)
{
    const auto& self = *this;
    assignExpressionTemplate (Expressions::MultiplyVecBySingle<extent, decltype (self)> (c, self));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
template <is::anyVctrOrExpression V>
constexpr void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator/= (const V& v)
{
    const auto& self = *this;
    assignExpressionTemplate (Expressions::DivideVectors<extent, decltype (self), const V&> (self, v));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
constexpr void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator/= (value_type c)
{
    const auto& self = *this;
    assignExpressionTemplate (Expressions::DivideVecBySingle<extent, decltype (self)> (c, self));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
template <is::anyVctrOrExpression V>
constexpr void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator+= (const V& v)
{
    // Special case: If V is a multiplication expression which has no chained expressions as sources,
    // it might implement a special accelerated multiply accumulate function.
    if constexpr (detail::IsMultiplicationExpression<V>::value && requires { v.evalVectorOpMultiplyAccumulate (data()); })
    {
        v.evalVectorOpMultiplyAccumulate (data());
        return;
    }

    const auto& self = *this;
    assignExpressionTemplate (Expressions::AddVectors<extent, decltype (self), const V&> (self, v));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
constexpr void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator+= (value_type c)
{
    const auto& self = *this;
    assignExpressionTemplate (Expressions::AddSingleToVec<extent, decltype (self)> (c, self));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
template <is::anyVctrOrExpression V>
constexpr void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator-= (const V& v)
{
    const auto& self = *this;
    assignExpressionTemplate (Expressions::SubtractVectors<extent, decltype (self), const V&> (self, v));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
constexpr void VctrBase<ElementType, StorageType, extent, StorageInfoType>::operator-= (value_type c)
{
    const auto& self = *this;
    assignExpressionTemplate (Expressions::SubtractSingleFromVec<extent, decltype (self)> (c, self));
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
ElementType VctrBase<ElementType, StorageType, extent, StorageInfoType>::min() const
requires std::totally_ordered<ElementType>
{
    return vctr::min << *this;
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
ElementType VctrBase<ElementType, StorageType, extent, StorageInfoType>::minAbs() const
requires is::number<ElementType>
{
    return vctr::minAbs << *this;
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
ElementType VctrBase<ElementType, StorageType, extent, StorageInfoType>::max() const
requires std::totally_ordered<ElementType>
{
    return vctr::max << *this;
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
ElementType VctrBase<ElementType, StorageType, extent, StorageInfoType>::maxAbs() const
requires is::number<ElementType>
{
    return vctr::maxAbs << *this;
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
ElementType VctrBase<ElementType, StorageType, extent, StorageInfoType>::mean() const
requires is::number<ElementType>
{
    return vctr::mean << *this;
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
ElementType VctrBase<ElementType, StorageType, extent, StorageInfoType>::meanSquare() const
requires is::number<ElementType>
{
    return vctr::meanSquare << *this;
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
ElementType VctrBase<ElementType, StorageType, extent, StorageInfoType>::rms() const
requires is::number<ElementType>
{
    return vctr::rms << *this;
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
ElementType VctrBase<ElementType, StorageType, extent, StorageInfoType>::sum() const
requires has::operatorPlusEquals<ElementType>
{
    return vctr::sum << *this;
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
constexpr bool VctrBase<ElementType, StorageType, extent, StorageInfoType>::allElementsAreFinite()
requires std::floating_point<ElementType>
{
    return all ([] (value_type val) { return std::isfinite (val); });
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
constexpr bool VctrBase<ElementType, StorageType, extent, StorageInfoType>::allElementsAreFinite()
requires is::complexFloatNumber<ElementType>
{
    return all ([] (auto val) { return std::isfinite (val.real()) && std::isfinite (val.imag()); });
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
constexpr bool VctrBase<ElementType, StorageType, extent, StorageInfoType>::anyElementIsNaN()
requires std::floating_point<ElementType>
{
    return any ([] (value_type val) { return std::isnan (val); });
}

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
constexpr bool VctrBase<ElementType, StorageType, extent, StorageInfoType>::anyElementIsNaN()
requires is::complexFloatNumber<ElementType>
{
    return any ([] (auto val) { return std::isnan (val.real()) || std::isnan (val.imag()); });
}

} // namespace vctr
