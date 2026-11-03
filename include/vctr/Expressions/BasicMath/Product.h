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

template <size_t extent, class SrcType>
requires is::number<ValueType<SrcType>>
class Product : public ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Product, src)

    static constexpr value_type reductionResultInitValue = 1;

    VCTR_FORCEDINLINE constexpr void reduceElementWise (value_type& result, size_t i) const
    {
        if constexpr (is::complexFloatNumber<value_type>)
        {
            if (std::is_constant_evaluated())
            {
                // Clang cannot constant-evaluate the built-in complex *= used by libstdc++.
                const auto rhs = src[i];
                const auto a = result.real(), b = result.imag();
                const auto c = rhs.real(), d = rhs.imag();
                result = value_type (a * c - b * d, a * d + b * c);
                return;
            }
        }

        result *= src[i];
    }

    //==============================================================================
    VCTR_FORWARD_PREPARE_SIMD_EVALUATION_UNARY_EXPRESSION_MEMBER_FUNCTIONS

    VCTR_FORCEDINLINE void reduceNeonRegisterWise (NeonRegister<value_type>& result, size_t i) const
    requires Config::archARM && has::getNeon<SrcType> && (is::realFloatNumber<value_type> || is::int32Number<value_type>)
    {
        result = Expression::Neon::mul (result, src.getNeon (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("fma") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getAVX<SrcType> && is::realFloatNumber<value_type>
    {
        result = Expression::AVX::mul (result, src.getAVX (i));
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const
    requires Config::archX64 && has::getSSE<SrcType> && is::realFloatNumber<value_type>
    {
        result = Expression::SSE::mul (result, src.getSSE (i));
    }

    //==============================================================================
    template <size_t n>
    VCTR_FORCEDINLINE static constexpr value_type finalizeReduction (const std::array<value_type, n>& products)
    {
        if constexpr (n == 1)
            return products[0];

        return std::reduce (products.begin(), products.end(), reductionResultInitValue, std::multiplies<>());
    }
};

} // namespace vctr::expressions

namespace vctr
{

/** Computes the product of the source values.

    Note that the product of a long sequence can quickly overflow integer types or under-/overflow
    floating point types. With SIMD acceleration, the multiplication order differs from a
    sequential evaluation, so floating point results might differ slightly.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::Product> product;

} // namespace vctr
