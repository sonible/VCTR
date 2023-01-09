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
class PlatformVectorOpsFilter : public ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (PlatformVectorOpsFilter)

    auto* data() const
    requires has::data<SrcType>
    {
        return src.data();
    }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return src[i];
    }

    //==============================================================================
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires has::evalNextVectorOpInExpressionChain<SrcType, value_type>
    {
        return src.evalNextVectorOpInExpressionChain (dst);
    }

private:
    SrcType src;
};

} // namespace vctr::Expressions

namespace vctr
{

/** Filter expression to force platform vector operation based accelerated evaluation.

    This filter expression ensures that only platform vector operation based accelerated evaluation of the previous
    expression is possible. If evalNextVectorOpInExpressionChain is not available on the source, this will only forward
    the basic operator[] based interface.

    @ingroup Expressions
 */
constexpr ExpressionChainBuilder<Expressions::PlatformVectorOpsFilter> usePlatformVectorOps;

} // namespace vctr
