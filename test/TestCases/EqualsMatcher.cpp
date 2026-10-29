/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2026 by sonible GmbH.

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

#include <vctr_test_utils/vctr_test_common.h>
#include <catch2/generators/catch_generators.hpp>

namespace
{
/** Checks element-wise that every value of actual is within an absolute margin of the corresponding value in reference. */
template <class Vec>
bool allWithinAbs (const Vec& actual, const Vec& reference, double margin)
{
    if (actual.size() != reference.size())
        return false;

    for (size_t i = 0; i < actual.size(); ++i)
    {
        if (! Catch::Matchers::WithinAbs (double (reference[i]), margin).match (double (actual[i])))
            return false;
    }

    return true;
}

/** Checks element-wise that every value of actual is within a relative margin of the corresponding value in reference. */
template <class Vec>
bool allWithinRel (const Vec& actual, const Vec& reference, double epsilon)
{
    if (actual.size() != reference.size())
        return false;

    for (size_t i = 0; i < actual.size(); ++i)
    {
        if (! Catch::Matchers::WithinRel (double (reference[i]), epsilon).match (double (actual[i])))
            return false;
    }

    return true;
}

/** A simple unary transform usable as the fn template argument of vctr::EqualsTransformedBy. */
template <class T>
T timesTwo (T x)
{
    return x * T (2);
}
} // namespace

TEMPLATE_TEST_CASE ("vctr::Equals matches element-wise WithinAbs/WithinRel for floating point element types", "[VCTR][Matchers][Equals]", float, double)
{
    using T = TestType;

    const vctr::Vector<T> reference { T (1), T (-2), T (3), T (-4), T (5) };

    SECTION ("WithinAbs")
    {
        constexpr auto margin = 0.2;

        // WithinAbs compares |actual - target| against margin directly, so probing margin plus/minus a tiny
        // offset puts the candidates right at the inclusive/exclusive edge of the boundary.
        constexpr auto edgeOffset = margin * 0.01;

        auto withinMargin = reference;
        withinMargin[1] += T (margin - edgeOffset);

        auto outsideMargin = reference;
        outsideMargin[1] += T (margin + edgeOffset);

        auto sizeMismatch = reference;
        sizeMismatch.push_back (T (0));

        auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
            { reference, true },
            { withinMargin, true },
            { outsideMargin, false },
            { sizeMismatch, false }
        }));

        const bool equalsResult = vctr::Equals (reference).withMargin (margin).match (candidate);
        const bool manualResult = allWithinAbs (candidate, reference, margin);

        CHECK (equalsResult == expected);
        CHECK (manualResult == expected);
        CHECK (equalsResult == manualResult);
    }

    SECTION ("WithinRel")
    {
        constexpr auto epsilon = 0.1;

        // WithinRel compares |actual - target| against epsilon * max (|actual|, |target|). Shrinking the
        // candidate's magnitude towards zero (rather than growing it) keeps |target| as the larger operand,
        // which gives an implementation-independent boundary distance of exactly epsilon * |target|. Probing
        // epsilon plus/minus a tiny offset therefore puts the candidates right at the matcher's boundary.
        constexpr auto edgeOffset = epsilon * 0.01;

        auto withinEpsilon = reference;
        withinEpsilon[1] *= T (1.0 - (epsilon - edgeOffset));

        auto outsideEpsilon = reference;
        outsideEpsilon[1] *= T (1.0 - (epsilon + edgeOffset));

        auto sizeMismatch = reference;
        sizeMismatch.push_back (T (0));

        auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
            { reference, true },
            { withinEpsilon, true },
            { outsideEpsilon, false },
            { sizeMismatch, false }
        }));

        const bool equalsResult = vctr::Equals (reference).withEpsilon (epsilon).match (candidate);
        const bool manualResult = allWithinRel (candidate, reference, epsilon);

        CHECK (equalsResult == expected);
        CHECK (manualResult == expected);
        CHECK (equalsResult == manualResult);
    }

    SECTION ("Length-one vector")
    {
        const vctr::Vector<T> singleRef { T (7) };

        constexpr auto margin = 0.2;
        constexpr auto epsilon = 0.1;
        constexpr auto marginEdgeOffset = margin * 0.01;
        constexpr auto epsilonEdgeOffset = epsilon * 0.01;

        auto withinMargin = singleRef;
        withinMargin[0] += T (margin - marginEdgeOffset);

        auto outsideMargin = singleRef;
        outsideMargin[0] += T (margin + marginEdgeOffset);

        auto withinEpsilon = singleRef;
        withinEpsilon[0] *= T (1.0 - (epsilon - epsilonEdgeOffset));

        auto outsideEpsilon = singleRef;
        outsideEpsilon[0] *= T (1.0 - (epsilon + epsilonEdgeOffset));

        SECTION ("WithinAbs")
        {
            auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
                { singleRef, true },
                { withinMargin, true },
                { outsideMargin, false }
            }));

            const bool equalsResult = vctr::Equals (singleRef).withMargin (margin).match (candidate);
            const bool manualResult = allWithinAbs (candidate, singleRef, margin);

            CHECK (equalsResult == expected);
            CHECK (manualResult == expected);
            CHECK (equalsResult == manualResult);
        }

        SECTION ("WithinRel")
        {
            auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
                { singleRef, true },
                { withinEpsilon, true },
                { outsideEpsilon, false }
            }));

            const bool equalsResult = vctr::Equals (singleRef).withEpsilon (epsilon).match (candidate);
            const bool manualResult = allWithinRel (candidate, singleRef, epsilon);

            CHECK (equalsResult == expected);
            CHECK (manualResult == expected);
            CHECK (equalsResult == manualResult);
        }
    }
}

