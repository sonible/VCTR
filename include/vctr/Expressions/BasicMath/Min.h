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
requires std::totally_ordered<ValueType<SrcType>>
class Min : public ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Min, src)

    static constexpr value_type reductionResultInitValue = std::numeric_limits<value_type>::max();

    VCTR_FORCEDINLINE constexpr void reduceElementWise (value_type& result, size_t i) const
    {
        result = std::min (result, src[i]);
    }

    //==============================================================================
    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForAccelerateRealFloatVectorReductionOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        return Expression::Accelerate::min (src.data(), size());
    };

    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForIppRealFloatVectorReductionOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        return Expression::IPP::min (src.data(), sizeToInt (size()));
    };

    //==============================================================================
    VCTR_FORWARD_PREPARE_SIMD_EVALUATION_UNARY_EXPRESSION_MEMBER_FUNCTIONS

    VCTR_FORCEDINLINE void reduceNeonRegisterWise (NeonRegister<value_type>& result, size_t i) const
    requires Config::archARM && has::getNeon<SrcType> && (is::realFloatNumber<value_type> || is::int32Number<value_type>)
    {
        result = Expression::Neon::min (result, src.getNeon (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && is::realFloatNumber<value_type>
    {
        result = Expression::AVX::min (result, src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && is::int32Number<value_type>
    {
        result = Expression::AVX::min (result, src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getSSE<SrcType> && (is::realFloatNumber<value_type> || is::int32Number<value_type>)
    {
        result = Expression::SSE::min (result, src.getSSE (i));
    }

    //==============================================================================
    template <size_t n>
    VCTR_FORCEDINLINE static constexpr value_type finalizeReduction (const std::array<value_type, n>& minima)
    {
        if constexpr (n == 1)
            return minima[0];

        return *std::min_element (minima.begin(), minima.end());
    }
};

template <size_t extent, class SrcType>
requires is::number<ValueType<SrcType>>
class MinAbs : public ExpressionTemplateBase
{
public:
    using value_type = RealType<ValueType<SrcType>>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (MinAbs, src)

    static constexpr value_type reductionResultInitValue = std::numeric_limits<value_type>::max();

    VCTR_FORCEDINLINE constexpr void reduceElementWise (value_type& result, size_t i) const
    requires is::signedNumber<value_type>
    {
        result = std::min (result, std::abs (src[i]));
    }

    VCTR_FORCEDINLINE constexpr void reduceElementWise (value_type& result, size_t i) const
    requires (! is::signedNumber<value_type>)
    {
        result = std::min (result, src[i]);
    }

    //==============================================================================
    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForIppRealFloatVectorReductionOp<SrcType, value_type>
    {
        return Expression::IPP::minAbs (src.data(), sizeToInt (size()));
    };

    //==============================================================================
    VCTR_FORWARD_PREPARE_SIMD_EVALUATION_UNARY_EXPRESSION_MEMBER_FUNCTIONS

    VCTR_FORCEDINLINE void reduceNeonRegisterWise (NeonRegister<value_type>& result, size_t i) const
    requires Config::archARM && has::getNeon<SrcType> && (is::realFloatNumber<value_type> || std::same_as<int32_t, value_type>)
    {
        result = Expression::Neon::min (result, Expression::Neon::abs (src.getNeon (i)));
    }

    VCTR_FORCEDINLINE void reduceNeonRegisterWise (NeonRegister<value_type>& result, size_t i) const
    requires Config::archARM && has::getNeon<SrcType> && std::same_as<uint32_t, value_type>
    {
        result = Expression::Neon::min (result, src.getNeon (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && is::realFloatNumber<value_type>
    {
        static const auto avxSignBit = Expression::AVX::broadcast (typename Expression::CommonElement::Type (-0.0));

        result = Expression::AVX::min (result, Expression::AVX::andNot (avxSignBit, src.getAVX (i)));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && std::same_as<int32_t, value_type>
    {
        result = Expression::AVX::min (result, Expression::AVX::abs (src.getAVX (i)));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && std::same_as<uint32_t, value_type>
    {
        result = Expression::AVX::min (result, src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getSSE<SrcType> && is::realFloatNumber<value_type>
    {
        static const auto sseSignBit = Expression::SSE::broadcast (typename Expression::CommonElement::Type (-0.0));

        result = Expression::SSE::min (result, Expression::SSE::andNot (sseSignBit, src.getSSE (i)));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getSSE<SrcType> && std::same_as<int32_t, value_type>
    {
        result = Expression::SSE::min (result, Expression::SSE::abs (src.getSSE (i)));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getSSE<SrcType> && std::same_as<uint32_t, value_type>
    {
        result = Expression::SSE::min (result, src.getSSE (i));
    }

    //==============================================================================
    template <size_t n>
    VCTR_FORCEDINLINE static constexpr value_type finalizeReduction (const std::array<value_type, n>& minima)
    {
        if constexpr (n == 1)
            return minima[0];

        return *std::min_element (minima.begin(), minima.end());
    }
};

} // namespace vctr::expressions

namespace vctr
{

/** Computes the minimum value of the source values.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::Min> min;

/** Computes the minimum value of the absolute value of the source values.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::MinAbs> minAbs;
}
