/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2025- by sonible GmbH.

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

namespace detail
{

template <std::floating_point T>
struct FastLog2Constants {};

template <>
struct FastLog2Constants<float>
{
    static constexpr float a              { 1.1920928955078125e-7f };
    static constexpr float b              { 124.22551499f };
    static constexpr float c              { 1.498030302f };
    static constexpr float d              { 1.72587999f };
    static constexpr float e              { 0.3520887068f };
    static constexpr int32_t c_0x007fffff { 0x007fffff }; // Masks the mantissa bits
    static constexpr int32_t c_0x3f000000 { 0x3f000000 };
    // clang-format on
};
}

//==============================================================================
/** Calculates a fast approximation for the log2 function.

    This implementation is inspired by
    https://github.com/romeric/fastapprox/blob/ccc534400ec3e0f67de4eafb53377334962d9db6/fastapprox/src/fastlog.h#L48.
    It relies on reinterpreting the bytes of a float as integer and manipulating the
    exponent and mantissa part.
 */
template <size_t extent, class SrcType>
requires std::same_as<float, ValueType<SrcType>>
class FastLog2 : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    using Constants = detail::FastLog2Constants<value_type>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (FastLog2, src)

    VCTR_FORCEDINLINE value_type operator[] (size_t i) const
    {
        auto x = src[i];

        auto xBitsInterpretedAsInt = bitCast<int32_t> (x);
        auto mantissa = (xBitsInterpretedAsInt & Constants::c_0x007fffff);
        auto mx = bitCast<float> (mantissa | Constants::c_0x3f000000);

        auto y = static_cast<float> (xBitsInterpretedAsInt) * Constants::a;

        auto dv = Constants::d / (Constants::e + mx);
        auto ml = Constants::c * mx;

        return y - Constants::b - ml - dv;
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void prepareAVXEvaluation() const
    requires (has::prepareAVXEvaluation<SrcType> && Expression::CommonElement::isRealFloat)
    {
        src.prepareAVXEvaluation();

        c_a.avx = Expression::AVX::broadcast (Constants::a);
        c_b.avx = Expression::AVX::broadcast (Constants::b);
        c_c.avx = Expression::AVX::broadcast (Constants::c);
        c_d.avx = Expression::AVX::broadcast (Constants::d);
        c_e.avx = Expression::AVX::broadcast (Constants::e);

        c_0x007fffff.avx = IntTypes::AVXSrc::broadcast (Constants::c_0x007fffff);
        c_0x3f000000.avx = IntTypes::AVXSrc::broadcast (Constants::c_0x3f000000);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("fma") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        auto x = src.getAVX (i);

        auto xBitsInterpretedAsInt = Expression::AVX::reinterpretAsInt (x);
        auto mantissa = IntTypes::AVXSrc::bitwiseAndLegacy (xBitsInterpretedAsInt, c_0x007fffff.avx);
        auto mx = IntTypes::AVXSrc::reinterpretAsFp (IntTypes::AVXSrc::bitwiseOrLegacy (mantissa, c_0x3f000000.avx));

        auto y = Expression::AVX::mul (IntTypes::AVXSrc::convertToFp (xBitsInterpretedAsInt), c_a.avx);

        auto dv = Expression::AVX::div (c_d.avx, Expression::AVX::add (c_e.avx, mx));
        auto ml = Expression::AVX::mul (c_c.avx, mx);

        return Expression::AVX::sub (Expression::AVX::sub (Expression::AVX::sub (y, c_b.avx), ml), dv);
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void prepareSSEEvaluation() const
    requires (has::prepareSSEEvaluation<SrcType> && Expression::CommonElement::isRealFloat)
    {
        src.prepareSSEEvaluation();

        c_a.sse = Expression::SSE::broadcast (Constants::a);
        c_b.sse = Expression::SSE::broadcast (Constants::b);
        c_c.sse = Expression::SSE::broadcast (Constants::c);
        c_d.sse = Expression::SSE::broadcast (Constants::d);
        c_e.sse = Expression::SSE::broadcast (Constants::e);

        c_0x007fffff.sse = IntTypes::SSESrc::broadcast (Constants::c_0x007fffff);
        c_0x3f000000.sse = IntTypes::SSESrc::broadcast (Constants::c_0x3f000000);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        auto x = src.getSSE (i);

        auto xBitsInterpretedAsInt = Expression::SSE::reinterpretAsInt (x);
        auto mantissa = IntTypes::SSESrc::bitwiseAnd (xBitsInterpretedAsInt, c_0x007fffff.sse);
        auto mx = IntTypes::SSESrc::reinterpretAsFp (IntTypes::SSESrc::bitwiseOr (mantissa, c_0x3f000000.sse));

        auto y = Expression::SSE::mul (IntTypes::SSESrc::convertToFp (xBitsInterpretedAsInt), c_a.sse);

        auto dv = Expression::SSE::div (c_d.sse, Expression::SSE::add (c_e.sse, mx));
        auto ml = Expression::SSE::mul (c_c.sse, mx);

        return Expression::SSE::sub (Expression::SSE::sub (Expression::SSE::sub (y, c_b.sse), ml), dv);
    }

    //==============================================================================
    // Neon Implementation
    void prepareNeonEvaluation() const
    requires (archARM && has::prepareNeonEvaluation<SrcType> && Expression::CommonElement::isRealFloat)
    {
        src.prepareNeonEvaluation();

        c_a.neon = Expression::Neon::broadcast (Constants::a);
        c_b.neon = Expression::Neon::broadcast (Constants::b);
        c_c.neon = Expression::Neon::broadcast (Constants::c);
        c_d.neon = Expression::Neon::broadcast (Constants::d);
        c_e.neon = Expression::Neon::broadcast (Constants::e);

        c_0x007fffff.neon = IntTypes::NeonSrc::broadcast (Constants::c_0x007fffff);
        c_0x3f000000.neon = IntTypes::NeonSrc::broadcast (Constants::c_0x3f000000);
    }

    NeonRegister<value_type> getNeon (size_t i) const
    requires (archARM && has::getNeon<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        auto x = src.getNeon (i);

        auto xBitsInterpretedAsInt = Expression::Neon::reinterpretAsInt (x);
        auto mantissa = IntTypes::NeonSrc::bitwiseAnd (xBitsInterpretedAsInt, c_0x007fffff.neon);
        auto mx = IntTypes::NeonSrc::reinterpretAsFp (IntTypes::NeonSrc::bitwiseOr (mantissa, c_0x3f000000.neon));

        auto y = Expression::Neon::mul (IntTypes::NeonSrc::convertToFp (xBitsInterpretedAsInt), c_a.neon);

        auto dv = Expression::Neon::div (c_d.neon, Expression::Neon::add (c_e.neon, mx));
        auto ml = Expression::Neon::mul (c_c.neon, mx);

        return Expression::Neon::sub (Expression::Neon::sub (Expression::Neon::sub (y, c_b.neon), ml), dv);
    }

private:
    struct IntTypes
    {
        using NeonSrc = NeonRegister<int32_t>;
        using AVXSrc = AVXRegister<int32_t>;
        using SSESrc = SSERegister<int32_t>;
    };

    mutable SIMDRegisterUnion<Expression> c_a;
    mutable SIMDRegisterUnion<Expression> c_b;
    mutable SIMDRegisterUnion<Expression> c_c;
    mutable SIMDRegisterUnion<Expression> c_d;
    mutable SIMDRegisterUnion<Expression> c_e;
    mutable SIMDRegisterUnion<IntTypes> c_0x007fffff;
    mutable SIMDRegisterUnion<IntTypes> c_0x3f000000;
};

} // namespace vctr::expressions

namespace vctr
{

/** A fast approximation of the log2 function (e.g. 2 raised to the source elements) exploiting some low level details of the
    binary representation of floating point values.

    Limit the input values to -126 < x < 128 for 32 bit float and -1022.0 < x < 2024 for 64 bit double for a reasonable
    approximation which should not exceed a relative error of 5.04e-5.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::FastLog2> fastLog2;

} // namespace vctr
