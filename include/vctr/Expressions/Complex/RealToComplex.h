/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2023 - by sonible GmbH.

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
  requires is::realNumber<ValueType<SrcType>>
  class RealToComplex : public ExpressionTemplateBase
  {
  public:
      using value_type = std::complex<ValueType<SrcType>>;

      VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (RealToComplex, src)

      VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
      {
          return std::complex (src[i], ValueType<SrcType> (0));
      }

      VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
      requires is::suitableForAccelerateComplexFloatVectorOp<SrcType, value_type>
      {
          std::fill (dst, dst + size(), value_type (0));
          Expression::Accelerate::copyRealToComplex (src.data(), dst, sizeToInt (size()));
          return dst;
      }
  };

} // namespace vctr::expressions

namespace vctr
{
/** Converts a sequence of real numbers into a sequence of complex numbers with their imaginary part set to 0.

    @ingroup Expressions
*/
constexpr inline ExpressionChainBuilder<expressions::RealToComplex> realToComplex;

} // namespace vctr
