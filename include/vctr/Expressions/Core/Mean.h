/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2023 by sonible GmbH.

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
requires is::number<ValueType<SrcType>>
class Mean : public ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Mean)

    static constexpr value_type reductionResultInitValue = 0;

    VCTR_FORCEDINLINE void reduceElementWise (value_type& result, size_t i) const
    {
        result += src[i];
    }

    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForAccelerateRealFloatVectorReductionOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        return Expression::Accelerate::mean (src.data(), size());
    };

    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForIppRealOrComplexFloatVectorReductionOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        return Expression::IPP::mean (src.data(), sizeToInt (size()));
    };

    VCTR_FORCEDINLINE void reduceNeonRegisterWise (NeonRegister<value_type>& result, size_t i) const
    requires Config::archARM && has::getNeon<SrcType> && (is::floatNumber<value_type> || is::int32Number<value_type>)
    {
        result = Expression::Neon::add (result, src.getNeon (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && is::floatNumber<value_type>
    {
        result = Expression::AVX::add (result, src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && is::int32Number<value_type>
    {
        result = Expression::AVX::add (result, src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getSSE<SrcType> && (is::floatNumber<value_type> || is::int32Number<value_type>)
    {
        result = Expression::SSE::add (result, src.getSSE (i));
    }

    template <size_t n>
    VCTR_FORCEDINLINE value_type finalizeReduction (const std::array<value_type, n>& sums) const
    {
        auto sum = n == 1 ? sums[0] : std::reduce (sums.begin(), sums.end());

        return sum / RealType<value_type> (src.size());
    }

private:
    SrcType src;
};

template <size_t extent, class SrcType>
requires is::number<ValueType<SrcType>>
class MeanSquare : public ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (MeanSquare)

    static constexpr value_type reductionResultInitValue = 0;

    VCTR_FORCEDINLINE void reduceElementWise (value_type& result, size_t i) const
    {
        auto s = src[i];
        result += s * s;
    }

    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForAccelerateRealFloatVectorReductionOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        return Expression::Accelerate::meanSquare (src.data(), size());
    };

    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForIppRealFloatVectorReductionOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        auto l2Norm = Expression::IPP::l2Norm (src.data(), sizeToInt (size()));
        return (l2Norm * l2Norm) / value_type (size());
    };

    VCTR_FORCEDINLINE void reduceNeonRegisterWise (NeonRegister<value_type>& result, size_t i) const
    requires Config::archARM && has::getNeon<SrcType> && (is::floatNumber<value_type> || is::int32Number<value_type>)
    {
        auto s = src.getNeon (i);
        s = Expression::Neon::mul (s, s);
        result = Expression::Neon::add (result, s);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && is::floatNumber<value_type>
    {
        auto s = src.getAVX (i);
        s = Expression::AVX::mul (s, s);
        result = Expression::AVX::add (result, s);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getSSE<SrcType> && is::floatNumber<value_type>
    {
        auto s = src.getSSE (i);
        s = Expression::SSE::mul (s, s);
        result = Expression::SSE::add (result, s);
    }

    template <size_t n>
    VCTR_FORCEDINLINE value_type finalizeReduction (const std::array<value_type, n>& sums) const
    {
        auto sum = n == 1 ? sums[0] : std::reduce (sums.begin(), sums.end());

        return sum / value_type (size());
    }

private:
    SrcType src;
};

template <size_t extent, class SrcType>
requires is::number<ValueType<SrcType>>
class RootMeanSquare : public ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (RootMeanSquare)

    static constexpr value_type reductionResultInitValue = 0;

    VCTR_FORCEDINLINE void reduceElementWise (value_type& result, size_t i) const
    {
        auto s = src[i];
        result += s * s;
    }

    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForAccelerateRealFloatVectorReductionOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        return Expression::Accelerate::rms (src.data(), size());
    };

    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForIppRealFloatVectorReductionOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        auto l2Norm = Expression::IPP::l2Norm (src.data(), sizeToInt (size()));
        return std::sqrt ((l2Norm * l2Norm) / value_type (size()));
    };

    VCTR_FORCEDINLINE void reduceNeonRegisterWise (NeonRegister<value_type>& result, size_t i) const
    requires Config::archARM && has::getNeon<SrcType> && (is::floatNumber<value_type> || is::int32Number<value_type>)
    {
        auto s = src.getNeon (i);
        s = Expression::Neon::mul (s, s);
        result = Expression::Neon::add (result, s);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && is::floatNumber<value_type>
    {
        auto s = src.getAVX (i);
        s = Expression::AVX::mul (s, s);
        result = Expression::AVX::add (result, s);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getSSE<SrcType> && is::floatNumber<value_type>
    {
        auto s = src.getSSE (i);
        s = Expression::SSE::mul (s, s);
        result = Expression::SSE::add (result, s);
    }

    template <size_t n>
    VCTR_FORCEDINLINE value_type finalizeReduction (const std::array<value_type, n>& sums) const
    {
        auto sum = n == 1 ? sums[0] : std::reduce (sums.begin(), sums.end());

        return std::sqrt (sum / value_type (src.size()));
    }

private:
    SrcType src;
};

} // namespace vctr::Expressions

namespace vctr
{

/** Computes the mean value of the source values.

    @ingroup Expressions
 */
constexpr ExpressionChainBuilder<Expressions::Mean> mean;

/** Computes the mean value of the squared source values.

    @ingroup Expressions
 */
constexpr ExpressionChainBuilder<Expressions::MeanSquare> meanSquare;

/** Computes the square root of the mean value of the squared source values.

    @ingroup Expressions
 */
constexpr ExpressionChainBuilder<Expressions::RootMeanSquare> rms;

} // namespace vctr