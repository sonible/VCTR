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
    size_t firstMismatchIndex = 0;
    size_t actualSize = 0;
    size_t referenceSize = 0;
    bool hasErrorStats = false;
    double minError = 0.0;
    double maxError = 0.0;
    double meanAbsErrorAllElements = 0.0;

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
            s << "\n\nMismatch statistics:"
              << "\n  Mismatching elements:               " << numMismatches << " out of " << referenceSize
              << "\n  First mismatch at index:            " << firstMismatchIndex;

            if (hasErrorStats)
            {
                s << "\n  Smallest error (actual - expected): " << minError
                  << "\n  Largest error  (actual - expected): " << maxError
                  << "\n  Mean abs error (all elements):      " << meanAbsErrorAllElements;

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
    constexpr bool hasNumericError = is::number<ActualValueType> && is::number<ExpectedValueType>;
    constexpr bool errorIsComplex = is::complexFloatNumber<ActualValueType> || is::complexFloatNumber<ExpectedValueType>;

    auto withinTolerance = [&] (double target, double value) {
        return (Catch::Matchers::WithinAbs (target, margin) || Catch::Matchers::WithinRel (target, epsilon)).match (value);
    };

    double sumAbsErrorAllElements = 0.0;

    for (size_t i = 0; i < numToCompare; ++i)
    {
        decltype (auto) actual = getActual (i);
        decltype (auto) reference = getExpected (i);

        double error = 0.0;

        if constexpr (hasNumericError)
        {
            if constexpr (errorIsComplex)
                error = std::abs (actual - reference);
            else
                error = double (actual) - double (reference);

            details.hasErrorStats = true;
            sumAbsErrorAllElements += std::abs (error);
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

        if (details.numMismatches == 0)
            details.firstMismatchIndex = i;

        ++details.numMismatches;

        if constexpr (hasNumericError)
        {
            details.minError = details.numMismatches == 1 ? error : std::min (details.minError, error);
            details.maxError = details.numMismatches == 1 ? error : std::max (details.maxError, error);

            if (details.collectedMismatches.size() < ElementwiseEqualityReport::maxNumErrorValuesToReport)
            {
                if constexpr (is::complexFloatNumber<ExpectedValueType>)
                    details.collectedMismatches.emplace_back (i, double (std::abs (actual)), double (std::abs (reference)));
                else
                    details.collectedMismatches.emplace_back (i, double (actual), double (reference));
            }
        }
    }

    if (numToCompare > 0 && details.hasErrorStats)
        details.meanAbsErrorAllElements = sumAbsErrorAllElements / double (numToCompare);

    return details.numMismatches == 0 && actualSize == referenceSize;
}

} // namespace vctr::detail
