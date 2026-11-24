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
/** Combines real and imaginary expressions into a split-complex expression.

    The two expressions must have the same size.

    @ingroup Expressions
 */
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
    // (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
    // The sub-expressions are built inline rather than stored in local variables first: expressions hold lvalue
    // sources by reference, so an expression built from locals would dangle after returning.
    return makeSplitComplexExpression (srcA.real() * srcB.real() - srcA.imag() * srcB.imag(),
                                       srcA.real() * srcB.imag() + srcA.imag() * srcB.real());
}

/** Returns an expression that multiplies each value in a split-complex source by a real scalar.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto operator* (SrcType&& src, typename std::remove_cvref_t<SrcType>::value_type single)
{
    return makeSplitComplexExpression (src.real() * single, src.imag() * single);
}

/** Returns an expression that multiplies each value in a split-complex source by a complex scalar.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto operator* (SrcType&& src, std::complex<typename std::remove_cvref_t<SrcType>::value_type> single)
{
    return makeSplitComplexExpression (src.real() * single.real() - src.imag() * single.imag(),
                                       src.real() * single.imag() + src.imag() * single.real());
}

/** Returns an expression that divides each value in a split-complex source by a real scalar.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto operator/ (SrcType&& src, typename std::remove_cvref_t<SrcType>::value_type single)
{
    return makeSplitComplexExpression (src.real() / single, src.imag() / single);
}

/** Returns an expression that divides each value in a split-complex source by a complex scalar.

    The reciprocal of the scalar is computed once, so this is evaluated as a multiplication by a complex scalar.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto operator/ (SrcType&& src, std::complex<typename std::remove_cvref_t<SrcType>::value_type> single)
{
    return std::forward<SrcType> (src) * (std::conj (single) / std::norm (single));
}

/** Returns an expression that adds a complex scalar to each value in a split-complex source.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto operator+ (SrcType&& src, std::complex<typename std::remove_cvref_t<SrcType>::value_type> single)
{
    return makeSplitComplexExpression (src.real() + single.real(), src.imag() + single.imag());
}

/** Returns an expression that subtracts a complex scalar from each value in a split-complex source.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto operator- (SrcType&& src, std::complex<typename std::remove_cvref_t<SrcType>::value_type> single)
{
    return makeSplitComplexExpression (src.real() - single.real(), src.imag() - single.imag());
}

/** Returns an expression that divides split-complex source a by split-complex source b.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcAType, is::anySplitComplexVectorOrExpression SrcBType>
constexpr auto operator/ (SrcAType&& srcA, SrcBType&& srcB)
{
    // (a + bi) / (c + di) = ((ac + bd) + (bc - ad)i) / (c² + d²)
    // The denominator is built inline for both parts on purpose, see operator*.
    return makeSplitComplexExpression ((srcA.real() * srcB.real() + srcA.imag() * srcB.imag()) / (srcB.real() * srcB.real() + srcB.imag() * srcB.imag()),
                                       (srcA.imag() * srcB.real() - srcA.real() * srcB.imag()) / (srcB.real() * srcB.real() + srcB.imag() * srcB.imag()));
}

/** Returns an expression that multiplies split-complex source a by the complex conjugate of source b.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcAType, is::anySplitComplexVectorOrExpression SrcBType>
constexpr auto multiplyConjugate (SrcAType&& srcA, SrcBType&& srcB)
{
    return makeSplitComplexExpression (srcA.real() * srcB.real() + srcA.imag() * srcB.imag(),
                                       srcA.imag() * srcB.real() - srcA.real() * srcB.imag());
}

/** Returns an expression containing the complex conjugate of each value in a split-complex source.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto splitComplexConjugate (SrcType&& src) // TODO: RENAME?
{
    return makeSplitComplexExpression (multiplyByConstant<+1> << src.real(), // TODO: REMOVE multiply!
                                       multiplyByConstant<-1> << src.imag());
}

/** Returns an expression containing the squared magnitude of each value in a split-complex source.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto magnitudeSquared (SrcType&& src)
{
    return src.real() * src.real() + src.imag() * src.imag();
}

/** Returns an expression containing the magnitude of each value in a split-complex source.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto magnitude (SrcType&& src)
{
    return sqrt << magnitudeSquared (src);
}

/** Returns an expression that converts a split-complex source into interleaved std::complex values.

    This is the counterpart of constructing a split-complex container from an interleaved container. Assigning a
    split-complex expression evaluates it directly into the interleaved destination, without a split-complex
    intermediate buffer, e.g.

    @code
    vctr::Vector<std::complex<float>> product = vctr::realImagToComplex (a * b);
    @endcode

    If the source is a split-complex container, its real and imaginary parts are passed on to
    realImagToComplex (RealType&&, ImagType&&) as spans, so Apple Accelerate or Intel IPP is used if available.

    @ingroup Expressions
 */
template <is::anySplitComplexVectorOrExpression SrcType>
constexpr auto realImagToComplex (SrcType&& src)
{
    // An rvalue split-complex expression hands over its component expressions, so that the returned expression doesn't
    // refer to the temporary.
    if constexpr (is::derivedFromSplitComplexExpression<SrcType>)
        return realImagToComplex (std::forward<SrcType> (src).realExpression, std::forward<SrcType> (src).imagExpression);
    else
        return realImagToComplex (src.real(), src.imag());
}
} // namespace vctr
