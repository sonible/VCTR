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

namespace vctr::expressions
{

template <size_t extent, class SrcType>
requires is::realFloatNumber<ValueType<SrcType>>
class SignedSqrt : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (SignedSqrt, src)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        const auto in = src[i];

        if (std::is_constant_evaluated())
            return in < value_type (0) ? -gcem::sqrt (-in) : gcem::sqrt (in);

        return std::copysign (std::sqrt (std::abs (in)), in);
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type>
    {
        const auto* s = src.evalNextVectorOpInExpressionChain (dst);

        if (s != dst)
        {
            const auto len = sizeToInt (size());
            Expression::Accelerate::abs (s, dst, len);
            Expression::Accelerate::sqrt (dst, dst, len);
            Expression::Accelerate::copysign (dst, s, dst, len);
            return dst;
        }

        // The vectorised approach does not work for in-place evaluation, so we fall back to a scalar implementation here
        for (size_t i = 0; i < size(); ++i)
            dst[i] = std::copysign (std::sqrt (std::abs (dst[i])), dst[i]);

        return dst;
    }
};

} // namespace vctr::expressions

namespace vctr
{

/** Computes the sign-preserving square root of the source values.

    Values >= 0 behave like a regular square root, values < 0 yield a negative square root of the absolute value.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::SignedSqrt> signedSqrt;

} // namespace vctr



