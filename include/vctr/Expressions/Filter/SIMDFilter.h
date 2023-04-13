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
class SSEFilter : public ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (SSEFilter, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return src[i];
    }

    //==============================================================================
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType>)
    {
        return src.getSSE (i);
    }
};

template <size_t extent, class SrcType>
class AVXFilter : public ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (AVXFilter, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return src[i];
    }

    //==============================================================================
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && is::floatNumber<value_type>)
    {
        return src.getAVX (i);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && ! is::floatNumber<value_type>)
    {
        return src.getAVX (i);
    }
};

template <size_t extent, class SrcType>
class NeonFilter : public ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (NeonFilter, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return src[i];
    }

    //==============================================================================
    NeonRegister<value_type> getNeon (size_t i) const
    requires (archARM && has::getNeon<SrcType>)
    {
        return src.getNeon (i);
    }
};

} // namespace vctr::expressions

namespace vctr
{

/** This filter expression ensures that only SSE based accelerated evaluation of the previous expression is possible.

    If getSSE is not available on the source, this will only forward the basic operator[] based interface.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::SSEFilter> useSSE;

/** This filter expression ensures that only AVX based accelerated evaluation of the previous expression is possible.

    If getAVX is not available on the source, this will only forward the basic operator[] based interface.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::AVXFilter> useAVX;

/** This filter expression ensures that only Neon based accelerated evaluation of the previous expression is possible.

    If getNeon is not available on the source, this will only forward the basic operator[] based interface.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::NeonFilter> useNeon;

#if VCTR_ARM
constexpr inline ExpressionChainBuilder<expressions::NeonFilter> useNeonOrAVX;
constexpr inline ExpressionChainBuilder<expressions::NeonFilter> useNeonOrSSE;
#else
constexpr inline ExpressionChainBuilder<expressions::AVXFilter> useNeonOrAVX;
constexpr inline ExpressionChainBuilder<expressions::SSEFilter> useNeonOrSSE;
#endif
} // namespace vctr
