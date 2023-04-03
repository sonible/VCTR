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

//==============================================================================
/** Divides two vector like types */
template <size_t extent, class SrcAType, class SrcBType>
class DivideVectors : ExpressionTemplateBase
{
public:
    using value_type = std::common_type_t<ValueType<SrcAType>, ValueType<SrcBType>>;

    VCTR_COMMON_BINARY_VEC_VEC_EXPRESSION_MEMBERS (DivideVectors, srcA, srcB)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return srcA[i] / srcB[i];
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealOrComplexComplexFloatBinaryVectorOp<SrcAType, SrcBType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        Expression::Accelerate::div (srcA.evalNextVectorOpInExpressionChain (dst), srcB.evalNextVectorOpInExpressionChain (dst), dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexComplexFloatBinaryVectorOp<SrcAType, SrcBType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::div (srcA.evalNextVectorOpInExpressionChain (dst), srcB.evalNextVectorOpInExpressionChain (dst), dst, size());
        return dst;
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcAType> && has::getAVX<SrcBType> && Expression::CommonElement::isFloatingPoint)
    {
        return Expression::AVX::div (srcA.getAVX (i), srcB.getAVX (i));
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcAType> && has::getSSE<SrcBType> && Expression::CommonElement::isFloatingPoint)
    {
        return Expression::SSE::div (srcA.getSSE (i), srcB.getSSE (i));
    }
};

//==============================================================================
/** Divides a single value by a vector */
template <size_t extent, class SrcType>
class DivideSingleByVec : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_BINARY_SINGLE_VEC_EXPRESSION_MEMBERS (DivideSingleByVec, src, single)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return single / src[i];
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealFloatVectorOp<SrcType, value_type> && std::same_as<float, value_type>
    {
        Expression::IPP::div (single, src.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
        return dst;
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isFloatingPoint)
    {
        return Expression::AVX::div (Expression::AVX::fromSSE (singleAsSSE, singleAsSSE), src.getAVX (i));
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isFloatingPoint)
    {
        return Expression::SSE::div (singleAsSSE, src.getSSE (i));
    }
};

//==============================================================================
/** Divides a vector like type by a single value */
template <size_t extent, class SrcType>
class DivideVecBySingle : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_BINARY_SINGLE_VEC_EXPRESSION_MEMBERS (DivideVecBySingle, src, single)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return src[i] / single;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        Expression::Accelerate::div (src.evalNextVectorOpInExpressionChain (dst), single, dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::div (src.evalNextVectorOpInExpressionChain (dst), single, dst, size());
        return dst;
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isFloatingPoint)
    {
        return Expression::AVX::div (src.getAVX (i), Expression::AVX::fromSSE (singleAsSSE, singleAsSSE));
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isFloatingPoint)
    {
        return Expression::SSE::div (src.getSSE (i), singleAsSSE);
    }
};

} // namespace vctr::Expressions

namespace vctr
{

/** Returns an expression that divides vector or expression a by vector or expression b.

    @ingroup Expressions
 */
template <class SrcAType, class SrcBType>
requires (is::anyVctrOrExpression<std::remove_cvref_t<SrcAType>> &&
          is::anyVctrOrExpression<std::remove_cvref_t<SrcBType>>)
constexpr auto operator/ (SrcAType&& a, SrcBType&& b)
{
    assertCommonSize (a, b);
    constexpr auto extent = getCommonExtent<SrcAType, SrcBType>();

    return Expressions::DivideVectors<extent, SrcAType, SrcBType> (std::forward<SrcAType> (a), std::forward<SrcBType> (b));
}

/** Returns an expression that divides a single value by a vector or expression source.

    @ingroup Expressions
 */
template <class Src>
requires is::anyVctrOrExpression<Src>
constexpr auto operator/ (typename std::remove_cvref_t<Src>::value_type single, Src&& vec)
{
    return Expressions::DivideSingleByVec<extentOf<Src>, Src> (single, std::forward<Src> (vec));
}

/** Returns an expression that divides a vector or expression source by a single value.

    @ingroup Expressions
 */
template <class Src>
requires is::anyVctrOrExpression<Src>
constexpr auto operator/ (Src&& vec, typename std::remove_cvref_t<Src>::value_type single)
{
    return Expressions::DivideVecBySingle<extentOf<Src>, Src> (single, std::forward<Src> (vec));
}

} // namespace vctr
