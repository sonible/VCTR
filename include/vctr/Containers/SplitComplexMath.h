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

namespace vctr
{
namespace is
{
template <class T>
concept derivedFromSplitComplexBase = requires (T& d) { [] <class ContainerType> (SplitComplexBase<ContainerType>&) {} (d); };

template <class T>
concept derivedFromSplitComplexExpression = requires (T& d) { [] <expression... Ts> (SplitComplexExpression<Ts...>&) {} (d); };

template <class T>
concept anySplitComplexVectorOrExpression = derivedFromSplitComplexBase<T> || derivedFromSplitComplexExpression<T>;

// TODO: write concept to make sure that value type is the same!
//template <class... Containers>
//concept 
} // namespace is

template <is::expression RealExpression, is::expression ImagExpression>
constexpr auto makeSplitComplexExpression (RealExpression&& realExpression, ImagExpression&& imagExpression)
{
    VCTR_ASSERT (realExpression.size() == imagExpression.size());
    return SplitComplexExpression<RealExpression, ImagExpression> { std::forward<RealExpression> (realExpression), std::forward<ImagExpression> (imagExpression) };
}

/** Returns an expression that adds two split-complex vector or expression sources.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcAType, is::anySplitComplexVectorOrExpression SrcBType>
constexpr auto operator+ (SrcAType&& srcA, SrcBType&& srcB)
{
    return makeSplitComplexExpression (srcA.real() + srcB.real(), srcA.imag() + srcB.imag());
}

/** Returns an expression that subtracts split-complex vector or expression b from split-complex vector or expression a.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcAType, is::anySplitComplexVectorOrExpression SrcBType>
constexpr auto operator- (SrcAType&& srcA, SrcBType&& srcB)
{
    return makeSplitComplexExpression (srcA.real() - srcB.real(), srcA.imag() - srcB.imag());
}

/** Returns an expression that multiplies split-complex vector or expression a with split-complex vector or expression b.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcAType, is::anySplitComplexVectorOrExpression SrcBType>
constexpr auto operator* (SrcAType&& srcA, SrcBType&& srcB)
{
    auto realMulExpression = srcA.real() * srcB.real();
    auto imagMulExpression = srcA.imag() * srcB.imag();

    return makeSplitComplexExpression (realMulExpression - imagMulExpression, realMulExpression + imagMulExpression);
}

// TODO: DESC!
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto operator* (SrcType&& src, typename std::remove_cvref_t<SrcType>::value_type single)
{
    return makeSplitComplexExpression (src.real() * single, src.imag() * single);
}

// TODO: DESC!
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto operator* (SrcType&& src, std::complex<typename std::remove_cvref_t<SrcType>::value_type> single)
{
    return makeSplitComplexExpression (src.real() * single.real() - src.imag() * single.imag(),
                                       src.real() * single.imag() + src.imag() * single.real());
}

// TODO: DESC!
template <is::anySplitComplexVectorOrExpression SrcAType, is::anySplitComplexVectorOrExpression SrcBType>
constexpr auto operator/ (SrcAType&& srcA, SrcBType&& srcB)
{
    auto denominator = srcB.real() * srcB.real() + srcB.imag() * srcB.imag();

    return makeSplitComplexExpression ((srcA.real() * srcB.real() + srcA.imag() * srcB.imag()) / denominator, 
                                       (srcA.imag() * srcB.real() - srcA.real() * srcB.imag()) / denominator);
}

// TODO: DESC!
template <is::anySplitComplexVectorOrExpression SrcAType, is::anySplitComplexVectorOrExpression SrcBType>
constexpr auto multiplyConjugate (SrcAType&& srcA, SrcBType&& srcB)
{
    return makeSplitComplexExpression (srcA.real() * srcB.real() + srcA.imag() * srcB.imag(),
                                       srcA.imag() * srcB.real() - srcA.real() * srcB.imag());
}

// TODO: DESC!
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto splitComplexConjugate (SrcType&& src) // TODO: RENAME?
{
    return makeSplitComplexExpression (multiplyByConstant<+1> << src.real(), // TODO: REMOVE multiply!
                                       multiplyByConstant<-1> << src.imag());
}

// TODO: compute expression
// TODO: DESC!
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto magnitudeSquared (SrcType&& src)
{
    return src.real() * src.real() + src.imag() * src.imag();
}

// TODO: DESC!
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto magnitude (SrcType&& src)
{
    return sqrt << magnitudeSquared (src);
}
} // namespace vctr
