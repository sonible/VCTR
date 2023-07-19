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
/** Multiplies two vector like types. */
template <size_t extent, class SrcAType, class SrcBType>
class MultiplyVectors : ExpressionTemplateBase
{
public:
    using value_type = std::common_type_t<ValueType<SrcAType>, ValueType<SrcBType>>;

    VCTR_COMMON_BINARY_VEC_VEC_EXPRESSION_MEMBERS (MultiplyVectors, srcA, srcB)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return srcA[i] * srcB[i];
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealOrComplexComplexFloatBinaryVectorOp<SrcAType, SrcBType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        Expression::Accelerate::mul (srcA.evalNextVectorOpInExpressionChain (dst), srcB.evalNextVectorOpInExpressionChain (dst), dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexComplexFloatBinaryVectorOp<SrcAType, SrcBType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::mul (srcA.evalNextVectorOpInExpressionChain (dst), srcB.evalNextVectorOpInExpressionChain (dst), dst, size());
        return dst;
    }


    VCTR_FORCEDINLINE void evalVectorOpMultiplyAccumulate (value_type* dst) const
    requires is::suitableForAccelerateRealFloatBinaryVectorOp<SrcAType, SrcBType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable> && is::anyVctr<SrcAType> && is::anyVctr<SrcBType>
    {
        Expression::Accelerate::multiplyAdd (srcA.data(), srcB.data(), dst, dst, size());
    }

    VCTR_FORCEDINLINE void evalVectorOpMultiplyAccumulate (value_type* dst) const
    requires is::suitableForIppRealOrComplexComplexFloatBinaryVectorOp<SrcAType, SrcBType, value_type, detail::preferIfIppAndAccelerateAreAvailable> && is::anyVctr<SrcAType> && is::anyVctr<SrcBType>
    {
        Expression::IPP::multiplyAccumulate (srcA.data(), srcB.data(), dst, sizeToInt (size()));
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcAType> && has::getAVX<SrcBType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        return Expression::AVX::mul (srcA.getAVX (i), srcB.getAVX (i));
    }
};

//==============================================================================
/** Multiplies a vector like type by a single value. */
template <size_t extent, class SrcType>
class MultiplyVecBySingle : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_BINARY_SINGLE_VEC_EXPRESSION_MEMBERS (MultiplyVecBySingle, src, single)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return single * src[i];
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        Expression::Accelerate::mul (src.evalNextVectorOpInExpressionChain (dst), single, dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::mul (src.evalNextVectorOpInExpressionChain (dst), single, dst, sizeToInt (size()));
        return dst;
    }

    VCTR_FORCEDINLINE void evalVectorOpMultiplyAccumulate (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable> && is::anyVctr<SrcType>
    {
        Expression::Accelerate::multiplyAdd (src.data(), single, dst, dst, size());
    }

    VCTR_FORCEDINLINE void evalVectorOpMultiplyAccumulate (value_type* dst) const
    requires is::suitableForIppRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable> && is::anyVctr<SrcType>
    {
        Expression::IPP::multiplyAccumulate (src.data(), single, dst, sizeToInt (size()));
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        return Expression::AVX::mul (Expression::AVX::fromSSE (singleAsSSE, singleAsSSE), src.getAVX (i));
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        return Expression::SSE::mul (singleAsSSE, src.getSSE (i));
    }
};

//==============================================================================
/** Multiplies a vector like type by a single compile time constant value. */
template <size_t extent, class SrcType, is::constant ConstantType>
class MultiplyVecByConstant : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    static constexpr auto constant = value_type (ConstantType::value);

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (MultiplyVecByConstant, src)

    void applyRuntimeArgs()
    {
        asSSE = Expression::SSESrc::broadcast (constant);
        asNeon = Expression::NeonSrc::broadcast (constant);
    }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return constant * src[i];
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        Expression::Accelerate::mul (src.evalNextVectorOpInExpressionChain (dst), constant, dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::mul (src.evalNextVectorOpInExpressionChain (dst), constant, dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE void evalVectorOpMultiplyAccumulate (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable> && is::anyVctr<SrcType>
    {
        Expression::Accelerate::multiplyAdd (src.data(), constant, dst, dst, size());
    }

    VCTR_FORCEDINLINE void evalVectorOpMultiplyAccumulate (value_type* dst) const
    requires is::suitableForIppRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable> && is::anyVctr<SrcType>
    {
        Expression::IPP::multiplyAccumulate (src.data(), constant, dst, sizeToInt (size()));
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        return Expression::AVX::mul (Expression::AVX::fromSSE (asSSE, asSSE), src.getAVX (i));
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        return Expression::SSE::mul (asSSE, src.getSSE (i));
    }

private:

    typename Expression::SSESrc asSSE;
    typename Expression::NeonSrc asNeon;
};

} // namespace vctr::expressions

namespace vctr::detail
{
template <class T>
struct IsMultiplicationExpression : std::false_type {};

template <size_t e, class A, class B>
struct IsMultiplicationExpression<expressions::MultiplyVectors<e, A, B>> : std::true_type {};

template <size_t e, class A>
struct IsMultiplicationExpression<expressions::MultiplyVecBySingle<e, A>> : std::true_type {};
} // namespace vctr::detail

namespace vctr
{

/** Returns an expression that multiplies vector or expression a with vector or expression b.

    @ingroup Expressions
 */
template <class SrcAType, class SrcBType>
requires (is::anyVctrOrExpression<std::remove_cvref_t<SrcAType>> &&
          is::anyVctrOrExpression<std::remove_cvref_t<SrcBType>>)
constexpr auto operator* (SrcAType&& a, SrcBType&& b)
{
    assertCommonSize (a, b);
    constexpr auto extent = getCommonExtent<SrcAType, SrcBType>();

    return expressions::MultiplyVectors<extent, SrcAType, SrcBType> (std::forward<SrcAType> (a), std::forward<SrcBType> (b));
}

/** Returns an expression that multiplies a single value with a vector or expression source.

    @ingroup Expressions
 */
template <class Src>
requires is::anyVctrOrExpression<Src>
constexpr auto operator* (typename std::remove_cvref_t<Src>::value_type single, Src&& vec)
{
    return expressions::MultiplyVecBySingle<extentOf<Src>, Src> (single, std::forward<Src> (vec));
}

/** Returns an expression that multiplies a vector or expression source with a single value.

    @ingroup Expressions
 */
template <class Src>
requires is::anyVctrOrExpression<Src>
constexpr auto operator* (Src&& vec, typename std::remove_cvref_t<Src>::value_type single)
{
    return expressions::MultiplyVecBySingle<extentOf<Src>, Src> (single, std::forward<Src> (vec));
}


/** Returns an expression that multiplies a vector or expression source with a compile time constant.

    @ingroup Expressions
 */
template <auto constantValue>
constexpr inline ExpressionChainBuilder<expressions::MultiplyVecByConstant, Constant<constantValue>> multiplyByConstant;
} // namespace vctr
