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

namespace vctr::detail
{

/** Mismatch/error statistics collected by computeElementwiseEqualityDetails().

    Shared between the Equals and EqualsTransformedBy family of matchers so that
    all of them report failures in the same format.
*/
struct ElementwiseEqualityReport
{
    static constexpr auto maxNumErrorValuesToReport = 5; ///< Maximum number of mismatching values to report in the description.

    size_t numMismatches = 0;
    size_t actualSize = 0;
    size_t referenceSize = 0;
    bool hasErrorStats = false;
    std::optional<double> minAbsError = std::nullopt;
    std::optional<double> maxAbsError = std::nullopt;
    std::optional<double> meanAbsErrorAllElements = std::nullopt;

    /** Stores the first up to maxNumErrorValuesToReport mismatching values (index, actual, expected) for reporting.

        In case of complex numbers, the absolute value is stored.
     */
    Vector<std::tuple<size_t, double, double>> collectedMismatches;

    /** Appends a human readable description of the stored mismatch/error statistics to the given stream. Does
        nothing if there were no mismatches and the sizes matched.
    */
    void appendDescriptionTo (std::ostringstream& s) const
    {
        if (numMismatches > 0)
        {
            s << "Mismatch statistics:"
              << "\n  Mismatching elements:               " << numMismatches << " out of " << referenceSize;

            if (hasErrorStats)
            {
                VCTR_ASSERT (minAbsError.has_value() == meanAbsErrorAllElements.has_value());
                VCTR_ASSERT (maxAbsError.has_value() == meanAbsErrorAllElements.has_value());

                if (meanAbsErrorAllElements.has_value())
                {
                    s << "\n  Minimum absolute error of all mismatches: " << *minAbsError
                      << "\n  Maximum absolute error of all mismatches: " << *maxAbsError
                      << "\n  Mean absolute error of all elements:      " << *meanAbsErrorAllElements;
                }

                if (! collectedMismatches.empty())
                {
                    s << "\nFirst " << collectedMismatches.size() << " mismatching values:";

                    const auto oldPrecision = s.precision();
                    s << std::setprecision (std::numeric_limits<double>::max_digits10);

                    for (const auto& [index, actual, expected] : collectedMismatches)
                        s << "\n  At index " << index << ": actual = " << actual << ", expected = " << expected;

                    s.precision (oldPrecision);
                }
            }
        }

        if (actualSize != referenceSize)
            s << "\n  Size mismatch: actual size " << actualSize << ", expected size " << referenceSize;

        s << "\n";
    }
};

/** Compares actual/reference elements for equality, filling in mismatch and error statistics into details.

    getActual/getExpected are callables invoked exactly once per index i in [0, min (actualSize, referenceSize)),
    in increasing order, returning the actual/expected value at that index respectively. This allows callers to
    supply either simple operator[] based lookups or stateful iterator-advancing lambdas.

    ExpectedValueType is the value type of the expected/reference side and is used to decide whether to use a
    WithinAbs/WithinRel tolerance comparison (for real or complex floating point types, where for complex numbers
    the real and imaginary parts are compared independently) or exact equality (everything else, including integers,
    bools and non-numeric types like strings). margin/epsilon are only used when ExpectedValueType is a real or
    complex floating point number.

    Returns true if there were no mismatches and actualSize == referenceSize.
*/
template <class ExpectedValueType, class GetActual, class GetExpected>
bool computeElementwiseEqualityReport (ElementwiseEqualityReport& details, size_t actualSize, size_t referenceSize, double margin, double epsilon, GetActual&& getActual, GetExpected&& getExpected)
{
    details = {};
    details.actualSize = actualSize;
    details.referenceSize = referenceSize;

    const size_t numToCompare = std::min (actualSize, referenceSize);

    using ActualValueType = std::decay_t<decltype (getActual (size_t {}))>;
    constexpr auto isNotBool = ! std::same_as<std::remove_cvref_t<ActualValueType>, bool> && ! std::same_as<std::remove_cvref_t<ExpectedValueType>, bool>;
    constexpr auto hasNumericErrorIncludingBool = is::number<ActualValueType> && is::number<ExpectedValueType>; // is::number doesn't exclude bool
    constexpr auto hasNumericErrorExcludingBool = isNotBool && is::number<ActualValueType> && is::number<ExpectedValueType>;

    constexpr auto errorIsComplex = is::complexFloatNumber<ActualValueType> || is::complexFloatNumber<ExpectedValueType>;

    auto withinTolerance = [&] (double target, double value) {
        return (Catch::Matchers::WithinAbs (target, margin) || Catch::Matchers::WithinRel (target, epsilon)).match (value);
    };

    static_assert (std::is_arithmetic_v<int>);
    static_assert (std::is_arithmetic_v<bool>);

    double sumAbsErrorAllElements = 0.0;

    auto minAbsError = std::numeric_limits<double>::max();
    auto maxAbsError = 0.0;

    for (size_t i = 0; i < numToCompare; ++i)
    {
        decltype (auto) actual = getActual (i);
        decltype (auto) reference = getExpected (i);

        double absError = 0.0;

        if constexpr (hasNumericErrorIncludingBool)
        {
            if constexpr (errorIsComplex)
                absError = std::abs (actual - reference);
            else
                absError = std::abs (double (actual) - double (reference));

            details.hasErrorStats = true;
            sumAbsErrorAllElements += std::abs (absError);
        }

        const bool isEqual = [&] {
            if constexpr (is::realFloatNumber<ExpectedValueType>)
                return withinTolerance (double (reference), double (actual));
            else if constexpr (is::complexFloatNumber<ExpectedValueType>)
                return withinTolerance (double (reference.real()), double (actual.real())) && withinTolerance (double (reference.imag()), double (actual.imag()));
            else
                return actual == reference;
        }();

        if (isEqual)
            continue;

        ++details.numMismatches;

        if constexpr (hasNumericErrorIncludingBool)
        {
            minAbsError = std::min (minAbsError, absError);
            maxAbsError = std::max (maxAbsError, absError);

            if (details.collectedMismatches.size() < ElementwiseEqualityReport::maxNumErrorValuesToReport)
            {
                if constexpr (is::complexFloatNumber<ExpectedValueType>)
                    details.collectedMismatches.emplace_back (i, double (std::abs (actual)), double (std::abs (reference)));
                else
                    details.collectedMismatches.emplace_back (i, double (actual), double (reference));
            }
        }
    }

    if constexpr (hasNumericErrorExcludingBool)
    {
        details.minAbsError = minAbsError;
        details.maxAbsError = maxAbsError;

        if (numToCompare > 0 && details.hasErrorStats)
            details.meanAbsErrorAllElements = sumAbsErrorAllElements / double (numToCompare);
    }

    return details.numMismatches == 0 && actualSize == referenceSize;
}

} // namespace vctr::detail
