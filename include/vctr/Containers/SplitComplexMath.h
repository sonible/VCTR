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

#include <algorithm>
#include <array>

namespace vctr
{

/** Tag base class of all split complex expressions, see SplitComplexMath.h. */
struct SplitComplexExpressionBase {};

namespace is
{
/** Constrains a type to be a split complex expression. */
template <class T>
concept splitComplexExpression = std::derived_from<std::remove_cvref_t<T>, SplitComplexExpressionBase>;
}

namespace detail::splitcomplex
{

template <class T>
using ValueTypeOf = typename std::remove_cvref_t<T>::value_type;

template <class First, class... Others>
constexpr bool sameValueType = (std::is_same_v<ValueTypeOf<First>, ValueTypeOf<Others>> && ...);

template <size_t n>
using ReadPointers = std::array<const void*, n>;

enum class Mode
{
    assign,
    add,
    subtract
};

/** Assigns, adds or subtracts the {real, imag} src pair to the {real, imag} dst reference pair, depending on mode. */
template <Mode mode, class T>
constexpr void assignResult (std::pair<T&, T&> dst, const std::type_identity_t<std::pair<T, T>>& src)
{
    if constexpr (mode == Mode::assign)
    {
        dst.first = src.first;
        dst.second = src.second;
    }
    else if constexpr (mode == Mode::add)
    {
        dst.first += src.first;
        dst.second += src.second;
    }
    else
    {
        dst.first -= src.first;
        dst.second -= src.second;
    }
}

/** Assigns, adds or subtracts a real valued vctr source or expression to a real valued destination span. */
template <Mode mode, class DstSpan, class Src>
constexpr void evaluatePart (DstSpan dst, const Src& src)
{
    if constexpr (mode == Mode::assign)
        dst = src;
    else if constexpr (mode == Mode::add)
        dst += src;
    else
        dst -= src;
}

/** Concatenates multiple read pointer arrays. */
template <size_t... n>
constexpr auto concat (const ReadPointers<n>&... arrays)
{
    ReadPointers<(n + ... + 0)> result {};
    size_t offset = 0;

    ((std::copy (arrays.begin(), arrays.end(), result.begin() + ptrdiff_t (offset)), offset += n), ...);

    return result;
}

/** Returns the buffers read by the real part of a container or expression. */
template <is::splitComplexOrExpression T>
constexpr auto realReads (const T& x)
{
    if constexpr (is::splitComplex<T>)
        return ReadPointers<1> { x.real().data() };
    else
        return x.realReads();
}

/** Returns the buffers read by the imaginary part of a container or expression. */
template <is::splitComplexOrExpression T>
constexpr auto imagReads (const T& x)
{
    if constexpr (is::splitComplex<T>)
        return ReadPointers<1> { x.imag().data() };
    else
        return x.imagReads();
}

/** Returns both buffers of a container. */
template <is::splitComplex T>
constexpr auto allReads (const T& x)
{
    return ReadPointers<2> { x.real().data(), x.imag().data() };
}

/** Returns the real part of a container as span or the (moved or copied) real part expression of an expression. */
template <is::splitComplexOrExpression T>
constexpr auto realPart (T&& x)
{
    if constexpr (is::splitComplex<T>)
        return x.real();
    else
        return std::forward<T> (x).realExpression();
}

/** Returns the imaginary part of a container as span or the (moved or copied) imaginary part expression of an expression. */
template <is::splitComplexOrExpression T>
constexpr auto imagPart (T&& x)
{
    if constexpr (is::splitComplex<T>)
        return x.imag();
    else
        return std::forward<T> (x).imagExpression();
}

} // namespace detail::splitcomplex

//==============================================================================
/** A split complex expression, consisting of one real valued vctr expression (or span) for the real part and one
    for the imaginary part. Both are built from the regular vctr expressions, so the evaluation of each part makes
    use of all SIMD and platform specific vector op accelerations those provide.

    Evaluation is triggered by assigning the expression to a SplitComplexBase container via operator=, += or -=.
    The expression keeps track of the buffers read by each part, so evaluation into a destination that is also a
    source operand is handled safely without allocations: the parts are evaluated in a non-conflicting order or,
    if no such order exists, element by element.

    @ingroup Expressions
 */
template <class RealExpression, class ImagExpression, size_t numRealReads, size_t numImagReads>
class SplitComplexExpression : public SplitComplexExpressionBase
{
public:
    //==============================================================================
    using value_type = std::remove_const_t<typename RealExpression::value_type>;
    using complex_type = std::complex<value_type>;

    static_assert (std::is_same_v<value_type, std::remove_const_t<typename ImagExpression::value_type>>);

