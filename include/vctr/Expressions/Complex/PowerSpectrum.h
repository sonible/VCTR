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
  requires is::complexFloatNumber<ValueType<SrcType>>
  class PowerSpectrum : public ExpressionTemplateBase
  {
  public:
      using value_type = RealType<ValueType<SrcType>>;

      VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (PowerSpectrum, src)

      VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
      {
          return std::norm (src[i]);
      }

      VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
      requires is::suitableForAccelerateComplexToRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
      {
          Expression::Accelerate::powerSpectrum (src.data(), dst, sizeToInt (size()));
          return dst;
      }

      VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
      requires is::suitableForIppComplexToRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
      {
          Expression::IPP::powerSpectrum (src.data(), dst, sizeToInt (size()));
          return dst;
      }
  };

} // namespace vctr::expressions

namespace vctr
{
/** Computes the power spectrum of the complex source values.

    The power spectrum is defined as the sum of the square of real and imaginary
    part like

    res[i] = (in_real[i]) ^ 2 + (in_imag[i]) ^ 2

    In case you want to chain more computationally expensive expressions on the
    complex source values before this expression, consider buffering the complex
    results to an intermediate complex container for better performance.

    @ingroup Expressions
*/
constexpr inline ExpressionChainBuilder<expressions::PowerSpectrum> powerSpectrum;

} // namespace vctr
