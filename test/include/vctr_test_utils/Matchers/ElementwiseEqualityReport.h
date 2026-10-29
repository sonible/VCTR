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

    static double relativeError (double actual, double expected)
    {
        const auto absoluteError = std::abs (actual - expected);
        const auto scale = std::max (std::abs (actual), std::abs (expected));

        if (scale == 0.0)
            return absoluteError == 0.0 ? 0.0 : std::numeric_limits<double>::infinity();

        return absoluteError / scale;
    }

    struct Mismatch
    {
        size_t index;
        double actualReal;
        double expectedReal;
        double actualImag;
        double expectedImag;
        bool isComplex;
    };

    size_t numMismatches = 0;
    size_t actualSize = 0;
    size_t referenceSize = 0;
    bool hasErrorStats = false;
    bool hasComplexErrorStats = false;
    std::optional<double> minAbsError = std::nullopt;
    std::optional<double> maxAbsError = std::nullopt;
    std::optional<double> meanAbsErrorAllElements = std::nullopt;
    std::optional<double> minRelError = std::nullopt;
    std::optional<double> maxRelError = std::nullopt;
    std::optional<double> minRealAbsError = std::nullopt;
    std::optional<double> maxRealAbsError = std::nullopt;
    std::optional<double> meanRealAbsErrorAllElements = std::nullopt;
    std::optional<double> minRealRelError = std::nullopt;
    std::optional<double> maxRealRelError = std::nullopt;
    std::optional<double> minImagAbsError = std::nullopt;
    std::optional<double> maxImagAbsError = std::nullopt;
    std::optional<double> meanImagAbsErrorAllElements = std::nullopt;
    std::optional<double> minImagRelError = std::nullopt;
    std::optional<double> maxImagRelError = std::nullopt;

    /** Stores the first up to maxNumErrorValuesToReport mismatching values for reporting. */
    Vector<Mismatch> collectedMismatches;

    /** Appends a human readable description of the stored mismatch/error statistics to the given stream.

       Does nothing if there were no mismatches and the sizes matched.
    */
    void appendDescriptionTo (std::ostringstream& s) const
    {
        if (numMismatches > 0)
        {
            s << "Mismatch statistics:"
              << "\n  Mismatching elements:                      " << numMismatches << " out of " << referenceSize;

            if (hasErrorStats)
            {
                if (hasComplexErrorStats)
                {
                    VCTR_ASSERT (minRealAbsError.has_value() == meanRealAbsErrorAllElements.has_value());
                    VCTR_ASSERT (maxRealAbsError.has_value() == meanRealAbsErrorAllElements.has_value());
                    VCTR_ASSERT (minRealRelError.has_value() == meanRealAbsErrorAllElements.has_value());
                    VCTR_ASSERT (maxRealRelError.has_value() == meanRealAbsErrorAllElements.has_value());
                    VCTR_ASSERT (minImagAbsError.has_value() == meanImagAbsErrorAllElements.has_value());
                    VCTR_ASSERT (maxImagAbsError.has_value() == meanImagAbsErrorAllElements.has_value());
                    VCTR_ASSERT (minImagRelError.has_value() == meanImagAbsErrorAllElements.has_value());
                    VCTR_ASSERT (maxImagRelError.has_value() == meanImagAbsErrorAllElements.has_value());

                    if (meanRealAbsErrorAllElements.has_value() && meanImagAbsErrorAllElements.has_value())
                    {
                        s << "\n  Real component:"
                          << "\n    Minimum abs (rel) error of all mismatches: " << *minRealAbsError << " (" << *minRealRelError << ")"
                          << "\n    Maximum abs (rel) error of all mismatches: " << *maxRealAbsError << " (" << *maxRealRelError << ")"
                          << "\n    Mean absolute error of all elements:       " << *meanRealAbsErrorAllElements
                          << "\n  Imaginary component:"
                          << "\n    Minimum abs (rel) error of all mismatches: " << *minImagAbsError << " (" << *minImagRelError << ")"
                          << "\n    Maximum abs (rel) error of all mismatches: " << *maxImagAbsError << " (" << *maxImagRelError << ")"
                          << "\n    Mean absolute error of all elements:       " << *meanImagAbsErrorAllElements;
                    }
                }
                else
                {
                    VCTR_ASSERT (minAbsError.has_value() == meanAbsErrorAllElements.has_value());
                    VCTR_ASSERT (maxAbsError.has_value() == meanAbsErrorAllElements.has_value());
                    VCTR_ASSERT (minRelError.has_value() == meanAbsErrorAllElements.has_value());
                    VCTR_ASSERT (maxRelError.has_value() == meanAbsErrorAllElements.has_value());

                    if (meanAbsErrorAllElements.has_value())
                    {
                        s << "\n  Minimum abs (rel) error of all mismatches: " << *minAbsError << " (" << *minRelError << ")"
                          << "\n  Maximum abs (rel) error of all mismatches: " << *maxAbsError << " (" << *maxRelError << ")"
                          << "\n  Mean absolute error of all elements:       " << *meanAbsErrorAllElements;
                    }
                }

                if (! collectedMismatches.empty())
                {
                    s << "\nFirst " << collectedMismatches.size() << " mismatching values:";

                    const auto oldPrecision = s.precision();
                    s << std::setprecision (std::numeric_limits<double>::max_digits10);

                    for (const auto& mismatch : collectedMismatches)
                    {
                        constexpr auto labelWidth = 16;

                        s << "\n  At index " << mismatch.index << ":";

                        if (mismatch.isComplex)
                        {
                            s << "\n    Real:"
                              << "\n      " << std::left << std::setw (labelWidth) << "actual" << " = " << mismatch.actualReal
                              << "\n      " << std::setw (labelWidth) << "expected" << " = " << mismatch.expectedReal
                              << "\n      " << std::setw (labelWidth) << "abs (rel) error" << " = " << std::abs (mismatch.actualReal - mismatch.expectedReal)
                              << " (" << relativeError (mismatch.actualReal, mismatch.expectedReal) << ")"
                              << "\n    Imaginary:"
                              << "\n      " << std::setw (labelWidth) << "actual" << " = " << mismatch.actualImag
                              << "\n      " << std::setw (labelWidth) << "expected" << " = " << mismatch.expectedImag
                              << "\n      " << std::setw (labelWidth) << "abs (rel) error" << " = " << std::abs (mismatch.actualImag - mismatch.expectedImag)
                              << " (" << relativeError (mismatch.actualImag, mismatch.expectedImag) << ")"
                              << std::right;
                        }
                        else
                        {
                            s << "\n    " << std::left << std::setw (labelWidth) << "actual" << " = " << mismatch.actualReal
                              << "\n    " << std::setw (labelWidth) << "expected" << " = " << mismatch.expectedReal
                              << "\n    " << std::setw (labelWidth) << "abs (rel) error" << " = " << std::abs (mismatch.actualReal - mismatch.expectedReal)
                              << " (" << relativeError (mismatch.actualReal, mismatch.expectedReal) << ")"
                              << std::right;
                        }
                    }

                    s.precision (oldPrecision);
                }
            }

            s << "\n";
        }

        if (actualSize != referenceSize)
            s << "\n  Size mismatch: actual size " << actualSize << ", expected size " << referenceSize << "\n";
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
template <class GetActual, class GetExpected>
bool computeElementwiseEqualityReport (ElementwiseEqualityReport& details, size_t actualSize, size_t referenceSize, double margin, double epsilon, GetActual&& getActual, GetExpected&& getExpected)
{
    details = {};
    details.actualSize = actualSize;
    details.referenceSize = referenceSize;

    const size_t numToCompare = std::min (actualSize, referenceSize);

    using ActualValueType = std::decay_t<decltype (getActual (size_t {}))>;
    using ExpectedValueType = std::decay_t<decltype (getExpected (size_t {}))>;

    constexpr auto isNotBool = ! std::same_as<std::remove_cvref_t<ActualValueType>, bool> && ! std::same_as<std::remove_cvref_t<ExpectedValueType>, bool>;
    constexpr auto hasNumericErrorIncludingBool = is::number<ActualValueType> && is::number<ExpectedValueType>; // is::number doesn't exclude bool
    constexpr auto hasNumericErrorExcludingBool = isNotBool && is::number<ActualValueType> && is::number<ExpectedValueType>;

    constexpr auto errorIsComplex = is::complexFloatNumber<ActualValueType> || is::complexFloatNumber<ExpectedValueType>;
    details.hasComplexErrorStats = errorIsComplex;

    auto withinTolerance = [&] (double target, double value) {
        return (Catch::Matchers::WithinAbs (target, margin) || Catch::Matchers::WithinRel (target, epsilon)).match (value);
    };

    static_assert (std::is_arithmetic_v<int>);
    static_assert (std::is_arithmetic_v<bool>);

    double sumAbsErrorAllElements = 0.0;
    double sumRealAbsErrorAllElements = 0.0;
    double sumImagAbsErrorAllElements = 0.0;

    auto minAbsError = std::numeric_limits<double>::max();
    auto maxAbsError = 0.0;
    auto minRelError = std::numeric_limits<double>::max();
    auto maxRelError = 0.0;
    auto minRealAbsError = std::numeric_limits<double>::max();
    auto maxRealAbsError = 0.0;
    auto minRealRelError = std::numeric_limits<double>::max();
    auto maxRealRelError = 0.0;
    auto minImagAbsError = std::numeric_limits<double>::max();
    auto maxImagAbsError = 0.0;
    auto minImagRelError = std::numeric_limits<double>::max();
    auto maxImagRelError = 0.0;

    for (size_t i = 0; i < numToCompare; ++i)
    {
        decltype (auto) actual = getActual (i);
        decltype (auto) reference = getExpected (i);

        double absError = 0.0;
        double realAbsError = 0.0;
        double imagAbsError = 0.0;

        if constexpr (hasNumericErrorIncludingBool)
        {
            if constexpr (errorIsComplex)
            {
                absError = std::abs (actual - reference);
                realAbsError = std::abs (double (actual.real()) - double (reference.real()));
                imagAbsError = std::abs (double (actual.imag()) - double (reference.imag()));
                sumRealAbsErrorAllElements += realAbsError;
                sumImagAbsErrorAllElements += imagAbsError;
            }
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
            if constexpr (errorIsComplex)
            {
                minRealAbsError = std::min (minRealAbsError, realAbsError);
                maxRealAbsError = std::max (maxRealAbsError, realAbsError);
                minRealRelError = std::min (minRealRelError, ElementwiseEqualityReport::relativeError (actual.real(), reference.real()));
                maxRealRelError = std::max (maxRealRelError, ElementwiseEqualityReport::relativeError (actual.real(), reference.real()));
                minImagAbsError = std::min (minImagAbsError, imagAbsError);
                maxImagAbsError = std::max (maxImagAbsError, imagAbsError);
                minImagRelError = std::min (minImagRelError, ElementwiseEqualityReport::relativeError (actual.imag(), reference.imag()));
                maxImagRelError = std::max (maxImagRelError, ElementwiseEqualityReport::relativeError (actual.imag(), reference.imag()));
            }
            else
            {
                minAbsError = std::min (minAbsError, absError);
                maxAbsError = std::max (maxAbsError, absError);
                minRelError = std::min (minRelError, ElementwiseEqualityReport::relativeError (actual, reference));
                maxRelError = std::max (maxRelError, ElementwiseEqualityReport::relativeError (actual, reference));
            }

            if (details.collectedMismatches.size() < ElementwiseEqualityReport::maxNumErrorValuesToReport)
            {
                if constexpr (is::complexFloatNumber<ExpectedValueType>)
                    details.collectedMismatches.push_back ({
                        i,
                        double (actual.real()),
                        double (reference.real()),
                        double (actual.imag()),
                        double (reference.imag()),
                        true
                    });
                else
                    details.collectedMismatches.push_back ({ i, double (actual), double (reference), 0.0, 0.0, false });
            }
        }
    }

    if constexpr (hasNumericErrorExcludingBool)
    {
        if constexpr (errorIsComplex)
        {
            details.minRealAbsError = minRealAbsError;
            details.maxRealAbsError = maxRealAbsError;
            details.minRealRelError = minRealRelError;
            details.maxRealRelError = maxRealRelError;
            details.minImagAbsError = minImagAbsError;
            details.maxImagAbsError = maxImagAbsError;
            details.minImagRelError = minImagRelError;
            details.maxImagRelError = maxImagRelError;

            if (numToCompare > 0 && details.hasErrorStats)
            {
                details.meanRealAbsErrorAllElements = sumRealAbsErrorAllElements / double (numToCompare);
                details.meanImagAbsErrorAllElements = sumImagAbsErrorAllElements / double (numToCompare);
            }
        }
        else
        {
            details.minAbsError = minAbsError;
            details.maxAbsError = maxAbsError;
            details.minRelError = minRelError;
            details.maxRelError = maxRelError;

            if (numToCompare > 0 && details.hasErrorStats)
                details.meanAbsErrorAllElements = sumAbsErrorAllElements / double (numToCompare);
        }
    }

    return details.numMismatches == 0 && actualSize == referenceSize;
}

} // namespace vctr::detail
