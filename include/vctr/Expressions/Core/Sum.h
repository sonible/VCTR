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

namespace vctr::detail
{
template <class ValueType>
struct SumInit
{
    static constexpr ValueType value {};
};

template <class StringType>
requires std::convertible_to<const char*, StringType>
struct SumInit<StringType>
{
    static constexpr const char* value = "";
};
}

namespace vctr::Expressions
{

template <size_t extent, class SrcType>
requires has::operatorPlusEquals<ValueType<SrcType>>
class Sum : public ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Sum)

    static constexpr auto reductionResultInitValue = detail::SumInit<value_type>::value;

    VCTR_FORCEDINLINE void reduceElementWise (value_type& result, size_t i) const
    {
        result += src[i];
    }

    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForAccelerateRealFloatVectorReductionOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        return Expression::Accelerate::sum (src.data(), size());
    };

    VCTR_FORCEDINLINE value_type reduceVectorOp() const
    requires is::suitableForIppRealOrComplexFloatVectorReductionOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        return Expression::IPP::sum (src.data(), size());
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
    VCTR_FORCEDINLINE static value_type finalizeSIMDReduction (const std::array<value_type, n>& sums)
    {
        return std::reduce (sums.begin(), sums.end());
    }

private:
    SrcType src;
};

} // namespace vctr::Expressions

namespace vctr
{

/** Computes the sum of the source values.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<Expressions::Sum> sum;

} // namespace vctr
