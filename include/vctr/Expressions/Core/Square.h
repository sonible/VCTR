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

namespace vctr::Expressions
{

template <size_t extent, class SrcType>
class Square : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Square)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        const auto s = src[i];
        return s * s;
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealOrComplexFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        const auto* s = src.evalNextVectorOpInExpressionChain (dst);
        Expression::Accelerate::mul (s, s, dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        const auto* s = src.evalNextVectorOpInExpressionChain (dst);
        Expression::IPP::mul (s, s, dst, size());
        return dst;
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires archX64 && has::getAVX<SrcType> && Expression::CommonElement::isFloatingPoint
    {
        auto x = src.getAVX (i);
        return Expression::AVX::mul (x, x);
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires archX64 && has::getSSE<SrcType> && Expression::CommonElement::isFloatingPoint
    {
        auto x = src.getSSE (i);
        return Expression::SSE::mul (x, x);
    }

private:
    SrcType src;
};

} // namespace vctr::Expressions

namespace vctr
{

/** Squares the source values.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<Expressions::Square> square;

} // namespace vctr
