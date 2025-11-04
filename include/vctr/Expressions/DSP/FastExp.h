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
        const auto in = src.getAVX (i);

        auto numerator = Expression::AVX::add (in, SIMDConst20.avx);
        numerator = Expression::AVX::mul (numerator, in);
        numerator = Expression::AVX::add (numerator, SIMDConst180.avx);
        numerator = Expression::AVX::mul (numerator, in);
        numerator = Expression::AVX::add (numerator, SIMDConst840.avx);
        numerator = Expression::AVX::mul (numerator, in);
        numerator = Expression::AVX::add (numerator, SIMDConst1680.avx);

        auto denominator = Expression::AVX::add (in, SIMDConstMinus20.avx);
        denominator = Expression::AVX::mul (denominator, in);
        denominator = Expression::AVX::add (denominator, SIMDConst180.avx);
        denominator = Expression::AVX::mul (denominator, in);
        denominator = Expression::AVX::add (denominator, SIMDConstMinus840.avx);
        denominator = Expression::AVX::mul (denominator, in);
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
        const auto in = src.getSSE (i);

        auto numerator = Expression::SSE::add (in, SIMDConst20.sse);
        numerator = Expression::SSE::mul (numerator, in);
        numerator = Expression::SSE::add (numerator, SIMDConst180.sse);
        numerator = Expression::SSE::mul (numerator, in);
        numerator = Expression::SSE::add (numerator, SIMDConst840.sse);
        numerator = Expression::SSE::mul (numerator, in);
        numerator = Expression::SSE::add (numerator, SIMDConst1680.sse);

        auto denominator = Expression::SSE::add (in, SIMDConstMinus20.sse);
        denominator = Expression::SSE::mul (denominator, in);
        denominator = Expression::SSE::add (denominator, SIMDConst180.sse);
        denominator = Expression::SSE::mul (denominator, in);
        denominator = Expression::SSE::add (denominator, SIMDConstMinus840.sse);
        denominator = Expression::SSE::mul (denominator, in);
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
        const auto in = src.getNeon (i);

        auto numerator = Expression::Neon::add (in, SIMDConst20.neon);
        numerator = Expression::Neon::mul (numerator, in);
        numerator = Expression::Neon::add (numerator, SIMDConst180.neon);
        numerator = Expression::Neon::mul (numerator, in);
        numerator = Expression::Neon::add (numerator, SIMDConst840.neon);
        numerator = Expression::Neon::mul (numerator, in);
        numerator = Expression::Neon::add (numerator, SIMDConst1680.neon);

        auto denominator = Expression::Neon::add (in, SIMDConstMinus20.neon);
        denominator = Expression::Neon::mul (denominator, in);
        denominator = Expression::Neon::add (denominator, SIMDConst180.neon);
        denominator = Expression::Neon::mul (denominator, in);
        denominator = Expression::Neon::add (denominator, SIMDConstMinus840.neon);
        denominator = Expression::Neon::mul (denominator, in);
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

namespace detail
{

template <std::floating_point>
struct FastExp2Constants {};

template <>
struct FastExp2Constants<float>
{
    static constexpr int mantissaBits = 23;
    static constexpr float minExpo = -126.0f; // exponent of minimum binary32 normal
    static constexpr float expoBias = 127.0f; // binary32 exponent bias
    static constexpr float a = -0x1.6e7592p+2f;
    static constexpr float b = 0x1.bba764p+4f;
    static constexpr float c = 0x1.35ed00p+2f;
    static constexpr float d = 0x1.f5e546p-2f;
    static constexpr float e = 1 << mantissaBits;
};

template <>
struct FastExp2Constants<double>
{
    static constexpr int mantissaBits = 52;
    static constexpr double minExpo = -1022.0; // exponent of minimum binary64 normal
    static constexpr double expoBias = 1023.0; // binary64 exponent bias
    static constexpr double a = -0x1.6e75d58p+2;
    static constexpr double b = 0x1.bba7414p+4;
    static constexpr double c = 0x1.35eccbap+2;
    static constexpr double d = 0x1.f5e53c2p-2;
    static constexpr double e = 1LL << mantissaBits;
};
}

//==============================================================================
/** Calculates a fast approximation for the exp2 function.

    This implementation is inspired by https://stackoverflow.com/a/65562273. It relies
    on reinterpreting the bytes of a float as integer and manipulating the exponent and
    mantissa part.
 */
