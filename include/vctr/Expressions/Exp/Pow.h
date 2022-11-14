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

template <size_t extent, class SrcType, is::constantWrapper ConstantType>
class PowConstantBase : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

    static constexpr value_type base = ConstantType::value;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (PowConstantBase)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
#if VCTR_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::pow (base, src[i]);
#endif

        return std::pow (base, src[i]);
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type>
    {
        // Todo: Quick workaround. Optimise it – maybe based on e.g. exp2 (log2 (base) * src)
        const auto s = size();
        const auto* x = src.evalNextVectorOpInExpressionChain (dst);

        for (size_t i = 0; i < s; ++i)
            dst[i] = std::pow (base, x[i]);

        return dst;
    }

private:
    SrcType src;
};

/** Evaluates base raised to the power of the source vector */
template <auto base>
constexpr ExpressionChainBuilder<PowConstantBase, Constant<base>> powConstantBase;

} // namespace vctr
