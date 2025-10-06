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

//==============================================================================
/** Multiply-Accumulates three vector like types. */
template <size_t extent, class SrcAType, class SrcBType, class SrcCType>
requires are::same<ValueType<SrcAType>, ValueType<SrcBType>, ValueType<SrcCType>>
class MultiplyAccumulateVectors : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcAType>;

private:
    using Expression = ExpressionTypes<value_type, SrcAType, SrcBType, SrcCType>;

    using SrcAStorageInfoType = std::invoke_result_t<decltype (&std::remove_cvref_t<SrcAType>::getStorageInfo), SrcAType>;
    using SrcBStorageInfoType = std::invoke_result_t<decltype (&std::remove_cvref_t<SrcBType>::getStorageInfo), SrcBType>;
    using SrcCStorageInfoType = std::invoke_result_t<decltype (&std::remove_cvref_t<SrcCType>::getStorageInfo), SrcCType>;

    SrcAType srcA;
    SrcBType srcB;
    SrcCType srcC;

    const CombinedStorageInfo<std::remove_cvref_t<SrcAStorageInfoType>, std::remove_cvref_t<SrcBStorageInfoType>, std::remove_cvref_t<SrcCStorageInfoType>> storageInfo;

public:
    template <class SrcA, class SrcB, class SrcC>
    constexpr MultiplyAccumulateVectors (SrcA&& a, SrcB&& b, SrcC&& c)
        : srcA (std::forward<SrcA> (a)),
          srcB (std::forward<SrcB> (b)),
          srcC (std::forward<SrcC> (c)),
          storageInfo (srcA.getStorageInfo(), srcB.getStorageInfo(), srcC.getStorageInfo())
    {}

    constexpr const auto& getStorageInfo() const { return storageInfo; }

    constexpr size_t size() const { return srcA.size(); }

    constexpr bool isNotAliased (const void*) const
    {
        return false;
    }

    VCTR_FORCEDINLINE constexpr auto operator[] (size_t i) const
    {
        return srcA[i] * srcB[i] + srcC[i];
    }

    //==============================================================================
    void prepareNeonEvaluation() const
    requires ::vctr::has::prepareNeonEvaluation<SrcAType> && ::vctr::has::prepareNeonEvaluation<SrcBType> && ::vctr::has::prepareNeonEvaluation<SrcCType>
    {
        srcA.prepareNeonEvaluation();
        srcB.prepareNeonEvaluation();
        srcC.prepareNeonEvaluation();
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void prepareAVXEvaluation() const
    requires ::vctr::has::prepareAVXEvaluation<SrcAType> && ::vctr::has::prepareAVXEvaluation<SrcBType> && ::vctr::has::prepareAVXEvaluation<SrcCType>
    {
        srcA.prepareAVXEvaluation();
        srcB.prepareAVXEvaluation();
        srcC.prepareAVXEvaluation();
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void prepareSSEEvaluation() const
    requires ::vctr::has::prepareSSEEvaluation<SrcAType> && ::vctr::has::prepareSSEEvaluation<SrcBType> && ::vctr::has::prepareSSEEvaluation<SrcCType>
    {
        srcA.prepareSSEEvaluation();
        srcB.prepareSSEEvaluation();
        srcC.prepareSSEEvaluation();
    }

    // AVX Implementation
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") AVXRegister<value_type> getAVX (size_t i) const
    requires (archX64 && has::getAVX<SrcAType> && has::getAVX<SrcBType> && Expression::allElementTypesSame && Expression::CommonElement::isRealFloat)
    {
        return Expression::AVX::fma (srcA.getAVX (i), srcB.getAVX (i), srcC.getAVX (i));
    }

    //==============================================================================
    // NEON Implementation
    NeonRegister<value_type> getNeon (size_t i) const
    requires (archARM && has::getNeon<SrcAType> && has::getNeon<SrcBType> && has::getNeon<SrcCType> && Expression::allElementTypesSame)
    {
        return Expression::Neon::fma (srcA.getNeon (i), srcB.getNeon (i), srcC.getNeon (i));
    }
};

} // namespace vctr::expressions

namespace vctr
{

/** Computes the multiply-accumulate operation (a * b) + c.

    This is usually faster than evaluating these operations individually as it exploits
    special hardware instructions. Prefer this for performance critical DSP implementations.

    @ingroup Expressions
 */
template <class SrcAType, class SrcBType, class SrcCType>
requires (is::anyVctrOrExpression<std::remove_cvref_t<SrcAType>> &&
          is::anyVctrOrExpression<std::remove_cvref_t<SrcBType>> &&
          is::anyVctrOrExpression<std::remove_cvref_t<SrcCType>>)
constexpr auto multiplyAccumulate (SrcAType&& a, SrcBType&& b, SrcCType&& c)
{
    assertCommonSize (a, b, c);
    constexpr auto extent = getCommonExtent<SrcAType, SrcBType, SrcCType>();

    return expressions::MultiplyAccumulateVectors<extent, SrcAType, SrcBType, SrcCType> (std::forward<SrcAType> (a), std::forward<SrcBType> (b), std::forward<SrcCType> (c));
}
} // namespace vctr
