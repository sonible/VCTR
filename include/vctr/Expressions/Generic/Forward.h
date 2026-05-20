/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2026- by sonible GmbH.

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
class Forward : public ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Forward, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return src[i];
    }

    //==============================================================================
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires has::evalNextVectorOpInExpressionChain<SrcType, value_type>
    {
        if (auto* srcData = src.evalNextVectorOpInExpressionChain (dst); srcData != dst)
            std::copy_n (srcData, size(), dst);

        return dst;
    }

    //==============================================================================
    VCTR_FORWARD_PREPARE_SIMD_EVALUATION_UNARY_EXPRESSION_MEMBER_FUNCTIONS

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType>)
    {
        return src.getSSE (i);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("fma") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && is::realFloatNumber<value_type>)
    {
        return src.getAVX (i);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && ! is::realFloatNumber<value_type>)
    {
        return src.getAVX (i);
    }

    VCTR_FORCEDINLINE NeonRegister<value_type> getNeon (size_t i) const
    requires (archARM && has::getNeon<SrcType>)
    {
        return src.getNeon (i);
    }
};

} // namespace vctr::expressions

namespace vctr
{

/** An expression that forwards the source unaltered.

    This is mainly useful as generic building block to insert in situations where an expression is required
    syntactically but nothing should be altered.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::Forward> forward;

}