    //==============================================================================
    constexpr SplitComplexExpression (RealExpression realExpr,
                                      ImagExpression imagExpr,
                                      detail::splitcomplex::ReadPointers<numRealReads> realReadPointers,
                                      detail::splitcomplex::ReadPointers<numImagReads> imagReadPointers)
        : re (std::move (realExpr)),
          im (std::move (imagExpr)),
          realReadPtrs (realReadPointers),
          imagReadPtrs (imagReadPointers)
    {
        VCTR_ASSERT (re.size() == im.size());
    }

    //==============================================================================
    /** Returns the number of complex values this expression evaluates to. */
    constexpr size_t size() const { return re.size(); }

    /** Evaluates the complex value at index i. */
    constexpr complex_type operator[] (size_t i) const { return { re[i], im[i] }; }

    //==============================================================================
    /** Returns the real part expression. */
    constexpr const RealExpression& realExpression() const& { return re; }
    constexpr RealExpression realExpression() && { return std::move (re); }

    /** Returns the imaginary part expression. */
    constexpr const ImagExpression& imagExpression() const& { return im; }
    constexpr ImagExpression imagExpression() && { return std::move (im); }

    /** Returns the buffers read by the real and the imaginary part expressions. */
    constexpr const auto& realReads() const { return realReadPtrs; }
    constexpr const auto& imagReads() const { return imagReadPtrs; }

    //==============================================================================
    /** Evaluation entry points, called by SplitComplexBase::operator=, += and -= */
    template <class C>
    constexpr void assignTo (SplitComplexBase<C>& dst) const { evaluateTo<detail::splitcomplex::Mode::assign> (dst); }

    template <class C>
    constexpr void addTo (SplitComplexBase<C>& dst) const { evaluateTo<detail::splitcomplex::Mode::add> (dst); }

    template <class C>
    constexpr void subtractFrom (SplitComplexBase<C>& dst) const { evaluateTo<detail::splitcomplex::Mode::subtract> (dst); }

private:
    //==============================================================================
    template <detail::splitcomplex::Mode mode, class C>
    constexpr void evaluateTo (SplitComplexBase<C>& dst) const
    {
        using namespace detail::splitcomplex;

        VCTR_ASSERT (dst.size() == size());

        const auto reads = [] (const auto& readPtrs, const void* ptr) { return std::find (readPtrs.begin(), readPtrs.end(), ptr) != readPtrs.end(); };

        const void* dstRe = dst.real().data();
        const void* dstIm = dst.imag().data();

        // writing the real part first is safe if the imaginary part expression does not read it
        if (! reads (imagReadPtrs, dstRe))
        {
            evaluatePart<mode> (dst.real(), re);
            evaluatePart<mode> (dst.imag(), im);
            return;
        }

        // writing the imaginary part first is safe if the real part expression does not read it
        if (! reads (realReadPtrs, dstIm))
        {
            evaluatePart<mode> (dst.imag(), im);
            evaluatePart<mode> (dst.real(), re);
            return;
        }

        // both parts depend on each other (e.g. in place complex multiplication), evaluate element-wise
        auto dstReal = dst.real();
        auto dstImag = dst.imag();

        for (size_t i = 0; i < size(); ++i)
        {
            const std::pair<value_type, value_type> result { re[i], im[i] };
            assignResult<mode> (std::pair<value_type&, value_type&> { dstReal[i], dstImag[i] }, result);
        }
    }

