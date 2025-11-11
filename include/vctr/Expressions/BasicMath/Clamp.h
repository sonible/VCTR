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

template <size_t extent, class SrcType, is::constantWithType<bool> ClampLow, is::constantWithType<bool> ClampHigh>
class Clamp : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Clamp, src)

    static constexpr bool clampLow = ClampLow::value;
    static constexpr bool clampHigh = ClampHigh::value;

    constexpr void applyRuntimeArgs (value_type newLowerBound, value_type newUpperBound)
    {
        lowerBound = newLowerBound;
        upperBound = newUpperBound;
    }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        assertBoundsAreSet();

        if constexpr (clampLow && clampHigh)
            return std::clamp (src[i], lowerBound, upperBound);

        if constexpr (clampLow && ! clampHigh)
            return std::max (src[i], lowerBound);

        if constexpr (! clampLow && clampHigh)
            return std::min (src[i], upperBound);
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        assertBoundsAreSet();

        if constexpr (clampLow && ! clampHigh)
            Expression::Accelerate::clampLow (src.evalNextVectorOpInExpressionChain (dst), lowerBound, dst, size());

        if constexpr (clampHigh)
        {
            auto l = clampLow ? lowerBound : std::numeric_limits<value_type>::lowest();
            Expression::Accelerate::clamp (src.evalNextVectorOpInExpressionChain (dst), l, upperBound, dst, size());
        }

        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        assertBoundsAreSet();
        const auto* s = src.evalNextVectorOpInExpressionChain (dst);

        if constexpr (clampLow)
        {
            Expression::IPP::clampLow (s, lowerBound, dst, sizeToInt (size()));

            if constexpr (clampHigh)
                s = dst;
        }

        if constexpr (clampHigh)
            Expression::IPP::clampHigh (s, upperBound, dst, sizeToInt (size()));

        return dst;
    }

private:
    void assertBoundsAreSet() const
    {
        // If you hit one of the assertions here, applyRuntimeArgs has not been called as expected.
        // This should not happen under normal circumstances
        if constexpr (clampLow)
            VCTR_ASSERT (lowerBound != std::numeric_limits<value_type>::max());

        if constexpr (clampHigh)
            VCTR_ASSERT (upperBound != std::numeric_limits<value_type>::max());
    }

    value_type lowerBound = std::numeric_limits<value_type>::max();
    value_type upperBound = std::numeric_limits<value_type>::max();
};

template <size_t extent, class SrcType>
using ClampLow = Clamp<extent, SrcType, std::true_type, std::false_type>;

template <size_t extent, class SrcType>
using ClampHigh = Clamp<extent, SrcType, std::false_type, std::true_type>;

template <size_t extent, class SrcType>
using ClampLowHigh = Clamp<extent, SrcType, std::true_type, std::true_type>;

template <size_t extent, class SrcType, is::constant LowerBound, is::constant UpperBound>
class ClampByConstant : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    static constexpr value_type lowerBound = LowerBound::value;
    static constexpr value_type upperBound = UpperBound::value;

    static constexpr bool clampLow = ! is::disabledConstant<LowerBound>;
    static constexpr bool clampHigh = ! is::disabledConstant<UpperBound>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (ClampByConstant, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        if constexpr (clampLow && clampHigh)
            return std::clamp (src[i], lowerBound, upperBound);

        if constexpr (clampLow && ! clampHigh)
            return std::max (src[i], lowerBound);

        if constexpr (! clampLow && clampHigh)
            return std::min (src[i], upperBound);

        return std::max (lowerBound, src[i]);
    }

    //==============================================================================
    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void prepareAVXEvaluation() const
    requires (has::prepareAVXEvaluation<SrcType> && Expression::CommonElement::isRealFloat)
    {
        src.prepareAVXEvaluation();

        if constexpr (clampLow)
            lowerBoundSIMD.avx = Expression::AVX::broadcast (lowerBound);

        if constexpr (clampHigh)
            upperBoundSIMD.avx = Expression::AVX::broadcast (upperBound);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("fma") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        auto x = src.getAVX (i);

        if constexpr (clampLow)
            x = Expression::AVX::max (lowerBoundSIMD.avx, x);

        if constexpr (clampHigh)
            x = Expression::AVX::min (x, upperBoundSIMD.avx);

        return x;
    }

    //==============================================================================
    // SSE Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void prepareSSEEvaluation() const
    requires (has::prepareSSEEvaluation<SrcType> && Expression::CommonElement::isRealFloat)
    {
        src.prepareSSEEvaluation();

        if constexpr (clampLow)
            lowerBoundSIMD.sse = Expression::SSE::broadcast (lowerBound);

        if constexpr (clampHigh)
            upperBoundSIMD.sse = Expression::SSE::broadcast (upperBound);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
    requires (archX64 && has::getSSE<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        auto x = src.getSSE (i);

        if constexpr (clampLow)
            x = Expression::SSE::max (lowerBoundSIMD.sse, x);

        if constexpr (clampHigh)
            x = Expression::SSE::min (x, upperBoundSIMD.sse);

        return x;
    }

    //==============================================================================
    // Neon Implementation
    void prepareNeonEvaluation() const
    requires (archARM && has::prepareNeonEvaluation<SrcType> && Expression::CommonElement::isRealFloat)
    {
        src.prepareNeonEvaluation();

        if constexpr (clampLow)
            lowerBoundSIMD.neon = Expression::Neon::broadcast (lowerBound);

        if constexpr (clampHigh)
            upperBoundSIMD.neon = Expression::Neon::broadcast (upperBound);
    }

    NeonRegister<value_type> getNeon (size_t i) const
    requires (archARM && has::getNeon<SrcType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        auto x = src.getNeon (i);

        if constexpr (clampLow)
            x = Expression::Neon::max (lowerBoundSIMD.neon, x);

        if constexpr (clampHigh)
            x = Expression::Neon::min (x, upperBoundSIMD.neon);

        return x;
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        if constexpr (clampLow && ! clampHigh)
            Expression::Accelerate::clampLow (src.evalNextVectorOpInExpressionChain (dst), lowerBound, dst, size());

        if constexpr (clampHigh)
        {
            auto l = clampLow ? lowerBound : std::numeric_limits<value_type>::lowest();
            Expression::Accelerate::clamp (src.evalNextVectorOpInExpressionChain (dst), l, upperBound, dst, size());
        }

        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        const auto* s = src.evalNextVectorOpInExpressionChain (dst);

        if constexpr (clampLow)
        {
            Expression::IPP::clampLow (s, lowerBound, dst, sizeToInt (size()));

            if constexpr (clampHigh)
                s = dst;
        }

        if constexpr (clampHigh)
            Expression::IPP::clampHigh (s, upperBound, dst, sizeToInt (size()));

        return dst;
    }

private:
    mutable SIMDRegisterUnion<Expression> upperBoundSIMD;
    mutable SIMDRegisterUnion<Expression> lowerBoundSIMD;
};

} // namespace vctr::expressions

