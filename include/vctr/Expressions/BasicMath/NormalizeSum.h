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

namespace vctr::expressions
{

template <size_t extent, class SrcType>
requires is::realFloatNumber<ValueType<SrcType>>
class NormalizeSum : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (NormalizeSum, src)

    constexpr void applyRuntimeArgs()
    {
        srcSum = sum << src;
    }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        VCTR_ASSERT (srcSum != value_type (0));
        VCTR_START_IGNORE_WARNING_MSVC (4723) // potential divide by 0
        return src[i] / srcSum;
        VCTR_END_IGNORE_WARNING_MSVC
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        VCTR_ASSERT (srcSum != value_type (0));
        Expression::Accelerate::div (src.evalNextVectorOpInExpressionChain (dst), srcSum, dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        VCTR_ASSERT (srcSum != value_type (0));
        Expression::IPP::div (src.evalNextVectorOpInExpressionChain (dst), srcSum, dst, sizeToInt (size()));
        return dst;
    }

private:
    //==============================================================================
    value_type srcSum = value_type (0);
};

}

namespace vctr
{

/** Computes the sum of the source elements and divides all source elements by that sum.

    Note: If the source is an expression itself, that source expression will possibly be evaluated twice
    since we need to evaluate it once to compute the sum and then again to compute the normalized expression
    result. For performance critical applications, you should consider buffering a source expression to an
    intermediate container.

    @attention The required sum is computed once when the source is assigned to the expression. In case
    the source values are changed after that time point, evaluating the expression will use an outdated
    sum. Therefore, we might consider deprecating and eventually removing this expression in a future release.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::NormalizeSum> normalizeSum;
}