    //==============================================================================
    RealExpression re;
    ImagExpression im;
    detail::splitcomplex::ReadPointers<numRealReads> realReadPtrs;
    detail::splitcomplex::ReadPointers<numImagReads> imagReadPtrs;
};

namespace detail::splitcomplex
{
template <class R, class I, size_t numR, size_t numI>
constexpr auto makeExpression (R&& realExpr, I&& imagExpr, const ReadPointers<numR>& realReadPtrs, const ReadPointers<numI>& imagReadPtrs)
{
    return SplitComplexExpression<std::remove_cvref_t<R>, std::remove_cvref_t<I>, numR, numI> (std::forward<R> (realExpr),
                                                                                               std::forward<I> (imagExpr),
                                                                                               realReadPtrs,
                                                                                               imagReadPtrs);
}
} // namespace detail::splitcomplex

//==============================================================================
/** @name SplitComplex expressions

    Expressions on SplitComplexBase containers. Addition, subtraction, conjugation and real scalar multiplication
    accept both containers and other split complex expressions, so they can be chained, e.g. dst = a * b + c * d.
    Operations that combine real and imaginary parts (complex multiplication, division, ...) read each operand
    part twice and therefore only accept containers as operands.
    @{
 */

/** Returns an expression for a + b. */
template <is::splitComplexOrExpression A, is::splitComplexOrExpression B>
requires detail::splitcomplex::sameValueType<A, B>
constexpr auto operator+ (A&& a, B&& b)
{
    using namespace detail::splitcomplex;

    VCTR_ASSERT (a.size() == b.size());

    const auto rr = concat (realReads (a), realReads (b));
    const auto ir = concat (imagReads (a), imagReads (b));

    // forwarding twice is fine here, realPart and imagPart only move from distinct members
    return makeExpression (realPart (std::forward<A> (a)) + realPart (std::forward<B> (b)),
                           imagPart (std::forward<A> (a)) + imagPart (std::forward<B> (b)),
                           rr,
                           ir);
}

/** Returns an expression for a - b. */
template <is::splitComplexOrExpression A, is::splitComplexOrExpression B>
requires detail::splitcomplex::sameValueType<A, B>
constexpr auto operator- (A&& a, B&& b)
{
    using namespace detail::splitcomplex;

    VCTR_ASSERT (a.size() == b.size());

    const auto rr = concat (realReads (a), realReads (b));
    const auto ir = concat (imagReads (a), imagReads (b));

    return makeExpression (realPart (std::forward<A> (a)) - realPart (std::forward<B> (b)),
                           imagPart (std::forward<A> (a)) - imagPart (std::forward<B> (b)),
                           rr,
                           ir);
}

/** Returns an expression for a * b. */
template <is::splitComplex A, is::splitComplex B>
requires detail::splitcomplex::sameValueType<A, B>
constexpr auto operator* (const A& a, const B& b)
{
    using namespace detail::splitcomplex;

    VCTR_ASSERT (a.size() == b.size());

    const auto reads = concat (allReads (a), allReads (b));

    return makeExpression (a.real() * b.real() - a.imag() * b.imag(),
                           a.real() * b.imag() + a.imag() * b.real(),
                           reads,
                           reads);
}

/** Returns an expression for a * conj (b). */
template <is::splitComplex A, is::splitComplex B>
requires detail::splitcomplex::sameValueType<A, B>
constexpr auto mulConj (const A& a, const B& b)
{
    using namespace detail::splitcomplex;

    VCTR_ASSERT (a.size() == b.size());

    const auto reads = concat (allReads (a), allReads (b));

    return makeExpression (a.real() * b.real() + a.imag() * b.imag(),
                           a.imag() * b.real() - a.real() * b.imag(),
                           reads,
                           reads);
}

/** Returns an expression for a / b. */
template <is::splitComplex A, is::splitComplex B>
requires detail::splitcomplex::sameValueType<A, B>
constexpr auto operator/ (const A& a, const B& b)
{
    using namespace detail::splitcomplex;

    VCTR_ASSERT (a.size() == b.size());

    const auto reads = concat (allReads (a), allReads (b));

    return makeExpression ((a.real() * b.real() + a.imag() * b.imag()) / (b.real() * b.real() + b.imag() * b.imag()),
                           (a.imag() * b.real() - a.real() * b.imag()) / (b.real() * b.real() + b.imag() * b.imag()),
                           reads,
                           reads);
}

/** Returns an expression for src * factor, where factor is a real scalar. */
template <is::splitComplexOrExpression A>
constexpr auto operator* (A&& src, detail::splitcomplex::ValueTypeOf<A> factor)
{
    using namespace detail::splitcomplex;

    const auto rr = realReads (src);
    const auto ir = imagReads (src);

    return makeExpression (realPart (std::forward<A> (src)) * factor,
                           imagPart (std::forward<A> (src)) * factor,
                           rr,
                           ir);
}

/** Returns an expression for factor * src, where factor is a real scalar. */
template <is::splitComplexOrExpression A>
constexpr auto operator* (detail::splitcomplex::ValueTypeOf<A> factor, A&& src)
{
    return std::forward<A> (src) * factor;
}

/** Returns an expression for src * factor, where factor is a complex scalar. */
template <is::splitComplex A>
constexpr auto operator* (const A& src, std::complex<detail::splitcomplex::ValueTypeOf<A>> factor)
{
    using namespace detail::splitcomplex;

    const auto fRe = factor.real();
    const auto fIm = factor.imag();
    const auto reads = allReads (src);

    return makeExpression (src.real() * fRe - src.imag() * fIm,
                           src.real() * fIm + src.imag() * fRe,
                           reads,
                           reads);
}

/** Returns an expression for factor * src, where factor is a complex scalar. */
template <is::splitComplex A>
constexpr auto operator* (std::complex<detail::splitcomplex::ValueTypeOf<A>> factor, const A& src)
{
    return src * factor;
}

/** Returns an expression for conj (src). */
template <is::splitComplexOrExpression A>
constexpr auto conj (A&& src)
{
    using namespace detail::splitcomplex;

    const auto rr = realReads (src);
    const auto ir = imagReads (src);

    return makeExpression (realPart (std::forward<A> (src)),
                           multiplyByConstant<-1> << imagPart (std::forward<A> (src)),
                           rr,
                           ir);
}

/** Returns a real valued vctr expression for |src|^2. */
template <is::splitComplex A>
constexpr auto magnitudeSquared (const A& src)
{
    return src.real() * src.real() + src.imag() * src.imag();
}

/** Returns a real valued vctr expression for |src|. */
template <is::splitComplex A>
constexpr auto magnitude (const A& src)
{
    return vctr::sqrt << magnitudeSquared (src);
}

/** @} */

//==============================================================================
/** @name SplitComplex math

    Allocation free math operations on SplitComplexBase containers. All operands must have the same size and the
    destination must be mutable and already sized, nothing is resized. The destination may be identical to a source
    operand (in-place operation) but must not partially overlap with it.
    @{
 */

/** dst = srcA + srcB */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<SplitComplexBase<A>, SplitComplexBase<B>, SplitComplexBase<C>>
void add (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    dst = srcA + srcB;
}

/** dst = srcA - srcB */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<SplitComplexBase<A>, SplitComplexBase<B>, SplitComplexBase<C>>
void sub (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    dst = srcA - srcB;
}

/** dst = srcA * srcB */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<SplitComplexBase<A>, SplitComplexBase<B>, SplitComplexBase<C>>
void mul (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    dst = srcA * srcB;
}

/** dst = srcA * conj (srcB) */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<SplitComplexBase<A>, SplitComplexBase<B>, SplitComplexBase<C>>
void mulConj (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    dst = mulConj (srcA, srcB);
}

/** dst += srcA * srcB */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<SplitComplexBase<A>, SplitComplexBase<B>, SplitComplexBase<C>>
void mulAdd (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    dst += srcA * srcB;
}

/** dst -= srcA * srcB */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<SplitComplexBase<A>, SplitComplexBase<B>, SplitComplexBase<C>>
void mulSub (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    dst -= srcA * srcB;
}

/** dst = srcA / srcB */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<SplitComplexBase<A>, SplitComplexBase<B>, SplitComplexBase<C>>
void div (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    dst = srcA / srcB;
}

/** dst = conj (src) */
template <class A, class C>
requires detail::splitcomplex::sameValueType<SplitComplexBase<A>, SplitComplexBase<C>>
void conj (const SplitComplexBase<A>& src, SplitComplexBase<C>& dst)
{
    VCTR_ASSERT (src.size() == dst.size());

    // avoids the redundant real part self copy compared to dst = conj (src) for in place operations
    if (src.real().data() != dst.real().data())
        dst.real() = src.real();

    dst.imag() = multiplyByConstant<-1> << src.imag();
}

/** dst = src * factor, where factor is a real scalar. */
template <class A, class C>
requires detail::splitcomplex::sameValueType<SplitComplexBase<A>, SplitComplexBase<C>>
void mulReal (const SplitComplexBase<A>& src, typename SplitComplexBase<A>::value_type factor, SplitComplexBase<C>& dst)
{
    dst = src * factor;
}

/** dst = src * factor, where factor is a complex scalar. */
template <class A, class C>
requires detail::splitcomplex::sameValueType<SplitComplexBase<A>, SplitComplexBase<C>>
void mulComplex (const SplitComplexBase<A>& src, std::complex<typename SplitComplexBase<A>::value_type> factor, SplitComplexBase<C>& dst)
{
    dst = src * factor;
}

/** dst = |src|^2 where dst is a real valued container. */
template <class A, class Dst>
requires is::anyVctr<Dst> && std::same_as<typename SplitComplexBase<A>::value_type, typename std::remove_cvref_t<Dst>::value_type>
void magnitudeSquared (const SplitComplexBase<A>& src, Dst&& dst)
{
    VCTR_ASSERT (src.size() == dst.size());
    dst = magnitudeSquared (src);
}

/** dst = |src| where dst is a real valued container. */
template <class A, class Dst>
requires is::anyVctr<Dst> && std::same_as<typename SplitComplexBase<A>::value_type, typename std::remove_cvref_t<Dst>::value_type>
void magnitude (const SplitComplexBase<A>& src, Dst&& dst)
{
    VCTR_ASSERT (src.size() == dst.size());
    dst = magnitude (src);
}

/** @} */

} // namespace vctr
} // namespace is
