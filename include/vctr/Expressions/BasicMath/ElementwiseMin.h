/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2026 by sonible GmbH.

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

namespace vctr::expressions
{
template <size_t extent, class SrcAType, class SrcBType>
requires std::totally_ordered_with<ValueType<SrcAType>, ValueType<SrcBType>>
class ElementwiseMin : ExpressionTemplateBase
{
public:
    using value_type = std::common_type_t<ValueType<SrcAType>, ValueType<SrcBType>>;

    VCTR_COMMON_BINARY_VEC_VEC_EXPRESSION_MEMBERS (ElementwiseMin, srcA, srcB)

    VCTR_FORCEDINLINE constexpr auto operator[] (size_t i) const
    {
        return std::min (srcA[i], srcB[i]);
    }

    //==============================================================================
    VCTR_FORWARD_PREPARE_SIMD_EVALUATION_BINARY_EXPRESSION_MEMBER_FUNCTIONS (srcA, srcB)

    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") AVXRegister<value_type> getAVX (size_t i) const
    requires archX64 && has::getAVX<SrcAType> && has::getAVX<SrcBType> && Expression::CommonElement::isRealFloat
    {
        return Expression::AVX::min (srcA.getAVX (i), srcB.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") AVXRegister<value_type> getAVX (size_t i) const
    requires archX64 && has::getAVX<SrcAType> && has::getAVX<SrcBType> && (Expression::CommonElement::isInt32 || Expression::CommonElement::isUint32)
    {
        return Expression::AVX::min (srcA.getAVX (i), srcB.getAVX (i));
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires archX64 && has::getSSE<SrcAType> && has::getSSE<SrcBType> && (Expression::CommonElement::isRealFloat || Expression::CommonElement::isInt32 || Expression::CommonElement::isUint32)
    {
        return Expression::SSE::min (srcA.getSSE (i), srcB.getSSE (i));
    }

    //==============================================================================
    // NEON Implementation
    NeonRegister<value_type> getNeon (size_t i) const
    requires archARM && has::getNeon<SrcAType> && has::getNeon<SrcBType> && (Expression::CommonElement::isRealFloat || Expression::CommonElement::isInt32 || Expression::CommonElement::isUint32)
    {
        return Expression::Neon::min (srcA.getNeon (i), srcB.getNeon (i));
    }
};

} // namespace vctr::expressions

namespace vctr
{

/** Computes the element-wise minimum of two sources. */
template <is::anyVctrOrExpression SrcAType, is::anyVctrOrExpression SrcBType>
constexpr auto elementwiseMin (SrcAType&& a, SrcBType&& b)
{
    assertCommonSize (a, b);
    constexpr auto extent = getCommonExtent<SrcAType, SrcBType>();

    return expressions::ElementwiseMin<extent, SrcAType, SrcBType> (std::forward<SrcAType> (a), std::forward<SrcBType> (b));
}
} // namespace vctr
