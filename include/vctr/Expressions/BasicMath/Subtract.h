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
/** Subtracts two vector like types. */
template <size_t extent, class SrcAType, class SrcBType>
class SubtractVectors : ExpressionTemplateBase
{
public:
    using value_type = std::common_type_t<ValueType<SrcAType>, ValueType<SrcBType>>;

    VCTR_COMMON_BINARY_VEC_VEC_EXPRESSION_MEMBERS (SubtractVectors, srcA, srcB)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return srcA[i] - srcB[i];
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatBinaryVectorOp<SrcAType, SrcBType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        Expression::Accelerate::sub (srcA.evalNextVectorOpInExpressionChain (dst), srcB.evalNextVectorOpInExpressionChain (dst), dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexComplexFloatBinaryVectorOp<SrcAType, SrcBType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::sub (srcA.evalNextVectorOpInExpressionChain (dst), srcB.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
        return dst;
    }

    //==============================================================================
    VCTR_FORWARD_PREPARE_SIMD_EVALUATION_BINARY_EXPRESSION_MEMBER_FUNCTIONS (srcA, srcB)

    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcAType> && has::getAVX<SrcBType> && Expression::CommonElement::isRealFloat)
    {
        return Expression::AVX::sub (srcA.getAVX (i), srcB.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcAType> && has::getAVX<SrcBType> && Expression::CommonElement::isUnsigned)
    {
        return Expression::AVX::sub (srcA.getAVX (i), srcB.getAVX (i));
    }

    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcAType> && has::getSSE<SrcBType>)
    {
        return Expression::SSE::sub (srcA.getSSE (i), srcB.getSSE (i));
    }

    // Neon Implementation
    NeonRegister<value_type> getNeon (size_t i) const
    requires (archARM && has::getNeon<SrcAType> && has::getNeon<SrcBType>)
    {
        return Expression::Neon::sub (srcA.getNeon (i), srcB.getNeon (i));
    }
};

//==============================================================================
/** Subtracts a vector like type from a single value. */
template <size_t extent, class SrcType>
class SubtractVecFromSingle : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_BINARY_SINGLE_VEC_EXPRESSION_MEMBERS (SubtractVecFromSingle, src, single)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return single - src[i];
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        auto fac = value_type (-1);
        Expression::Accelerate::multiplyAdd (src.evalNextVectorOpInExpressionChain (dst), fac, single, dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::sub (single, src.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
        return dst;
    }

    //==============================================================================
    // AVX Implementation
    void prepareAVXEvaluation() const
    requires has::prepareAVXEvaluation<SrcType>
    {
        src.prepareAVXEvaluation();
        singleSIMD.avx = Expression::AVX::broadcast (single);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        return Expression::AVX::sub (singleSIMD.avx, src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isInt)
    {
        return Expression::AVX::sub (singleSIMD.avx, src.getAVX (i));
    }

    // SSE Implementation
    void prepareSSEEvaluation() const
    requires has::prepareSSEEvaluation<SrcType>
    {
        src.prepareSSEEvaluation();
        singleSIMD.sse = Expression::SSE::broadcast (single);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame)
    {
        return Expression::SSE::sub (singleSIMD.sse, src.getSSE (i));
    }

    // Neon Implementation
    void prepareNeonEvaluation() const
    requires has::prepareNeonEvaluation<SrcType>
    {
        src.prepareNeonEvaluation();
        singleSIMD.neon = Expression::Neon::broadcast (single);
    }

    NeonRegister<value_type> getNeon (size_t i) const
    requires (archARM && has::getNeon<SrcType> && Expression::allElementTypesSame)
    {
        return Expression::Neon::sub (singleSIMD.neon, src.getNeon (i));
    }

private:
    mutable SIMDRegisterUnion<Expression> singleSIMD {};
};

//==============================================================================
/** Subtracts a single value from a vector like type. */
template <size_t extent, class SrcType>
class SubtractSingleFromVec : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_BINARY_SINGLE_VEC_EXPRESSION_MEMBERS (SubtractSingleFromVec, src, single)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return src[i] - single;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealOrComplexFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        Expression::Accelerate::add (src.evalNextVectorOpInExpressionChain (dst), -single, dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::sub (src.evalNextVectorOpInExpressionChain (dst), single, dst, sizeToInt (size()));
        return dst;
    }

    //==============================================================================
    void prepareAVXEvaluation() const
    requires has::prepareAVXEvaluation<SrcType>
    {
        src.prepareAVXEvaluation();
        singleSIMD.avx = Expression::AVX::broadcast (single);
    }

    void prepareSSEEvaluation() const
    requires has::prepareSSEEvaluation<SrcType>
    {
        src.prepareSSEEvaluation();
        singleSIMD.sse = Expression::SSE::broadcast (single);
    }

    void prepareNeonEvaluation() const
    requires has::prepareNeonEvaluation<SrcType>
    {
        src.prepareNeonEvaluation();
        singleSIMD.neon = Expression::Neon::broadcast (single);
    }

    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        return Expression::AVX::sub (src.getAVX (i), singleSIMD.avx);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isInt)
    {
        return Expression::AVX::sub (src.getAVX (i), singleSIMD.avx);
    }

    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame)
    {
        return Expression::SSE::sub (src.getSSE (i), singleSIMD.sse);
    }

    // Neon Implementation
    NeonRegister<value_type> getNeon (size_t i) const
    requires (archARM && has::getNeon<SrcType> && Expression::allElementTypesSame)
    {
        return Expression::Neon::sub (src.getNeon (i), singleSIMD.neon);
    }

private:
    mutable SIMDRegisterUnion<Expression> singleSIMD {};
};

} // namespace vctr::expressions

namespace vctr
{

/** Returns an expression that subtracts vector or expression b from vector or expression a.

    @ingroup Expressions
 */
template <class SrcAType, class SrcBType>
requires (is::anyVctrOrExpression<std::remove_cvref_t<SrcAType>> &&
          is::anyVctrOrExpression<std::remove_cvref_t<SrcBType>>)
constexpr auto operator- (SrcAType&& a, SrcBType&& b)
{
    assertCommonSize (a, b);
    constexpr auto extent = getCommonExtent<SrcAType, SrcBType>();

    return expressions::SubtractVectors<extent, SrcAType, SrcBType> (std::forward<SrcAType> (a), std::forward<SrcBType> (b));
}

/** Returns an expression that subtracts a vector or expression source from a single value.

    @ingroup Expressions
 */
template <class Src>
requires is::anyVctrOrExpression<Src>
constexpr auto operator- (typename std::remove_cvref_t<Src>::value_type single, Src&& vec)
{
    return expressions::SubtractVecFromSingle<extentOf<Src>, Src> (single, std::forward<Src> (vec));
}

/** Returns an expression that subtracts a single value from a vector or expression.

    @ingroup Expressions
 */
template <class Src>
requires is::anyVctrOrExpression<Src>
constexpr auto operator- (Src&& vec, typename std::remove_cvref_t<Src>::value_type single)
{
    return expressions::SubtractSingleFromVec<extentOf<Src>, Src> (single, std::forward<Src> (vec));
}

} // namespace vctr
