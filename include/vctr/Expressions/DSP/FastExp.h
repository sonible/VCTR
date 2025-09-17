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

//==============================================================================
/** Calculates a fast approximation for the exp function. */
template <size_t extent, class SrcType>
requires is::realOrComplexFloatNumber<ValueType<SrcType>>
class FastExp : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (FastExp, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return (Const1680 + src[i] * (Const840 + src[i] * (Const180 + src[i] * (Const20 + src[i])))) / (Const1680 + src[i] * (ConstMinus840 + src[i] * (Const180 + src[i] * (ConstMinus20 + src[i]))));
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void prepareAVXEvaluation() const
    requires (has::prepareAVXEvaluation<SrcType> && Expression::CommonElement::isRealFloat)
    {
        src.prepareAVXEvaluation();

        SIMDConst20.avx = Expression::AVX::broadcast (Const20);
        SIMDConst180.avx = Expression::AVX::broadcast (Const180);
        SIMDConst840.avx = Expression::AVX::broadcast (Const840);
        SIMDConst1680.avx = Expression::AVX::broadcast (Const1680);
        SIMDConstMinus20.avx = Expression::AVX::broadcast (ConstMinus20);
        SIMDConstMinus840.avx = Expression::AVX::broadcast (ConstMinus840);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("fma") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        auto numerator = Expression::AVX::add (src.getAVX (i), SIMDConst20.avx);
        numerator = Expression::AVX::mul (numerator, src.getAVX (i));
        numerator = Expression::AVX::add (numerator, SIMDConst180.avx);
        numerator = Expression::AVX::mul (numerator, src.getAVX (i));
        numerator = Expression::AVX::add (numerator, SIMDConst840.avx);
        numerator = Expression::AVX::mul (numerator, src.getAVX (i));
        numerator = Expression::AVX::add (numerator, SIMDConst1680.avx);

        auto denominator = Expression::AVX::add (src.getAVX (i), SIMDConstMinus20.avx);
        denominator = Expression::AVX::mul (denominator, src.getAVX (i));
        denominator = Expression::AVX::add (denominator, SIMDConst180.avx);
        denominator = Expression::AVX::mul (denominator, src.getAVX (i));
        denominator = Expression::AVX::add (denominator, SIMDConstMinus840.avx);
        denominator = Expression::AVX::mul (denominator, src.getAVX (i));
        denominator = Expression::AVX::add (denominator, SIMDConst1680.avx);

        return Expression::AVX::div (numerator, denominator);
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void prepareSSEEvaluation() const
    requires (has::prepareSSEEvaluation<SrcType> && Expression::CommonElement::isRealFloat)
    {
        src.prepareSSEEvaluation();

        SIMDConst20.sse = Expression::SSE::broadcast (Const20);
        SIMDConst180.sse = Expression::SSE::broadcast (Const180);
        SIMDConst840.sse = Expression::SSE::broadcast (Const840);
        SIMDConst1680.sse = Expression::SSE::broadcast (Const1680);
        SIMDConstMinus20.sse = Expression::SSE::broadcast (ConstMinus20);
        SIMDConstMinus840.sse = Expression::SSE::broadcast (ConstMinus840);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        auto numerator = Expression::SSE::add (src.getSSE (i), SIMDConst20.sse);
        numerator = Expression::SSE::mul (numerator, src.getSSE (i));
        numerator = Expression::SSE::add (numerator, SIMDConst180.sse);
        numerator = Expression::SSE::mul (numerator, src.getSSE (i));
        numerator = Expression::SSE::add (numerator, SIMDConst840.sse);
        numerator = Expression::SSE::mul (numerator, src.getSSE (i));
        numerator = Expression::SSE::add (numerator, SIMDConst1680.sse);

        auto denominator = Expression::SSE::add (src.getSSE (i), SIMDConstMinus20.sse);
        denominator = Expression::SSE::mul (denominator, src.getSSE (i));
        denominator = Expression::SSE::add (denominator, SIMDConst180.sse);
        denominator = Expression::SSE::mul (denominator, src.getSSE (i));
        denominator = Expression::SSE::add (denominator, SIMDConstMinus840.sse);
        denominator = Expression::SSE::mul (denominator, src.getSSE (i));
        denominator = Expression::SSE::add (denominator, SIMDConst1680.sse);

        return Expression::SSE::div (numerator, denominator);
    }

    //==============================================================================
    // Neon Implementation
    void prepareNeonEvaluation() const
    requires (archARM && has::prepareNeonEvaluation<SrcType> && Expression::CommonElement::isRealFloat)
    {
        src.prepareNeonEvaluation();

        SIMDConst20.neon = Expression::Neon::broadcast (Const20);
        SIMDConst180.neon = Expression::Neon::broadcast (Const180);
        SIMDConst840.neon = Expression::Neon::broadcast (Const840);
        SIMDConst1680.neon = Expression::Neon::broadcast (Const1680);
        SIMDConstMinus20.neon = Expression::Neon::broadcast (ConstMinus20);
        SIMDConstMinus840.neon = Expression::Neon::broadcast (ConstMinus840);
    }

    NeonRegister<value_type> getNeon (size_t i) const
    requires (archARM && has::getNeon<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        auto numerator = Expression::Neon::add (src.getNeon (i), SIMDConst20.neon);
        numerator = Expression::Neon::mul (numerator, src.getNeon (i));
        numerator = Expression::Neon::add (numerator, SIMDConst180.neon);
        numerator = Expression::Neon::mul (numerator, src.getNeon (i));
        numerator = Expression::Neon::add (numerator, SIMDConst840.neon);
        numerator = Expression::Neon::mul (numerator, src.getNeon (i));
        numerator = Expression::Neon::add (numerator, SIMDConst1680.neon);

        auto denominator = Expression::Neon::add (src.getNeon (i), SIMDConstMinus20.neon);
        denominator = Expression::Neon::mul (denominator, src.getNeon (i));
        denominator = Expression::Neon::add (denominator, SIMDConst180.neon);
        denominator = Expression::Neon::mul (denominator, src.getNeon (i));
        denominator = Expression::Neon::add (denominator, SIMDConstMinus840.neon);
        denominator = Expression::Neon::mul (denominator, src.getNeon (i));
        denominator = Expression::Neon::add (denominator, SIMDConst1680.neon);

        return Expression::Neon::div (numerator, denominator);
    }

private:
    static constexpr value_type Const20 = value_type (20);
    static constexpr value_type Const180 = value_type (180);
    static constexpr value_type Const840 = value_type (840);
    static constexpr value_type Const1680 = value_type (1680);
    static constexpr value_type ConstMinus20 = value_type (-20);
    static constexpr value_type ConstMinus840 = value_type (-840);

    mutable SIMDRegisterUnion<Expression> SIMDConst20 {};
    mutable SIMDRegisterUnion<Expression> SIMDConst180 {};
    mutable SIMDRegisterUnion<Expression> SIMDConst840 {};
    mutable SIMDRegisterUnion<Expression> SIMDConst1680 {};
    mutable SIMDRegisterUnion<Expression> SIMDConstMinus20 {};
    mutable SIMDRegisterUnion<Expression> SIMDConstMinus840 {};
};

} // namespace vctr::expressions

namespace vctr
{

/** A fast approximation of the exp function, using only basic algebraic operations in a continued fraction Padé approximation.

    Limit the input values to -6 < x < 4 for a reasonable approximation.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::FastExp> fastExp;

} // namespace vctr
