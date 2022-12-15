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

class ReductionExpression
{
public:
    template <is::reductionExpression Expression>
    VCTR_FORCEDINLINE static auto reduce (const Expression& e)
    {
        const auto s = e.size();

        // Reducing an empty vector can lead to undefined edge cases
        VCTR_ASSERT (s > 0);

        if constexpr (has::reduceVectorOp<Expression, ValueType<Expression>>)
        {
            return e.reduceVectorOp();
        }

        if constexpr (has::reduceNeonRegisterWise<Expression, ValueType<Expression>>)
        {
            return reduceNeon (e);
        }

        if constexpr (has::reduceAVXRegisterWise<Expression, ValueType<Expression>>)
        {
            if constexpr (is::floatNumber<ValueType<Expression>>)
            {
                if (Config::supportsAVX)
                    return reduceAVX (e);
            }
            else
            {
                if (Config::supportsAVX2)
                    return reduceAVX2 (e);
            }
        }

        if constexpr (has::reduceSSERegisterWise<Expression, ValueType<Expression>>)
        {
            if (Config::highestSupportedCPUInstructionSet != CPUInstructionSet::fallback)
                return reduceSSE (e);
        }

        ValueType<Expression> v = Expression::reductionResultInitValue;
        for (size_t i = 0; i < s; ++i)
            e.reduceElementWise (v, i);

        return v;
    }

private:
    template <is::reductionExpression Expression>
    VCTR_TARGET ("avx2") static auto reduceAVX2 (const Expression& e)
    requires Config::archX64
    {
        using VType = ValueType<Expression>;
        using RType = AVXRegister<VType>;

        constexpr auto inc = RType::numElements;
        const auto n = e.size();
        const auto nSIMD = detail::previousMultipleOf<inc> (n);

        auto avxValue = RType::broadcast (Expression::reductionResultInitValue);

        size_t i = 0;
        for (; i < nSIMD; i += inc)
            e.reduceAVXRegisterWise (avxValue, i);

        alignas (Config::maxSIMDRegisterSize) std::array<VType, RType::numElements + 1> results;
        avxValue.storeAligned (results.data());
        results.back() = Expression::reductionResultInitValue;

        for (; i < n; ++i)
            e.reduceElementWise (results.back(), i);

        return Expression::finalizeSIMDReduction (results);
    }

    template <is::reductionExpression Expression>
    VCTR_TARGET ("avx") static auto reduceAVX (const Expression& e)
    requires Config::archX64
    {
        using VType = ValueType<Expression>;
        using RType = AVXRegister<VType>;

        constexpr auto inc = RType::numElements;
        const auto n = e.size();
        const auto nSIMD = detail::previousMultipleOf<inc> (n);

        auto avxValue = RType::broadcast (Expression::reductionResultInitValue);

        size_t i = 0;
        for (; i < nSIMD; i += inc)
            e.reduceAVXRegisterWise (avxValue, i);

        alignas (Config::maxSIMDRegisterSize) std::array<VType, RType::numElements + 1> results;
        avxValue.storeAligned (results.data());
        results.back() = Expression::reductionResultInitValue;

        for (; i < n; ++i)
            e.reduceElementWise (results.back(), i);

        return Expression::finalizeSIMDReduction (results);
    }

    template <is::reductionExpression Expression>
    VCTR_TARGET ("sse4.1") static auto reduceSSE (const Expression& e)
    requires Config::archX64
    {
        using VType = ValueType<Expression>;
        using RType = SSERegister<VType>;

        constexpr auto inc = RType::numElements;
        const auto n = e.size();
        const auto nSIMD = detail::previousMultipleOf<inc> (n);

        auto sseValue = RType::broadcast (Expression::reductionResultInitValue);

        size_t i = 0;
        for (; i < nSIMD; i += inc)
            e.reduceSSERegisterWise (sseValue, i);

        alignas (Config::maxSIMDRegisterSize) std::array<VType, RType::numElements + 1> results;
        sseValue.storeAligned (results.data());
        results.back() = Expression::reductionResultInitValue;

        for (; i < n; ++i)
            e.reduceElementWise (results.back(), i);

        return Expression::finalizeSIMDReduction (results);
    }

    template <is::reductionExpression Expression>
    static auto reduceNeon (const Expression& e)
    requires Config::archARM
    {
        using VType = ValueType<Expression>;
        using RType = NeonRegister<VType>;

        constexpr auto inc = RType::numElements;
        const auto n = e.size();
        const auto nSIMD = detail::previousMultipleOf<inc> (n);

        auto sseValue = RType::broadcast (Expression::reductionResultInitValue);

        size_t i = 0;
        for (; i < nSIMD; i += inc)
            e.reduceNeonRegisterWise (sseValue, i);

        alignas (Config::maxSIMDRegisterSize) std::array<VType, RType::numElements + 1> results;
        sseValue.store (results.data());
        results.back() = Expression::reductionResultInitValue;

        for (; i < n; ++i)
            e.reduceElementWise (results.back(), i);

        return Expression::finalizeSIMDReduction (results);
    }
};

} // namespace vctr
