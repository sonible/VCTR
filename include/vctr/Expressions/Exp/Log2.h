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
requires is::realNumber<ValueType<SrcType>>
class Log2 : ExpressionTemplateBase
{
    using SrcValueType = ValueType<SrcType>;

public:
    using value_type = std::conditional_t<is::intNumber<SrcValueType>, float, SrcValueType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Log2, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
#if VCTR_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::log2 (src[i]);
#endif

        return std::log2 (src[i]);
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable> && is::floatNumber<SrcValueType>
    {
        Expression::Accelerate::log2 (src.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealIntToFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable> && is::int32Number<SrcValueType>
    {
        auto s = size();

        Expression::Accelerate::intToFloat (src.evalNextVectorOpInExpressionChain (reinterpret_cast<SrcValueType*> (dst)), dst, s);
        Expression::Accelerate::log2 (dst, dst, sizeToInt (s));

        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        // No direct log2 in IPP. Therefore, we compute the ln and then multiply by 1 / ln (2)
        constexpr auto factor = value_type (1.4426950408889634);

        auto s = sizeToInt (size());

        Expression::IPP::ln (src.evalNextVectorOpInExpressionChain (dst), dst, s);
        Expression::IPP::mul (factor, dst, s);

        return dst;
    }
};

} // namespace vctr::expressions

namespace vctr
{

/** Computes the logarithm to the base of two of the source values.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::Log2> log2;

} // namespace vctr
