/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2023 by sonible GmbH.

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

template <size_t extent, class SrcType, is::rangeWithValueType<ValueType<SrcType>> RangeType>
requires is::realFloatNumber<ValueType<SrcType>>
class Map : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;
    
    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Map, src)

    void applyRuntimeArgs (RangeType newSrcRange, RangeType newDstRange)
    {
        srcRangeStartNegated = -newSrcRange.getStart();
        srcDstLenRatio = newDstRange.getLength() / newSrcRange.getLength();
        dstRangeStart = newDstRange.getStart();
    }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return (src[i] + srcRangeStartNegated) * srcDstLenRatio + dstRangeStart;
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        const auto s = size();

        Expression::Accelerate::add (src.evalNextVectorOpInExpressionChain (dst), srcRangeStartNegated, dst, s);
        Expression::Accelerate::mul (dst, srcDstLenRatio, dst, s);
        Expression::Accelerate::add (dst, dstRangeStart, dst, s);

        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        const auto s = size();

        Expression::IPP::add (src.evalNextVectorOpInExpressionChain (dst), srcRangeStartNegated, dst, s);
        Expression::IPP::mul (dst, srcDstLenRatio, dst, s);
        Expression::IPP::add (dst, dstRangeStart, dst, s);

        return dst;
    }

private:
    //==============================================================================
    value_type srcRangeStartNegated = 0;
    value_type srcDstLenRatio = 0;
    value_type dstRangeStart = 0;
};

template <size_t extent, class SrcType, is::rangeWithValueType<ValueType<SrcType>> RangeType>
requires is::realFloatNumber<ValueType<SrcType>>
class MapFrom0To1 : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (MapFrom0To1, src)

    void applyRuntimeArgs (RangeType newDstRange)
    {
        dstRangeStart = newDstRange.getStart();
        dstRangeLen = newDstRange.getLength();
    }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return src[i] * dstRangeLen + dstRangeStart;
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        const auto s = size();

        Expression::Accelerate::mul (src.evalNextVectorOpInExpressionChain (dst), dstRangeLen, dst, s);
        Expression::Accelerate::add (dst, dstRangeStart, dst, s);

        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        const auto s = size();

        Expression::IPP::mul (src.evalNextVectorOpInExpressionChain (dst), dstRangeLen, dst, s);
        Expression::IPP::add (dst, dstRangeStart, dst, s);

        return dst;
    }

private:
    //==============================================================================
    value_type dstRangeStart = 0;
    value_type dstRangeLen = 0;
};

template <size_t extent, class SrcType, is::rangeWithValueType<ValueType<SrcType>> RangeType>
requires is::realFloatNumber<ValueType<SrcType>>
class MapTo0To1 : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (MapTo0To1, src)

    void applyRuntimeArgs (RangeType newSrcRange)
    {
        srcRangeStartNegated = -newSrcRange.getStart();
        srcRangeLen = newSrcRange.getLength();
    }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return (src[i] + srcRangeStartNegated) / srcRangeLen;
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        const auto s = size();

        Expression::Accelerate::add (src.evalNextVectorOpInExpressionChain (dst), srcRangeStartNegated, dst, s);
        Expression::Accelerate::div (dst, srcRangeLen, dst, s);

        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        const auto s = size();

        Expression::IPP::add (src.evalNextVectorOpInExpressionChain (dst), srcRangeStartNegated, dst, s);
        Expression::IPP::div (dst, srcRangeLen, dst, s);

        return dst;
    }

private:
    //==============================================================================
    value_type srcRangeStartNegated = 0;
    value_type srcRangeLen = 0;
};

} // namespace vctr::expressions

namespace vctr
{

/** Maps all source element values from srcValueRange to values in dstValueRange.

    The expression does not check if the source values are actually in the expected range.

    @ingroup Expressions
 */
template <is::range RangeType>
auto map (RangeType&& srcValueRange, RangeType&& dstValueRange)
{
    return makeTemplateExpressionChainBuilderWithRuntimeArgs<expressions::Map, std::remove_cvref_t<RangeType>> (std::forward<RangeType> (srcValueRange), std::forward<RangeType> (dstValueRange));
}

/** Maps all source element values from the range srcValueRangeStart to srcValueRangeEnd to values in [dstValueRangeStart, dstValueRangeEnd].

    The expression does not check if the source values are actually in the expected range.

    @ingroup Expressions
 */
template <is::realFloatNumber T>
auto map (T srcValueRangeStart, T srcValueRangeEnd, T dstValueRangeStart, T dstValueRangeEnd)
{
    return map (Range (srcValueRangeStart, srcValueRangeEnd), Range (dstValueRangeStart, dstValueRangeEnd));
}

/** Maps all source element values from the range [0.0 to 1.0] to values in dstValueRange.

    The expression does not check if the source values are actually in the expected range.

    @ingroup Expressions
 */
template <is::range RangeType>
auto mapFrom0To1 (RangeType&& dstValueRange)
{
    return makeTemplateExpressionChainBuilderWithRuntimeArgs<expressions::MapFrom0To1, std::remove_cvref_t<RangeType>> (std::forward<RangeType> (dstValueRange));
}

/** Maps all source element values from the range [0.0 to 1.0] to values in the range from [dstValueRangeStart to dstValueRangeEnd].

    The expression does not check if the source values are actually in the expected range.

    @ingroup Expressions
 */
template <is::realFloatNumber T>
auto mapFrom0To1 (T dstValueRangeStart, T dstValueRangeEnd)
{
    return mapFrom0To1 (vctr::Range (dstValueRangeStart, dstValueRangeEnd));
}

/** Maps all source element values from srcValueRange to values in the range [0.0 to 1.0].

    The expression does not check if the source values are actually in the expected range.

    @ingroup Expressions
 */
template <is::range RangeType>
auto mapTo0To1 (RangeType&& srcValueRange)
{
    return makeTemplateExpressionChainBuilderWithRuntimeArgs<expressions::MapTo0To1, std::remove_cvref_t<RangeType>> (std::forward<RangeType> (srcValueRange));
}

/** Maps all source element values from teh range [srcValueRangeStart to srcValueRangeEnd] to values in the range [0.0 to 1.0].

    The expression does not check if the source values are actually in the expected range.

    @ingroup Expressions
 */
template <is::realFloatNumber T>
auto mapTo0To1 (T srcValueRangeStart, T srcValueRangeEnd)
{
    return mapTo0To1 (Range (srcValueRangeStart, srcValueRangeEnd));
}

}