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

/** Helper struct to pass multiple partly non-type template parameters to a BinaryCaseDistinction expression.
 
    @tparam caseTrueExpression  The ExpressionChainBuilder instance to use if comparing the input to the threshold using op compares to true
    @tparam op                  The compare operation to evaluate. The binary expression will evaluate "input op threshold"
    @tparam Threshold           The threshold constant to use as rhs argument to the compare operation
    @tparam caseFalseExpression The ExpressionChainBuilder instance to use if comparing the input to the threshold using op compares to false
 */
template <const auto& caseTrueExpression, CompareOp op, is::constant Threshold, const auto& caseFalseExpression>
struct BinaryCaseDistinctionConfig
{
    static constexpr auto& caseTrue = caseTrueExpression;
    static constexpr auto& caseFalse = caseFalseExpression;

    static constexpr auto compareOp = op;

    using ThresholdValue = Threshold;
};
}

namespace vctr::is
{

/** Constrains T to be any instance of BinaryCaseDistinctionConfig. */
template <class T>
concept binaryCaseDistinctionConfig = requires (const T& t) { [] <const auto& b, CompareOp op, class Th, const auto& a> (const BinaryCaseDistinctionConfig<b, op, Th, a>&) {} (t); };

}

namespace vctr::expressions
{
template <size_t extent, class SrcType, is::binaryCaseDistinctionConfig CaseConfig>
requires is::comparableBy<ValueType<SrcType>, CaseConfig::compareOp>
class BinaryCaseDistinction : vctr::ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;

private:
    using Expression = ExpressionTypes<value_type, SrcType>;
    SrcType src;

    using CaseTrueExpressionChainBuilder = decltype (CaseConfig::caseTrue << std::declval<SrcType>());
    using CaseFalseExpressionChainBuilder = decltype (CaseConfig::caseFalse << std::declval<SrcType>());

    CaseTrueExpressionChainBuilder caseTrue;
    CaseFalseExpressionChainBuilder caseFalse;

    static constexpr value_type threshold = CaseConfig::ThresholdValue::value;
    mutable SIMDRegisterUnion<Expression> thresholdSIMDConst;

public:
    template <class Src>
    constexpr BinaryCaseDistinction (Src&& s)
        : src (std::forward<Src>(s)),
          caseTrue (CaseConfig::caseTrue << src),
          caseFalse (CaseConfig::caseFalse << src)
    {
        // Any expression altering the size of the input won't work here
        VCTR_ASSERT (caseTrue.size() == src.size());
        VCTR_ASSERT (caseFalse.size() == src.size());
    }

    constexpr const auto& getStorageInfo() const
    {
        VCTR_ASSERT (&caseTrue.getStorageInfo() == &src.getStorageInfo());
        VCTR_ASSERT (&caseFalse.getStorageInfo() == &src.getStorageInfo());

        return src.getStorageInfo();
    }

    constexpr bool isNotAliased (const void *other) const
    {
        // Optimization potential: Probably both cases are unary so no way for them to be aliased. How to assert that property?
        return caseTrue.isNotAliased (other) && caseFalse.isNotAliased (other) && src.isNotAliased (other);
    }

    constexpr size_t size() const { return src.size(); }

