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
requires is::realOrComplexFloatNumber<ValueType<SrcType>>
class Cosh : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Cosh, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
#if VCTR_USE_GCEM
        if constexpr (is::realFloatNumber<value_type>)
        {
            if (std::is_constant_evaluated())
                return gcem::cosh (src[i]);
        }
#endif

        return std::cosh (src[i]);
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        Expression::Accelerate::cosh (src.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::cosh (src.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
        return dst;
    }

    //==============================================================================
    // Apple specific SIMD Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires is::suitableForAccelerateSSEOp<value_type, SrcType>
    {
        return { vcoshf (src.getSSE (i).value) };
    }

    NeonRegister<value_type> getNeon (size_t i) const
    requires is::suitableForAccelerateNeonOp<value_type, SrcType>
    {
        return { vcoshf (src.getNeon (i).value) };
    }
};

} // namespace vctr::expressions

namespace vctr
{

/** Computes the hyperbolic cosine of each source element.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::Cosh> cosh;

} // namespace vctr
