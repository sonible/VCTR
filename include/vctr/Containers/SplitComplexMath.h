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

namespace detail::splitcomplex
{

template <class... Containers>
constexpr bool sameValueType = (std::is_same_v<typename SplitComplexBase<Containers>::value_type, typename SplitComplexBase<std::tuple_element_t<0, std::tuple<Containers...>>>::value_type> && ...);

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

/** Evaluates operation for every element and assigns, adds or subtracts the returned {real, imag} pair to dst,
    depending on mode.

    The operation receives the real and imaginary values of all sources at the current index in the order
    (aRe, aIm, bRe, bIm, ...).
 */
template <Mode mode = Mode::assign, class C, class Operation, class... Srcs>
void evaluate (SplitComplexBase<C>& dst, Operation&& operation, const SplitComplexBase<Srcs>&... srcs)
{
    using T = typename SplitComplexBase<C>::value_type;

    VCTR_ASSERT (((srcs.size() == dst.size()) && ...));

    for (size_t i = 0; i < dst.size(); ++i)
    {
        const auto result = std::apply (operation, std::tuple_cat (std::tuple { srcs.real()[i], srcs.imag()[i] }...));
        assignResult<mode> (std::pair<T&, T&> { dst.real()[i], dst.imag()[i] }, result);
    }
}

} // namespace detail::splitcomplex

/** @name SplitComplex math

    Allocation free math operations on SplitComplexBase containers. All operands must have the same size and the
    destination must be mutable and already sized, nothing is resized. The destination may be identical to a source
    operand (in-place operation) but must not partially overlap with it.
    @{
 */

/** dst = srcA + srcB */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<A, B, C>
void add (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    VCTR_ASSERT (srcA.size() == srcB.size() && srcA.size() == dst.size());

    dst.real() = srcA.real() + srcB.real();
    dst.imag() = srcA.imag() + srcB.imag();
}

/** dst = srcA - srcB */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<A, B, C>
void sub (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    VCTR_ASSERT (srcA.size() == srcB.size() && srcA.size() == dst.size());

    dst.real() = srcA.real() - srcB.real();
    dst.imag() = srcA.imag() - srcB.imag();
}

/** dst = srcA * srcB */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<A, B, C>
void mul (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    detail::splitcomplex::evaluate (dst, [] (auto aRe, auto aIm, auto bRe, auto bIm) {
        return std::pair { aRe * bRe - aIm * bIm, aRe * bIm + aIm * bRe };
    }, srcA, srcB);
}

/** dst = srcA * conj (srcB) */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<A, B, C>
void mulConj (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    detail::splitcomplex::evaluate (dst, [] (auto aRe, auto aIm, auto bRe, auto bIm) {
        return std::pair { aRe * bRe + aIm * bIm, aIm * bRe - aRe * bIm };
    }, srcA, srcB);
}

/** dst += srcA * srcB */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<A, B, C>
void mulAdd (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    using namespace detail::splitcomplex;

    evaluate<Mode::add> (dst, [] (auto aRe, auto aIm, auto bRe, auto bIm) {
        return std::pair { aRe * bRe - aIm * bIm, aRe * bIm + aIm * bRe };
    }, srcA, srcB);
}

/** dst -= srcA * srcB */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<A, B, C>
void mulSub (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    using namespace detail::splitcomplex;

    evaluate<Mode::subtract> (dst, [] (auto aRe, auto aIm, auto bRe, auto bIm) {
        return std::pair { aRe * bRe - aIm * bIm, aRe * bIm + aIm * bRe };
    }, srcA, srcB);
}

/** dst = srcA / srcB */
template <class A, class B, class C>
requires detail::splitcomplex::sameValueType<A, B, C>
void div (const SplitComplexBase<A>& srcA, const SplitComplexBase<B>& srcB, SplitComplexBase<C>& dst)
{
    detail::splitcomplex::evaluate (dst, [] (auto aRe, auto aIm, auto bRe, auto bIm) {
        const auto denominator = bRe * bRe + bIm * bIm;
        return std::pair { (aRe * bRe + aIm * bIm) / denominator, (aIm * bRe - aRe * bIm) / denominator };
    }, srcA, srcB);
}

/** dst = conj (src) */
template <class A, class C>
requires detail::splitcomplex::sameValueType<A, C>
void conj (const SplitComplexBase<A>& src, SplitComplexBase<C>& dst)
{
    VCTR_ASSERT (src.size() == dst.size());

    if (src.real().data() != dst.real().data())
        dst.real() = src.real();

    dst.imag() = multiplyByConstant<-1> << src.imag();
}

/** dst = src * factor, where factor is a real scalar. */
template <class A, class C>
requires detail::splitcomplex::sameValueType<A, C>
void mulReal (const SplitComplexBase<A>& src, typename SplitComplexBase<A>::value_type factor, SplitComplexBase<C>& dst)
{
    VCTR_ASSERT (src.size() == dst.size());

    dst.real() = src.real() * factor;
    dst.imag() = src.imag() * factor;
}

/** dst = src * factor, where factor is a complex scalar. */
template <class A, class C>
requires detail::splitcomplex::sameValueType<A, C>
void mulComplex (const SplitComplexBase<A>& src, std::complex<typename SplitComplexBase<A>::value_type> factor, SplitComplexBase<C>& dst)
{
    const auto fRe = factor.real();
    const auto fIm = factor.imag();

    detail::splitcomplex::evaluate (dst, [fRe, fIm] (auto sRe, auto sIm) {
        return std::pair { sRe * fRe - sIm * fIm, sRe * fIm + sIm * fRe };
    }, src);
}

/** dst = |src|^2 where dst is a real valued container. */
template <class A, class Dst>
requires is::anyVctr<Dst> && std::same_as<typename SplitComplexBase<A>::value_type, typename std::remove_cvref_t<Dst>::value_type>
void magnitudeSquared (const SplitComplexBase<A>& src, Dst&& dst)
{
    VCTR_ASSERT (src.size() == dst.size());

    const auto sRe = src.real();
    const auto sIm = src.imag();

    dst = sRe * sRe + sIm * sIm;
}

/** dst = |src| where dst is a real valued container. */
template <class A, class Dst>
requires is::anyVctr<Dst> && std::same_as<typename SplitComplexBase<A>::value_type, typename std::remove_cvref_t<Dst>::value_type>
void magnitude (const SplitComplexBase<A>& src, Dst&& dst)
{
    VCTR_ASSERT (src.size() == dst.size());

    const auto sRe = src.real();
    const auto sIm = src.imag();

    dst = vctr::sqrt << (sRe * sRe + sIm * sIm);
}

/** @} */

} // namespace vctr
