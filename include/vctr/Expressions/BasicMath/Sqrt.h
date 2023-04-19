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
requires is::realFloatNumber<ValueType<SrcType>> || is::complexFloatNumber<ValueType<SrcType>>
class Sqrt : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Sqrt, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        VCTR_ASSERT (inputValueValid (src[i]));
        return std::sqrt (src[i]);
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        const auto* s = src.evalNextVectorOpInExpressionChain (dst);
        VCTR_ASSERT (std::all_of (s, s + size(), inputValueValid));

        Expression::Accelerate::sqrt (s, dst, sizeToInt (size()));
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        const auto* s = src.evalNextVectorOpInExpressionChain (dst);
        VCTR_ASSERT (std::all_of (s, s + size(), inputValueValid));

        Expression::IPP::sqrt (s, dst, sizeToInt (size()));
        return dst;
    }

private:
    /** If the value type is real, this checks if the value is greater or equal zero */
    static constexpr bool inputValueValid (value_type in)
    {
        if constexpr (is::realNumber<value_type>)
            return in >= value_type (0);

        return true;
    }
};

} // namespace vctr::expressions

namespace vctr
{

/** Computes the square root of the source values.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::Sqrt> sqrt;

} // namespace vctr
