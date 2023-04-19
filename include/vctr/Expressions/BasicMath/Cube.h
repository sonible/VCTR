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
class Cube : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Cube, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        const auto s = src[i];
        return s * s * s;
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires archX64 && has::getAVX<SrcType> && Expression::CommonElement::isRealFloat
    {
        auto x = src.getAVX (i);
        auto y = Expression::AVX::mul (x, x);
        return Expression::AVX::mul (x, y);
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires archX64 && has::getSSE<SrcType> && Expression::CommonElement::isRealFloat
    {
        auto x = src.getSSE (i);
        auto y = Expression::SSE::mul (x, x);
        return Expression::SSE::mul (x, y);
    }

    //==============================================================================
    // Neon Implementation
    NeonRegister<value_type> getNeon (size_t i) const
    requires archARM && has::getNeon<SrcType> && (Expression::CommonElement::isRealFloat || Expression::CommonElement::isInt32 || Expression::CommonElement::isUint32)
    {
        auto x = src.getNeon (i);
        auto y = Expression::Neon::mul (x, x);
        return Expression::Neon::mul (x, y);
    }
};

} // namespace vctr::expressions

namespace vctr
{

/** Raises the source values to the power of three.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::Cube> cube;

} // namespace vctr
