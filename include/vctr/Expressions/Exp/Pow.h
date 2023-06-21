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

template <size_t extent, class SrcAType, class SrcBType>
class PowVectors : ExpressionTemplateBase
{
public:
    using CommonType = std::common_type_t<ValueType<SrcAType>, ValueType<SrcBType>>;
    using value_type = std::conditional_t<std::integral<CommonType>, double, CommonType>;

    VCTR_COMMON_BINARY_VEC_VEC_EXPRESSION_MEMBERS (PowVectors, srcBase, srcExp)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
#if VCTR_USE_GCEM
        if constexpr (! is::complexFloatNumber<value_type>)
        {
            if (std::is_constant_evaluated())
                return value_type (gcem::pow (srcBase[i], srcExp[i]));
        }
#endif

        return value_type (std::pow (srcBase[i], srcExp[i]));
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatBinaryVectorOp<SrcAType, SrcBType, CommonType, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        Expression::Accelerate::pow (srcBase.evalNextVectorOpInExpressionChain (dst), srcExp.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexComplexFloatBinaryVectorOp<SrcAType, SrcBType, CommonType, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::pow (srcBase.evalNextVectorOpInExpressionChain (dst), srcExp.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
        return dst;
    }

    //==============================================================================
    // Apple specific SIMD Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires is::suitableForAccelerateSSEOp<value_type, SrcAType, SrcBType>
    {
        return { vpowf (srcBase.getSSE (i).value, srcExp.getSSE (i).value) };
    }

    NeonRegister<value_type> getNeon (size_t i) const
    requires is::suitableForAccelerateNeonOp<value_type, SrcBType, SrcBType>
    {
        return { vpowf (srcBase.getNeon (i).value, srcExp.getNeon (i).value) };
    }
};

template <size_t extent, class SrcType>
class PowSingleExponent : ExpressionTemplateBase
{
public:
    using value_type = std::conditional_t<std::is_integral_v<ValueType<SrcType>>, double, ValueType<SrcType>>;

    VCTR_COMMON_BINARY_SINGLE_VEC_EXPRESSION_MEMBERS (PowSingleExponent, base, exp)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
#if VCTR_USE_GCEM
        if constexpr (! is::complexFloatNumber<value_type>)
        {
            if (std::is_constant_evaluated())
                return value_type (gcem::pow (base[i], exp));
        }
#endif

        return value_type (std::pow (base[i], exp));
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, ValueType<SrcType>>
    {
        Expression::IPP::pow (base.evalNextVectorOpInExpressionChain (dst), exp, dst, sizeToInt (size()));
        return dst;
    }
};

template <size_t extent, class SrcType>
class PowSingleBase : ExpressionTemplateBase
{
public:
    using value_type = std::conditional_t<std::is_integral_v<ValueType<SrcType>>, double, ValueType<SrcType>>;

    VCTR_COMMON_BINARY_SINGLE_VEC_EXPRESSION_MEMBERS (PowSingleBase, exp, base)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
#if VCTR_USE_GCEM
        if constexpr (! is::complexFloatNumber<value_type>)
        {
            if (std::is_constant_evaluated())
                return value_type (gcem::pow (base, exp[i]));
        }
#endif

        return value_type (std::pow (base, exp[i]));
    }
};

template <size_t extent, class SrcType, is::constant ConstantType>
class PowConstantExponent : ExpressionTemplateBase
{
public:
    using value_type = std::conditional_t<std::is_integral_v<ValueType<SrcType>>, double, ValueType<SrcType>>;

    static constexpr value_type exp = ConstantType::value;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (PowConstantExponent, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
#if VCTR_USE_GCEM
        if constexpr (! is::complexFloatNumber<value_type>)
        {
            if (std::is_constant_evaluated())
                return value_type (gcem::pow (src[i], exp));
        }
#endif

        return value_type (std::pow (src[i], exp));
    }
    
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, ValueType<SrcType>>
    {
        Expression::IPP::pow (src.evalNextVectorOpInExpressionChain (dst), exp, dst, sizeToInt (size()));
        return dst;
    }
};

template <size_t extent, class SrcType, is::constant ConstantType>
class PowConstantBase : ExpressionTemplateBase
{
public:
    using value_type = std::conditional_t<std::is_integral_v<ValueType<SrcType>>, double, ValueType<SrcType>>;

    static constexpr value_type base = ConstantType::value;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (PowConstantBase, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
#if VCTR_USE_GCEM
        if constexpr (! is::complexFloatNumber<value_type>)
        {
            if (std::is_constant_evaluated())
                return value_type (gcem::pow (base, src[i]));
        }
#endif

        return value_type (std::pow (base, src[i]));
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, ValueType<SrcType>> ||
             is::suitableForIppRealFloatVectorOp<SrcType, ValueType<SrcType>>
    {
        // Todo: Quick workaround to use this in the context of chained accelerated operations.
        //   This is useful to speed up decibel calculation. We should optimise it – maybe based
        //   on chaining e.g. exp2 (log2 (base) * src)
        const auto s = size();
        const auto* x = src.evalNextVectorOpInExpressionChain (dst);

        for (size_t i = 0; i < s; ++i)
            dst[i] = std::pow (base, x[i]);

        return dst;
    }
};

} // namespace vctr::expressions

namespace vctr
{

/** Returns an expression that raises the elements in bases element-wise to the power of the elements in exponents.

    @ingroup Expressions
 */
    template <is::anyVctrOrExpression SrcBaseType, is::anyVctrOrExpression SrcExpType>
    constexpr auto pow (SrcBaseType&& bases, SrcExpType&& exponents)
{
    assertCommonSize (bases, exponents);
    constexpr auto extent = getCommonExtent<SrcBaseType, SrcExpType>();

    return expressions::PowVectors<extent, SrcBaseType, SrcExpType> (std::forward<SrcBaseType> (bases), std::forward<SrcExpType> (exponents));
}

/** Returns an expression that raises the base value base to the power of the elements in exponents.

    @ingroup Expressions
 */
template <is::anyVctrOrExpression Src>
constexpr auto pow (typename std::remove_cvref_t<Src>::value_type base, Src&& exponents)
{
    return expressions::PowSingleBase<extentOf<Src>, Src> (base, exponents);
}

/** Returns an expression that raises the elements in bases to the power of the exponent value.

    In case you want to raise the elements to the power of 2 or 3, better use the square or cube
    expression.

    @ingroup Expressions

    @see square
    @see cube
 */
template <is::anyVctrOrExpression Src>
constexpr auto pow (Src&& bases, typename std::remove_cvref_t<Src>::value_type exponent)
{
    return expressions::PowSingleExponent<extentOf<Src>, Src> (exponent, bases);
}

/** Evaluates base raised to the power of the source elements.

    @ingroup Expressions
 */
template <auto base>
constexpr inline ExpressionChainBuilder<expressions::PowConstantBase, Constant<base>> powConstantBase;

/** Evaluates the source elements raised to the power of exponent.

    @ingroup Expressions
 */
template <auto exponent>
constexpr inline ExpressionChainBuilder<expressions::PowConstantExponent, Constant<exponent>> powConstantExponent;

} // namespace vctr
