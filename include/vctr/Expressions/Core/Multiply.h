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
/** Multiplies two vector like types */
template <size_t extent, class SrcAType, class SrcBType>
class MultiplyVectors : ExpressionTemplateBase
{
public:
    using value_type = std::common_type_t<typename std::remove_cvref_t<SrcAType>::value_type, typename std::remove_cvref_t<SrcBType>::value_type>;

    using Expression = ExpressionTypes<value_type, SrcAType, SrcBType>;

    template <class SrcA, class SrcB>
    constexpr MultiplyVectors (SrcA&& a, SrcB&& b)
        : srcA (std::forward<SrcA> (a)),
          srcB (std::forward<SrcB> (b)),
          storageInfo (srcA.getStorageInfo(), srcB.getStorageInfo())
    {}

    constexpr const auto& getStorageInfo() const { return storageInfo; }

    constexpr size_t size() const { return srcA.size(); }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return srcA[i] * srcB[i];
    }

    constexpr bool isNotAliased (const void* dst) const
    {
        if constexpr (is::expression<SrcAType> && is::anyVctr<SrcBType>)
        {
            return dst != srcB.data();
        }

        if constexpr (is::anyVctr<SrcAType> && is::expression<SrcBType>)
        {
            return dst != srcA.data();
        }

        return true;
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

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcAType> && has::getAVX<SrcBType> && Expression::allElementTypesSame && Expression::CommonElement::isFloatingPoint)
    {
        return Expression::AVX::mul (srcA.getAVX (i), srcB.getAVX (i));
    }

private:
    SrcAType srcA;
    SrcBType srcB;

    using SrcAStorageInfoType = std::remove_cvref_t<std::invoke_result_t<decltype (&std::remove_cvref_t<SrcAType>::getStorageInfo), SrcAType>>;
    using SrcBStorageInfoType = std::remove_cvref_t<std::invoke_result_t<decltype (&std::remove_cvref_t<SrcBType>::getStorageInfo), SrcBType>>;

    const CombinedStorageInfo<SrcAStorageInfoType, SrcBStorageInfoType> storageInfo;
};

//==============================================================================
/** Multiplies a vector like type by a single value */
template <size_t extent, class SrcType>
class MultiplyVecBySingle : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    using Expression = ExpressionTypes<value_type, SrcType>;

    template <class Src>
    constexpr MultiplyVecBySingle (typename Expression::CommonSrcElement::Type a, Src&& b)
        : src (std::forward<Src> (b)),
          single (a),
          asSSE (Expression::SSESrc::broadcast (a)),
          asNeon (Expression::NeonSrc::broadcast (a))
    {}

    constexpr const auto& getStorageInfo() const { return src.getStorageInfo(); }

    constexpr size_t size() const { return src.size(); }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return single * src[i];
    }

    constexpr bool isNotAliased (const void* other) const
    {
        return src.isNotAliased (other);
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
        Expression::IPP::mul (src.evalNextVectorOpInExpressionChain (dst), single, dst, size());
        return dst;
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isFloatingPoint)
    {
        return Expression::AVX::mul (Expression::AVX::fromSSE (asSSE, asSSE), src.getAVX (i));
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isFloatingPoint)
    {
        return Expression::SSE::mul (asSSE, src.getSSE (i));
    }

private:
    SrcType src;

    const typename Expression::CommonSrcElement::Type single;
    const typename Expression::SSESrc asSSE;
    const typename Expression::NeonSrc asNeon;
};

//==============================================================================
/** Multiplies a vector like type by a single compile time constant value. */
template <size_t extent, class SrcType, is::constantWrapper ConstantType>
class MultiplyVecByConstant : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    using Expression = ExpressionTypes<value_type, SrcType>;

    static constexpr value_type constant = ConstantType::value;

    template <class Src>
    constexpr MultiplyVecByConstant (Src&& b)
        : src (std::forward<Src> (b)),
          asSSE (Expression::SSESrc::broadcast (constant)),
          asNeon (Expression::NeonSrc::broadcast (constant))
    {}

    constexpr const auto& getStorageInfo() const { return src.getStorageInfo(); }

    constexpr size_t size() const { return src.size(); }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return constant * src[i];
    }

    constexpr bool isNotAliased (const void* other) const
    {
        return src.isNotAliased (other);
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

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isFloatingPoint)
    {
        return Expression::AVX::mul (Expression::AVX::fromSSE (asSSE, asSSE), src.getAVX (i));
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isFloatingPoint)
    {
        return Expression::SSE::mul (asSSE, src.getSSE (i));
    }

private:
    SrcType src;

    const typename Expression::SSESrc asSSE;
    const typename Expression::NeonSrc asNeon;
};

} // namespace vctr::Expressions

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

    return Expressions::MultiplyVectors<extent, SrcAType, SrcBType> (std::forward<SrcAType> (a), std::forward<SrcBType> (b));
}

/** Returns an expression that multiplies a single value with a vector or expression source.

    @ingroup Expressions
 */
template <class Src>
requires is::anyVctrOrExpression<Src>
constexpr auto operator* (typename std::remove_cvref_t<Src>::value_type single, Src&& vec)
{
    return Expressions::MultiplyVecBySingle<extentOf<Src>, Src> (single, std::forward<Src> (vec));
}

/** Returns an expression that multiplies a vector or expression source with a single value.

    @ingroup Expressions
 */
template <class Src>
requires is::anyVctrOrExpression<Src>
constexpr auto operator* (Src&& vec, typename std::remove_cvref_t<Src>::value_type single)
{
    return Expressions::MultiplyVecBySingle<extentOf<Src>, Src> (single, std::forward<Src> (vec));
}


/** Returns an expression that multiplies a vector or expression source with a compile time constant.

    @ingroup Expressions
 */
template <auto constantValue>
constexpr ExpressionChainBuilder<Expressions::MultiplyVecByConstant, Constant<constantValue>> multiplyByConstant;
} // namespace vctr