TEMPLATE_TEST_CASE ("vctr::Equals matches element-wise WithinAbs/WithinRel for exact-comparison element types", "[VCTR][Matchers][Equals]", int, unsigned int, bool)
{
    using T = TestType;

    // bool, int and unsigned int are not realFloatNumber, so vctr::Equals ignores margin/epsilon for them and
    // always requires exact equality. A margin/epsilon of 0 makes WithinAbs/WithinRel behave the same way,
    // so comparing against them here still verifies that vctr::Equals' behaviour lines up with the matchers.
    const vctr::Vector<T> reference = [] () -> vctr::Vector<T>
    {
        if constexpr (std::same_as<T, bool>)
            return { true, false, true, false, true };
        else
            return { T (1), T (2), T (3), T (4), T (5) };
    }();

    auto differing = reference;
    differing[1] = differing[1] ? T (0) : T (1);

    auto sizeMismatch = reference;
    sizeMismatch.push_back (reference.back());

    SECTION ("WithinAbs")
    {
        constexpr auto margin = 0.0;

        auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
            { reference, true },
            { differing, false },
            { sizeMismatch, false }
        }));

        const bool equalsResult = vctr::Equals (reference).withMargin (margin).match (candidate);
        const bool manualResult = allWithinAbs (candidate, reference, margin);

        CHECK (equalsResult == expected);
        CHECK (manualResult == expected);
        CHECK (equalsResult == manualResult);
    }

    SECTION ("WithinRel")
    {
        constexpr auto epsilon = 0.0;

        auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
            { reference, true },
            { differing, false },
            { sizeMismatch, false }
        }));

        const bool equalsResult = vctr::Equals (reference).withEpsilon (epsilon).match (candidate);
        const bool manualResult = allWithinRel (candidate, reference, epsilon);

        CHECK (equalsResult == expected);
        CHECK (manualResult == expected);
        CHECK (equalsResult == manualResult);
    }

    SECTION ("Length-one vector")
    {
        const vctr::Vector<T> singleRef { reference[0] };

        auto singleDiffering = singleRef;
        singleDiffering[0] = singleDiffering[0] ? T (0) : T (1);

        auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
            { singleRef, true },
            { singleDiffering, false }
        }));

        const bool equalsAbsResult = vctr::Equals (singleRef).withMargin (0.0).match (candidate);
        const bool manualAbsResult = allWithinAbs (candidate, singleRef, 0.0);

        const bool equalsRelResult = vctr::Equals (singleRef).withEpsilon (0.0).match (candidate);
        const bool manualRelResult = allWithinRel (candidate, singleRef, 0.0);

        CHECK (equalsAbsResult == expected);
        CHECK (manualAbsResult == expected);
        CHECK (equalsAbsResult == manualAbsResult);

        CHECK (equalsRelResult == expected);
        CHECK (manualRelResult == expected);
        CHECK (equalsRelResult == manualRelResult);
    }
}

