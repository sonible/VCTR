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
requires is::complexFloatNumber<ValueType<SrcType>>
class Imag : public ExpressionTemplateBase
{
public:
    using value_type = RealType<ValueType<SrcType>>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Imag, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return src[i].imag();
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateComplexToRealFloatVectorOp<SrcType, value_type>
    {
        Expression::Accelerate::copyImag (src.data(), dst, sizeToInt (size()));
        return dst;
    }
};

} // namespace vctr::expressions

namespace vctr
{
/** Extracts the imaginary parts of the complex source values.

    In case you want to chain more computationally expensive expressions on the
    complex source values before this expression, consider buffering the complex
    results to an intermediate complex container for better performance.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::Imag> imag;

} // namespace vctr
