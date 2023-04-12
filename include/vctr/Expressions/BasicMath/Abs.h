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

namespace vctr::expressions
{

template <size_t extent, class SrcType>
class Abs : public ExpressionTemplateBase
{
public:
    using value_type = RealType<ValueType<SrcType>>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Abs, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    requires is::signedNumber<value_type>
    {
        return std::abs (src[i]);
    }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    requires is::unsignedIntNumber<value_type>
    {
        return src[i];
    }

    // clang-format off
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp         <SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable> ||
             is::suitableForAccelerateComplexToRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    // clang-format on
    {
        Expression::Accelerate::abs (src.evalNextVectorOpInExpressionChain (dst), dst, size());
        return dst;
    }

    // clang-format off
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealSingedInt32VectorOp   <SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable> ||
             is::suitableForIppRealFloatVectorOp         <SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable> ||
             is::suitableForIppComplexToRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    // clang-format on
    {
        Expression::IPP::abs (src.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
        return dst;
    }

    //==============================================================================
    NeonRegister<value_type> getNeon (size_t i) const
    requires (archARM && has::getNeon<SrcType> && Expression::CommonElement::isSigned)
    {
        return Expression::Neon::abs (src.getNeon (i));
    }

    NeonRegister<value_type> getNeon (size_t i) const
    requires (archARM && has::getNeon<SrcType> && ! Expression::CommonElement::isSigned)
    {
        return src.getNeon (i); // unsigned integers are always positive
    }

    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::CommonElement::isFloatingPoint)
    {
        static const auto avxSignBit = Expression::AVX::broadcast (typename Expression::CommonElement::Type (-0.0));

        return Expression::AVX::andNot (avxSignBit, src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::CommonElement::isInt32)
    {
        return Expression::AVX::abs (src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::CommonElement::isUnsigned)
    {
        return src.getAVX (i); // unsigned integers are always positive
    }

    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::CommonElement::isFloatingPoint)
    {
        static const auto sseSignBit = Expression::SSE::broadcast (typename Expression::CommonElement::Type (-0.0));

        return Expression::SSE::andNot (sseSignBit, src.getSSE (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::CommonElement::isInt32)
    {
        return Expression::SSE::abs (src.getSSE (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::CommonElement::isUnsigned)
    {
        return src.getSSE (i); // unsigned integers are always positive
    }
};

} // namespace vctr::expressions

namespace vctr
{
/** Computes the absolute value of the source values.

    Complex source values are transformed into real values. Unsigned integers remain unaltered.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::Abs> abs;

} // namespace vctr