namespace vctr
{

/** Ensures that the elements are not lower than lowerBound.

    This version takes a runtime computed value. If a compile time fix constant lower bound suits your needs,
    consider clampLowByConstant.

    @ingroup Expressions
 */
template <class T>
constexpr auto clampLow (T lowerBound)
{
    return makeExpressionChainBuilderWithRuntimeArgs<expressions::ClampLow> (lowerBound, std::numeric_limits<T>::max());
}

/** Ensures that the elements are not greater than upperBound.

    This version takes a runtime computed value. If a compile time fix constant lower bound suits your needs,
    consider clampHighByConstant.

    @ingroup Expressions
 */
template <class T>
constexpr auto clampHigh (T upperBound)
{
    return makeExpressionChainBuilderWithRuntimeArgs<expressions::ClampHigh> (std::numeric_limits<T>::max(), upperBound);
}

/** Ensures that the elements are not lower than lowerBound and not higher than upperBound.

    This version takes runtime computed values. If compile time fix constants suit your needs,
    consider clampByConstant.

    @ingroup Expressions
 */
template <class T>
constexpr auto clamp (T lowerBound, T upperBound)
{
    return makeExpressionChainBuilderWithRuntimeArgs<expressions::ClampLowHigh> (lowerBound, upperBound);
}

/** Ensures that the elements are within the range.

    This version takes runtime computed values. If compile time fix constants suit your needs,
    consider clampByRange.

    @ingroup Expressions
 */
template <is::range Range>
constexpr auto clamp (const Range& range)
{
    return makeExpressionChainBuilderWithRuntimeArgs<expressions::ClampLowHigh> (range.getStart(), range.getEnd());
}


/** Ensures that the elements are not lower than lowerBound.

    Note: At this time, most compilers will refuse to take floating point non-type template values.
    Passing an integer bound will also work for floating point expressions. In case you need decimal precision,
    better use clampLow which works fine with floating point values.

    @ingroup Expressions
 */
template <auto lowerBound>
constexpr inline ExpressionChainBuilder<expressions::ClampByConstant, Constant<lowerBound>, DisabledConstant> clampLowByConstant;

/** Ensures that the elements are not higher than upperBound.

    Note: At this time, most compilers will refuse to take floating point non-type template values.
    Passing an integer bound will also work for floating point expressions. In case you need decimal precision,
    better use clampHigh which works fine with floating point values.

    @ingroup Expressions
 */
template <auto upperBound>
constexpr inline ExpressionChainBuilder<expressions::ClampByConstant, DisabledConstant, Constant<upperBound>> clampHighByConstant;

/** Ensures that the elements are not lower than lowerBound and not higher than upperBound.

    Note: At this time, most compilers will refuse to take floating point non-type template values.
    Passing integer bounds will also work for floating point expressions. In case you need decimal precision,
    better use clamp which works fine with floating point values.

    @ingroup Expressions
 */
template <auto lowerBound, auto upperBound>
constexpr inline ExpressionChainBuilder<expressions::ClampByConstant, Constant<lowerBound>, Constant<upperBound>> clampByConstant;

/** Ensures that the elements are within the range.

    @ingroup Expressions
 */
template <const auto& range>
requires is::range<decltype (range)>
constexpr inline ExpressionChainBuilder<expressions::ClampByConstant, ConstantRangeStart<range>, ConstantRangeEnd<range>> clampToRange;

} // namespace vctr
