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

template <size_t extent, class SrcAType, class SrcBType>
requires is::realFloatNumber<ValueType<SrcAType>> && std::same_as<ValueType<SrcAType>, ValueType<SrcBType>>
class RealImagToComplex : public ExpressionTemplateBase
{
public:
    using value_type = std::complex<ValueType<SrcAType>>;

    VCTR_COMMON_BINARY_VEC_VEC_EXPRESSION_MEMBERS (RealImagToComplex, real, imag)

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return { real[i], imag[i] };
    }

    //==============================================================================
    // Platform Vector Operation Implementation. The real sources can't be evaluated into the complex destination, so the
    // concepts only allow containers as sources. Expression::Accelerate / Expression::IPP refer to the real source type,
    // the functions live in the complex one.
    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForAccelerateComplexFloatBinaryVectorOp<SrcAType, SrcBType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
    {
        platform_vec_ops::AppleAccelerate<value_type>::realImagToComplex (real.data(), imag.data(), dst, size());
        return dst;
    }

    VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
    requires is::suitableForIppComplexFloatBinaryVectorOp<SrcAType, SrcBType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
    {
        platform_vec_ops::IntelIPP<value_type>::realImagToComplex (real.data(), imag.data(), dst, sizeToInt (size()));
        return dst;
    }
};

} // namespace vctr::expressions

namespace vctr
{
/** Combines a sequence of real parts and a sequence of imaginary parts into a sequence of complex numbers.

    When both sources are containers, Apple Accelerate (vDSP_ztoc) resp. Intel IPP (ippsRealToCplx) is used.

    @ingroup Expressions
*/
template <is::anyVctrOrExpression RealType, is::anyVctrOrExpression ImagType>
constexpr auto realImagToComplex (RealType&& real, ImagType&& imag)
{
    assertCommonSize (real, imag);
    constexpr auto extent = getCommonExtent<RealType, ImagType>();
    return expressions::RealImagToComplex<extent, RealType, ImagType> (std::forward<RealType> (real), std::forward<ImagType> (imag));
}
} // namespace vctr