template <size_t extent, class SrcType>
requires is::realFloatNumber<ValueType<SrcType>>
class FastExp2 : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    using Constants = detail::FastExp2Constants<value_type>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (FastExp2, src)

    VCTR_FORCEDINLINE value_type operator[] (size_t i) const
    {
        auto p = src[i];

        p = std::max (p, Constants::minExpo);

        auto w = std::floor (p);
        auto z = p - w;

        auto approx = Constants::a + Constants::b / (Constants::c - z) - Constants::d * z;

        auto resi = IntType (Constants::e * (w + Constants::expoBias + approx));


        return bitCast<value_type> (resi);
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void prepareAVXEvaluation() const
    requires (has::prepareAVXEvaluation<SrcType> && Expression::CommonElement::isFloat)
    {
        src.prepareAVXEvaluation();

        minExpo.avx = Expression::AVX::broadcast (Constants::minExpo);
        expoBias.avx = Expression::AVX::broadcast (Constants::expoBias);
        c_a.avx = Expression::AVX::broadcast (Constants::a);
        c_b.avx = Expression::AVX::broadcast (Constants::b);
        c_c.avx = Expression::AVX::broadcast (Constants::c);
        c_d.avx = Expression::AVX::broadcast (Constants::d);
        c_e.avx = Expression::AVX::broadcast (Constants::e);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("fma") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isFloat)
    {
        auto in = src.getAVX (i);

        in = Expression::AVX::max (in, minExpo.avx);

        auto w = Expression::AVX::floor (in);
        auto z = Expression::AVX::sub (in, w);

        auto approx = Expression::AVX::sub (Expression::AVX::add (c_a.avx ,
                                                                  Expression::AVX::div (c_b.avx,
                                                                                        Expression::AVX::sub (c_c.avx, z))),
                                            Expression::AVX::mul (c_d.avx,
                                                                  z)
                                             );

        auto resi = Expression::AVX::mul (c_e.avx, Expression::AVX::add (w, Expression::AVX::add (expoBias.avx, approx)));

        // ConvertToInt requires AVX512 features for double registers, so this implementation is constrained to float
        return AVXRegister<IntType>::reinterpretAsFp (Expression::AVX::convertToInt (resi));
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void prepareSSEEvaluation() const
    requires (has::prepareSSEEvaluation<SrcType> && Expression::CommonElement::isFloat)
    {
        src.prepareSSEEvaluation();

        minExpo.sse = Expression::SSE::broadcast (Constants::minExpo);
        expoBias.sse = Expression::SSE::broadcast (Constants::expoBias);
        c_a.sse = Expression::SSE::broadcast (Constants::a);
        c_b.sse = Expression::SSE::broadcast (Constants::b);
        c_c.sse = Expression::SSE::broadcast (Constants::c);
        c_d.sse = Expression::SSE::broadcast (Constants::d);
        c_e.sse = Expression::SSE::broadcast (Constants::e);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isFloat)
    {
        auto in = src.getSSE (i);

        in = Expression::SSE::max (in, minExpo.sse);

        auto w = Expression::SSE::floor (in);
        auto z = Expression::SSE::sub (in, w);

        auto approx = Expression::SSE::sub (Expression::SSE::add (c_a.sse ,
                                                                  Expression::SSE::div (c_b.sse,
                                                                                        Expression::SSE::sub (c_c.sse, z))),
                                            Expression::SSE::mul (c_d.sse,
                                                                  z)
                                             );

        auto resi = Expression::SSE::mul (c_e.sse, Expression::SSE::add (w, Expression::SSE::add (expoBias.sse, approx)));

        // ConvertToInt requires AVX512 features for double registers, so this implementation is constrained to float
        return SSERegister<IntType>::reinterpretAsFp (Expression::SSE::convertToInt (resi));
    }

    //==============================================================================
    // Neon Implementation
    void prepareNeonEvaluation() const
    requires (archARM && has::prepareNeonEvaluation<SrcType> && Expression::CommonElement::isRealFloat)
    {
        src.prepareNeonEvaluation();

        minExpo.neon = Expression::Neon::broadcast (Constants::minExpo);
        expoBias.neon = Expression::Neon::broadcast (Constants::expoBias);
        c_a.neon = Expression::Neon::broadcast (Constants::a);
        c_b.neon = Expression::Neon::broadcast (Constants::b);
        c_c.neon = Expression::Neon::broadcast (Constants::c);
        c_d.neon = Expression::Neon::broadcast (Constants::d);
        c_e.neon = Expression::Neon::broadcast (Constants::e);
    }

    NeonRegister<value_type> getNeon (size_t i) const
    requires (archARM && has::getNeon<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        auto in = src.getNeon (i);

        in = Expression::Neon::max (in, minExpo.neon);

        auto w = Expression::Neon::floor (in);
        auto z = Expression::Neon::sub (in, w);

        auto approx = Expression::Neon::sub (Expression::Neon::add (c_a.neon ,
                                                                    Expression::Neon::div (c_b.neon,
                                                                                           Expression::Neon::sub (c_c.neon, z))),
                                             Expression::Neon::mul (c_d.neon,
                                                                    z)
                                             );

        auto resi = Expression::Neon::mul (c_e.neon, Expression::Neon::add (w, Expression::Neon::add (expoBias.neon, approx)));

        return NeonRegister<IntType>::reinterpretAsFp (Expression::Neon::convertToInt (resi));
    }

private:
    using IntType = std::conditional_t<std::same_as<float, value_type>, int32_t, int64_t>;

    mutable SIMDRegisterUnion<Expression> minExpo;
    mutable SIMDRegisterUnion<Expression> expoBias;
    mutable SIMDRegisterUnion<Expression> c_e;
    mutable SIMDRegisterUnion<Expression> c_a;
    mutable SIMDRegisterUnion<Expression> c_b;
    mutable SIMDRegisterUnion<Expression> c_c;
    mutable SIMDRegisterUnion<Expression> c_d;
};

} // namespace vctr::expressions

namespace vctr
{

/** A fast approximation of the exp function, using only basic algebraic operations in a continued fraction Padé approximation.

    Limit the input values to -6 < x < 4 for a reasonable approximation.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::FastExp> fastExp;

/** A fast approximation of the exp2 function (e.g. 2 raised to the source elements) exploiting some low level details of the
    binary representation of floating point values.

    Limit the input values to -126 < x < 128 for 32 bit float and -1022.0 < x < 2024 for 64 bit double for a reasonable
    approximation which should not exceed a relative error of 5.04e-5.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::FastExp2> fastExp2;

} // namespace vctr
