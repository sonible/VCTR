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
/** Adds two vector like types */
template <size_t extent, class SrcAType, class SrcBType>
class AddVectors : ExpressionTemplateBase
{
public:
    using value_type = std::common_type_t<ValueType<SrcAType>, ValueType<SrcBType>>;

    VCTR_COMMON_BINARY_VEC_VEC_EXPRESSION_MEMBERS (AddVectors, srcA, srcB)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return srcA[i] + srcB[i];
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealOrComplexComplexFloatBinaryVectorOp<SrcAType, SrcBType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        Expression::Accelerate::add (srcA.evalNextVectorOpInExpressionChain (dst), srcB.evalNextVectorOpInExpressionChain (dst), dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexComplexFloatBinaryVectorOp<SrcAType, SrcBType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::add (srcA.evalNextVectorOpInExpressionChain (dst), srcB.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
        return dst;
    }

    //==============================================================================
    VCTR_FORWARD_PREPARE_SIMD_EVALUATION_BINARY_EXPRESSION_MEMBER_FUNCTIONS (srcA, srcB)

    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcAType> && has::getAVX<SrcBType> && Expression::CommonElement::isRealFloat)
    {
        return Expression::AVX::add (srcA.getAVX (i), srcB.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcAType> && has::getAVX<SrcBType> && Expression::CommonElement::isUnsigned)
    {
        return Expression::AVX::add (srcA.getAVX (i), srcB.getAVX (i));
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcAType> && has::getSSE<SrcBType>)
    {
        return Expression::SSE::add (srcA.getSSE (i), srcB.getSSE (i));
    }
};

//==============================================================================
/** Adds a single value to a vector like type */
template <size_t extent, class SrcType>
class AddSingleToVec : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_BINARY_SINGLE_VEC_EXPRESSION_MEMBERS (AddSingleToVec, src, single)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return single + src[i];
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealOrComplexFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        Expression::Accelerate::add (src.evalNextVectorOpInExpressionChain (dst), single, dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::add (src.evalNextVectorOpInExpressionChain (dst), single, dst, sizeToInt (size()));
        return dst;
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE  VCTR_TARGET ("avx") void prepareAVXEvaluation() const
    requires has::prepareAVXEvaluation<SrcType> && Expression::CommonElement::isRealFloat
    {
        src.prepareAVXEvaluation();
        singleSIMD.avx = Expression::AVX::broadcast (single);
    }

    VCTR_FORCEDINLINE  VCTR_TARGET ("avx2") void prepareAVXEvaluation() const
    requires has::prepareAVXEvaluation<SrcType> && Expression::CommonElement::isInt
    {
        src.prepareAVXEvaluation();
        singleSIMD.avx = Expression::AVX::broadcast (single);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        return Expression::AVX::add (singleSIMD.avx, src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isInt)
    {
        return Expression::AVX::add (singleSIMD.avx, src.getAVX (i));
    }

    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void prepareSSEEvaluation() const
    requires has::prepareSSEEvaluation<SrcType>
    {
        src.prepareSSEEvaluation();
        singleSIMD.sse = Expression::SSE::broadcast (single);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame)
    {
        return Expression::SSE::add (singleSIMD.sse, src.getSSE (i));
    }

private:
    mutable SIMDRegisterUnion<Expression> singleSIMD {};
};

} // namespace vctr::expressions

namespace vctr
{

/** Returns an expression that adds two vector or expression sources.

    @ingroup Expressions
 */
template <is::anyVctrOrExpression SrcAType, is::anyVctrOrExpression SrcBType>
constexpr auto operator+ (SrcAType&& a, SrcBType&& b)
{
    assertCommonSize (a, b);
    constexpr auto extent = getCommonExtent<SrcAType, SrcBType>();

    return expressions::AddVectors<extent, SrcAType, SrcBType> (std::forward<SrcAType> (a), std::forward<SrcBType> (b));
}

/** Returns an expression that adds a single value to a vector or expression source.

    @ingroup Expressions
 */
template <is::anyVctrOrExpression Src>
constexpr auto operator+ (typename std::remove_cvref_t<Src>::value_type single, Src&& vec)
{
    return expressions::AddSingleToVec<extentOf<Src>, Src> (single, std::forward<Src> (vec));
}

/** Returns an expression that adds a vector or expression source to a single value.

    @ingroup Expressions
 */
template <is::anyVctrOrExpression Src>
constexpr auto operator+ (Src&& vec, typename std::remove_cvref_t<Src>::value_type single)
{
    return expressions::AddSingleToVec<extentOf<Src>, Src> (single, std::forward<Src> (vec));
}

} // namespace vctr