TEMPLATE_TEST_CASE ("vctr::EqualsTransformedBy matches element-wise WithinAbs/WithinRel for floating point element types", "[VCTR][Matchers][EqualsTransformedBy]", float, double)
{
    using T = TestType;

    const vctr::Vector<T> reference { T (1), T (-2), T (3), T (-4), T (5) };

    vctr::Vector<T> transformedReference (reference.size());
    std::transform (reference.begin(), reference.end(), transformedReference.begin(), timesTwo<T>);

    SECTION ("WithinAbs")
    {
        constexpr auto margin = 0.2;

        // WithinAbs compares |actual - target| against margin directly, so probing margin plus/minus a tiny
        // offset puts the candidates right at the inclusive/exclusive edge of the boundary.
        constexpr auto edgeOffset = margin * 0.01;

        auto withinMargin = transformedReference;
        withinMargin[1] += T (margin - edgeOffset);

        auto outsideMargin = transformedReference;
        outsideMargin[1] += T (margin + edgeOffset);

        auto sizeMismatch = transformedReference;
        sizeMismatch.push_back (T (0));

        auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
            { transformedReference, true },
            { withinMargin, true },
            { outsideMargin, false },
            { sizeMismatch, false }
        }));

        const bool equalsResult = vctr::EqualsTransformedBy<timesTwo<T>> (reference).withMargin (margin).match (candidate);
        const bool manualResult = allWithinAbs (candidate, transformedReference, margin);

        CHECK (equalsResult == expected);
        CHECK (manualResult == expected);
        CHECK (equalsResult == manualResult);
    }

    SECTION ("WithinRel")
    {
        constexpr auto epsilon = 0.1;

        // WithinRel compares |actual - target| against epsilon * max (|actual|, |target|). Shrinking the
        // candidate's magnitude towards zero (rather than growing it) keeps |target| as the larger operand,
        // which gives an implementation-independent boundary distance of exactly epsilon * |target|. Probing
        // epsilon plus/minus a tiny offset therefore puts the candidates right at the matcher's boundary.
        constexpr auto edgeOffset = epsilon * 0.01;

        auto withinEpsilon = transformedReference;
        withinEpsilon[1] *= T (1.0 - (epsilon - edgeOffset));

        auto outsideEpsilon = transformedReference;
        outsideEpsilon[1] *= T (1.0 - (epsilon + edgeOffset));

        auto sizeMismatch = transformedReference;
        sizeMismatch.push_back (T (0));

        auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
            { transformedReference, true },
            { withinEpsilon, true },
            { outsideEpsilon, false },
            { sizeMismatch, false }
        }));

        const bool equalsResult = vctr::EqualsTransformedBy<timesTwo<T>> (reference).withEpsilon (epsilon).match (candidate);
        const bool manualResult = allWithinRel (candidate, transformedReference, epsilon);

        CHECK (equalsResult == expected);
        CHECK (manualResult == expected);
        CHECK (equalsResult == manualResult);
    }

    SECTION ("Length-one vector")
    {
        const vctr::Vector<T> singleRef { T (7) };

        vctr::Vector<T> singleTransformedRef { timesTwo (singleRef[0]) };

        constexpr auto margin = 0.2;
        constexpr auto epsilon = 0.1;
        constexpr auto marginEdgeOffset = margin * 0.01;
        constexpr auto epsilonEdgeOffset = epsilon * 0.01;

        auto withinMargin = singleTransformedRef;
        withinMargin[0] += T (margin - marginEdgeOffset);

        auto outsideMargin = singleTransformedRef;
        outsideMargin[0] += T (margin + marginEdgeOffset);

        auto withinEpsilon = singleTransformedRef;
        withinEpsilon[0] *= T (1.0 - (epsilon - epsilonEdgeOffset));

        auto outsideEpsilon = singleTransformedRef;
        outsideEpsilon[0] *= T (1.0 - (epsilon + epsilonEdgeOffset));

        SECTION ("WithinAbs")
        {
            auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
                { singleTransformedRef, true },
                { withinMargin, true },
                { outsideMargin, false }
            }));

            const bool equalsResult = vctr::EqualsTransformedBy<timesTwo<T>> (singleRef).withMargin (margin).match (candidate);
            const bool manualResult = allWithinAbs (candidate, singleTransformedRef, margin);

            CHECK (equalsResult == expected);
            CHECK (manualResult == expected);
            CHECK (equalsResult == manualResult);
        }

        SECTION ("WithinRel")
        {
            auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
                { singleTransformedRef, true },
                { withinEpsilon, true },
                { outsideEpsilon, false }
            }));

            const bool equalsResult = vctr::EqualsTransformedBy<timesTwo<T>> (singleRef).withEpsilon (epsilon).match (candidate);
            const bool manualResult = allWithinRel (candidate, singleTransformedRef, epsilon);

            CHECK (equalsResult == expected);
            CHECK (manualResult == expected);
            CHECK (equalsResult == manualResult);
        }
    }
}