    template<size_t i, class RuntimeArgs>
    constexpr void iterateOverRuntimeArgChain(const RuntimeArgs &rtArgs) {
        tryApplyingRuntimeArgsToThisExpression<i>(rtArgs, *this);
        tryApplyingRuntimeArgsToSrc<i + 1>(rtArgs, src);
    }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return scalarCompare<CaseConfig::compareOp> (src[i], threshold) ? caseTrue[i] : caseFalse[i];
    }

    //==============================================================================

    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void prepareAVXEvaluation() const
    requires has::prepareAVXEvaluation<SrcType> && has::prepareAVXEvaluation<CaseTrueExpressionChainBuilder> && has::prepareAVXEvaluation<CaseFalseExpressionChainBuilder>
    {
        src.prepareAVXEvaluation();
        caseTrue.prepareAVXEvaluation();
        caseFalse.prepareAVXEvaluation();

        thresholdSIMDConst.avx = Expression::AVX::broadcast (threshold);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") AVXRegister<value_type> getAVX (size_t i) const
    requires archX64 && has::getAVX<SrcType> && has::getAVX<CaseTrueExpressionChainBuilder> && has::getAVX<CaseFalseExpressionChainBuilder> && Expression::CommonElement::isInt
    {
        auto caseTrueResult = caseTrue.getAVX (i);
        auto caseFalseResult = caseFalse.getAVX (i);

        auto compareResult = Expression::AVX::template compare<CaseConfig::compareOp> (src.getAVX (i), thresholdSIMDConst.avx);

        return Expression::AVX::bitwiseBlend (caseFalseResult, caseTrueResult, compareResult);
    }

    VCTR_FORCEDINLINE VCTR_TARGET ("fma") AVXRegister<value_type> getAVX (size_t i) const
    requires archX64 && has::getAVX<SrcType> && has::getAVX<CaseTrueExpressionChainBuilder> && has::getAVX<CaseFalseExpressionChainBuilder> && Expression::CommonElement::isRealFloat
    {
        auto caseTrueResult = caseTrue.getAVX (i);
        auto caseFalseResult = caseFalse.getAVX (i);

        auto compareResult = Expression::AVX::template compare<CaseConfig::compareOp> (src.getAVX (i), thresholdSIMDConst.avx);

        return Expression::AVX::bitwiseBlend (caseFalseResult, caseTrueResult, compareResult);
    }

    // Todo: Currently no compare operation for SSE implemented, so no SSE version implented here

    VCTR_FORCEDINLINE void prepareNeonEvaluation() const
    requires has::prepareNeonEvaluation<SrcType> && has::prepareNeonEvaluation<CaseTrueExpressionChainBuilder> && has::prepareNeonEvaluation<CaseFalseExpressionChainBuilder>
    {
        src.prepareNeonEvaluation();
        caseTrue.prepareNeonEvaluation();
        caseFalse.prepareNeonEvaluation();

        thresholdSIMDConst.neon = Expression::Neon::broadcast (threshold);
    }

    VCTR_FORCEDINLINE NeonRegister<value_type> getNeon (size_t i) const
    requires archARM && has::getNeon<SrcType> && has::getNeon<CaseTrueExpressionChainBuilder> && has::getNeon<CaseFalseExpressionChainBuilder>
    {
        auto caseTrueResult = caseTrue.getNeon (i);
        auto caseFalseResult = caseFalse.getNeon (i);

        auto compareResult = Expression::Neon::template compare<CaseConfig::compareOp> (src.getNeon (i), thresholdSIMDConst.neon);

        return Expression::Neon::bitwiseBlend (caseFalseResult, caseTrueResult, compareResult);
    }
};
}

namespace vctr
{
/** Evaluates one of two expressions depending on a binary condition.

    The comparison of the input value with the threshold using the specified compare operation decides which one is
    evaluated.

    The expression will prepend the source to both caseTrueExpression and caseFalseExpression internally. If a source
    element compares true to threshold using compareOp, the result element will be the evaluation result of
    caseTrueExpression, otherwise caseFalseExpression. If you only want to apply an expression to either the true or
    false path, consider passing vctr::forward as expression to the other path.

    @note: Since this expression evaluates both expression paths and the compare operation this leads to possibly
           multiple accesses of the source. Therefore, it's good advice to only use containers rather than unevaluated
           expressions as source to this expression to avoid multiple evaluations of the same expression.

    @ingroup Expressions
 */
template <const auto& caseTrueExpression, CompareOp compareOp, const auto& threshold, const auto& caseFalseExpression>
constexpr inline ExpressionChainBuilder<expressions::BinaryCaseDistinction, BinaryCaseDistinctionConfig<caseTrueExpression, compareOp, ConstantRef<threshold>, caseFalseExpression>> binaryCaseDistinction;
}

