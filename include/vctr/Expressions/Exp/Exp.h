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

template <size_t extent, class SrcType>
requires is::realNumber<ValueType<SrcType>>
class Exp : ExpressionTemplateBase
{
    using SrcValueType = ValueType<SrcType>;

public:
    using value_type = std::conditional_t<is::intNumber<SrcValueType>, float, SrcValueType>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (Exp)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
#if VCTR_USE_GCEM
        if (std::is_constant_evaluated())
            return gcem::exp (src[i]);
#endif

        return std::exp (src[i]);
    }

    //==============================================================================
    // Platform Vector Operation Implementation
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable> && is::floatNumber<SrcValueType>
    {
        Expression::Accelerate::exp (src.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateRealIntToFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable> && is::int32Number<SrcValueType>
    {
        auto s = size();

        Expression::Accelerate::intToFloat (src.evalNextVectorOpInExpressionChain (reinterpret_cast<SrcValueType*> (dst)), dst, s);
        Expression::Accelerate::exp (dst, dst, sizeToInt (s));

        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppRealOrComplexFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        Expression::IPP::exp (src.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
        return dst;
    }

private:
    SrcType src;
};

/** Computes e (Euler's number, 2.7182818...) raised to the source vector elements power */
constexpr ExpressionChainBuilder<Exp> exp;

} // namespace vctr