TEMPLATE_TEST_CASE ("vctr::EqualsTransformedBy matches element-wise WithinAbs/WithinRel for exact-comparison element types", "[VCTR][Matchers][EqualsTransformedBy]", int32_t, uint32_t)
{
    using T = TestType;

    const vctr::Vector<T> reference { T (1), T (2), T (3), T (4), T (5) };

    vctr::Vector<T> transformedReference (reference.size());
    std::transform (reference.begin(), reference.end(), transformedReference.begin(), timesTwo<T>);

    // T is not realFloatNumber, so vctr::EqualsTransformedBy ignores margin/epsilon for it and always requires
    // exact equality, just like vctr::Equals does.
    auto differing = transformedReference;
    differing[1] += T (1);

    auto sizeMismatch = transformedReference;
    sizeMismatch.push_back (transformedReference.back());

    SECTION ("WithinAbs")
    {
        constexpr auto margin = 0.0;

        auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
            { transformedReference, true },
            { differing, false },
            { sizeMismatch, false }
        }));

        const bool equalsResult = vctr::EqualsTransformedBy<timesTwo<T>> (reference).withMargin (margin).match (candidate);
        const bool manualResult = allWithinAbs (candidate, transformedReference, margin);

        CHECK (equalsResult == expected);
        CHECK (manualResult == expected);
        CHECK (equalsResult == manualResult);
    }

    SECTION ("WithinRel")
    {
        constexpr auto epsilon = 0.0;

        auto [candidate, expected] = GENERATE_REF (table<vctr::Vector<T>, bool> ({
            { transformedReference, true },
            { differing, false },
            { sizeMismatch, false }
        }));

        const bool equalsResult = vctr::EqualsTransformedBy<timesTwo<T>> (reference).withEpsilon (epsilon).match (candidate);
        const bool manualResult = allWithinRel (candidate, transformedReference, epsilon);

        CHECK (equalsResult == expected);
        CHECK (manualResult == expected);
        CHECK (equalsResult == manualResult);
    }
}

