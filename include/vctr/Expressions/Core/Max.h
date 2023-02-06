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
requires std::totally_ordered<ValueType<SrcType>>
class Max : public ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Max)

    static constexpr value_type reductionResultInitValue = std::numeric_limits<value_type>::lowest();

    VCTR_FORCEDINLINE void reduceElementWise (value_type& result, size_t i) const
    {
        result = std::max (result, src[i]);
    }

    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForAccelerateRealFloatVectorReductionOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        return Expression::Accelerate::max (src.data(), size());
    };

    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForIppRealFloatVectorReductionOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        return Expression::IPP::max (src.data(), size());
    };

    VCTR_FORCEDINLINE void reduceNeonRegisterWise (NeonRegister<value_type>& result, size_t i) const
    requires Config::archARM && has::getNeon<SrcType> && (is::floatNumber<value_type> || is::int32Number<value_type>)
    {
        result = Expression::Neon::max (result, src.getNeon (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && is::floatNumber<value_type>
    {
        result = Expression::AVX::max (result, src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && is::int32Number<value_type>
    {
        result = Expression::AVX::max (result, src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getSSE<SrcType> && (is::floatNumber<value_type> || is::int32Number<value_type>)
    {
        result = Expression::SSE::max (result, src.getSSE (i));
    }

    template <size_t n>
    VCTR_FORCEDINLINE static value_type finalizeReduction (const std::array<value_type, n>& maxima)
    {
        if constexpr (n == 1)
            return maxima[0];

        return *std::max_element (maxima.begin(), maxima.end());
    }

private:
    SrcType src;
};

template <size_t extent, class SrcType>
requires is::number<ValueType<SrcType>>
class MaxAbs : public ExpressionTemplateBase
{
public:
    using value_type = RealType<ValueType<SrcType>>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (MaxAbs)

    static constexpr value_type reductionResultInitValue = 0;

    VCTR_FORCEDINLINE void reduceElementWise (value_type& result, size_t i) const
    requires is::signedNumber<value_type>
    {
        result = std::max (result, std::abs (src[i]));
    }

    VCTR_FORCEDINLINE void reduceElementWise (value_type& result, size_t i) const
    requires (! is::signedNumber<value_type>)
    {
        result = std::max (result, src[i]);
    }

    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForIppRealFloatVectorReductionOp<SrcType, value_type>
    {
        return Expression::IPP::maxAbs (src.data(), size());
    };

    VCTR_FORCEDINLINE void reduceNeonRegisterWise (NeonRegister<value_type>& result, size_t i) const
    requires Config::archARM && has::getNeon<SrcType> && (is::floatNumber<value_type> || std::same_as<int32_t, value_type>)
    {
        result = Expression::Neon::max (result, Expression::Neon::abs (src.getNeon (i)));
    }

    VCTR_FORCEDINLINE void reduceNeonRegisterWise (NeonRegister<value_type>& result, size_t i) const
    requires Config::archARM && has::getNeon<SrcType> && std::same_as<uint32_t, value_type>
    {
        result = Expression::Neon::max (result, src.getNeon (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && is::floatNumber<value_type>
    {
        static const auto avxSignBit = Expression::AVX::broadcast (typename Expression::CommonElement::Type (-0.0));

        result = Expression::AVX::max (result, Expression::AVX::andNot (avxSignBit, src.getAVX (i)));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && std::same_as<int32_t, value_type>
    {
        result = Expression::AVX::max (result, Expression::AVX::abs (src.getAVX (i)));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && std::same_as<uint32_t, value_type>
    {
        result = Expression::AVX::max (result, src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getSSE<SrcType> && is::floatNumber<value_type>
    {
        static const auto sseSignBit = Expression::SSE::broadcast (typename Expression::CommonElement::Type (-0.0));

        result = Expression::SSE::max (result, Expression::SSE::andNot (sseSignBit, src.getSSE (i)));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getSSE<SrcType> && std::same_as<int32_t, value_type>
    {
        result = Expression::SSE::max (result, Expression::SSE::abs (src.getSSE (i)));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getSSE<SrcType> && std::same_as<uint32_t, value_type>
    {
        result = Expression::SSE::max (result, src.getSSE (i));
    }

    template <size_t n>
    VCTR_FORCEDINLINE static value_type finalizeReduction (const std::array<value_type, n>& maxima)
    {
        if constexpr (n == 1)
            return maxima[0];

        return *std::max_element (maxima.begin(), maxima.end());
    }

private:
    SrcType src;
};

} // namespace vctr::Expressions

namespace vctr
{

/** Computes the maximum value of the source values.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<Expressions::Max> max;

/** Computes the maximum value of the absolute value of the source values.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<Expressions::MaxAbs> maxAbs;

} // namespace vctr
