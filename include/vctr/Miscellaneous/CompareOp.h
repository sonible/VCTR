/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2026 - by sonible GmbH.

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

#if (! VCTR_X64)
// These are defined in immintrin.h – defining them here for non x86 builds
// to be able to assign the CompareOp enum cross-platform
// NOLINTBEGIN (*-reserved-identifier)
constexpr auto _CMP_LT_OQ = 0;
constexpr auto _CMP_LE_OQ = 1;
constexpr auto _CMP_GT_OQ = 2;
constexpr auto _CMP_GE_OQ = 3;
constexpr auto _CMP_EQ_OQ = 4;
constexpr auto _CMP_NEQ_OQ = 5;
// NOLINTEND (*-reserved-identifier)
#endif

/** Possible types of (SIMD) compare operations */
enum class CompareOp : int
{
    less = _CMP_LT_OQ,           // <
    lessOrEqual = _CMP_LE_OQ,    // <=
    greater = _CMP_GT_OQ,        // >
    greaterOrEqual = _CMP_GE_OQ, // >=
    equal = _CMP_EQ_OQ,          // ==
    notEqual = _CMP_NEQ_OQ       // !=
};

namespace is
{
// clang-format off
/** Constrains an instance of Lhs to be comparable by op with Rhs. */
template <class Lhs, CompareOp op, class Rhs>
concept comparableByWith = (op == CompareOp::less           && requires (const Lhs& lhs, const Rhs& rhs) { lhs <  rhs; }) ||
                           (op == CompareOp::lessOrEqual    && requires (const Lhs& lhs, const Rhs& rhs) { lhs <= rhs; }) ||
                           (op == CompareOp::greater        && requires (const Lhs& lhs, const Rhs& rhs) { lhs >  rhs; }) ||
                           (op == CompareOp::greaterOrEqual && requires (const Lhs& lhs, const Rhs& rhs) { lhs >= rhs; }) ||
                           (op == CompareOp::equal          && requires (const Lhs& lhs, const Rhs& rhs) { lhs == rhs; }) ||
                           (op == CompareOp::notEqual       && requires (const Lhs& lhs, const Rhs& rhs) { lhs != rhs; });
// clang-format on

/** Constrains two instances of T to be comparable by the operation specified by op. */
template <class T, CompareOp op>
concept comparableBy = comparableByWith<T, op, T>;
}

/** Helper struct for scalar fallback evaluation of compare operations specified via CompareOp constants.

    In most cases you probably want to use the scalarCompare function instead of instantiating this manually.

    @see scalarCompare
 */
template <CompareOp>
struct ScalarCompareOp {};

template<>
struct ScalarCompareOp<CompareOp::less>
{
    template <class Lhs, is::comparableByWith<CompareOp::less, Lhs> Rhs>
    constexpr bool operator() (const Lhs& lhs, const Rhs& rhs) const { return lhs < rhs; }
};

template<>
struct ScalarCompareOp<CompareOp::lessOrEqual>
{
    template <class Lhs, is::comparableByWith<CompareOp::lessOrEqual, Lhs> Rhs>
    constexpr bool operator() (const Lhs& lhs, const Rhs& rhs) const { return lhs <= rhs; }
};

template<>
struct ScalarCompareOp<CompareOp::greater>
{
    template <class Lhs, is::comparableByWith<CompareOp::greater, Lhs> Rhs>
    constexpr bool operator() (const Lhs& lhs, const Rhs& rhs) const { return lhs > rhs; }
};

template<>
struct ScalarCompareOp<CompareOp::greaterOrEqual>
{
    template <class Lhs, is::comparableByWith<CompareOp::greaterOrEqual, Lhs> Rhs>
    constexpr bool operator() (const Lhs& lhs, const Rhs& rhs) const { return lhs >= rhs; }
};

template<>
struct ScalarCompareOp<CompareOp::equal>
{
    template <class Lhs, is::comparableByWith<CompareOp::equal, Lhs> Rhs>
    constexpr bool operator() (const Lhs& lhs, const Rhs& rhs) const { return lhs == rhs; }
};

template<>
struct ScalarCompareOp<CompareOp::notEqual>
{
    template <class Lhs, is::comparableByWith<CompareOp::notEqual, Lhs> Rhs>
    constexpr bool operator() (const Lhs& lhs, const Rhs& rhs) const { return lhs != rhs; }
};

/** Compares two values using the compare operation specified by CompareOp. */
template <CompareOp op, class Lhs, is::comparableByWith<op, Lhs> Rhs>
constexpr bool scalarCompare (const Lhs& lhs, const Rhs& rhs)
{
    return ScalarCompareOp<op>{} (lhs, rhs);
}

}
