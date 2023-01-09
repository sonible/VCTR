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

template <size_t extent, class SrcType, is::constantWrapper ConstantType>
class ClampLowByConstant : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    static constexpr value_type lowerBound = ConstantType::value;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (ClampLowByConstant)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return std::max (lowerBound, src[i]);
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        Expression::Accelerate::threshold (src.evalNextVectorOpInExpressionChain (dst), lowerBound, dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::threshold (src.evalNextVectorOpInExpressionChain (dst), lowerBound, dst, sizeToInt (size()));
        return dst;
    }

private:
    SrcType src;
};

} // namespace vctr::Expressions

namespace vctr
{

/** Ensures that the vector are not lower than lowerLimit.

    @ingroup Expressions
 */
template <auto lowerLimit>
constexpr inline ExpressionChainBuilder<Expressions::ClampLowByConstant, Constant<lowerLimit>> clampLowByConstant;

} // namespace vctr
