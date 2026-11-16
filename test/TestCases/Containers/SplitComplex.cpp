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

#include <vctr_test_utils/vctr_test_common.h>

namespace
{
//==============================================================================
// Reference implementations using std::complex, to compare the split complex results against.
template <std::floating_point T>
std::complex<T> addition (std::complex<T> a, std::complex<T> b) { return a + b; }

template <std::floating_point T>
std::complex<T> subtraction (std::complex<T> a, std::complex<T> b) { return a - b; }

template <std::floating_point T>
std::complex<T> multiplication (std::complex<T> a, std::complex<T> b) { return a * b; }

template <std::floating_point T>
std::complex<T> division (std::complex<T> a, std::complex<T> b) { return a / b; }

template <std::floating_point T>
std::complex<T> multiplicationWithConjugate (std::complex<T> a, std::complex<T> b) { return a * std::conj (b); }

template <std::floating_point T>
std::complex<T> multiplyAccumulate (std::complex<T> acc, std::complex<T> b) { return acc + acc * b; }

template <std::floating_point T>
std::complex<T> multiplySubtract (std::complex<T> acc, std::complex<T> b) { return acc - acc * b; }

template <std::floating_point T>
std::complex<T> square (std::complex<T> a) { return a * a; }

template <std::floating_point T>
std::complex<T> conjugate (std::complex<T> a) { return std::conj (a); }

//==============================================================================
/** The split complex math is evaluated with a different formula than std::complex uses, so the results may differ
    by some rounding errors. The margin covers the cases where the real or imaginary result is close to zero while
    the intermediate products are not.
 */
template <class T>
constexpr double margin = std::same_as<T, float> ? 1e-2 : 1e-10;

/** Returns the values of a split complex container as an interleaved std::complex Vector, so that it can be checked
    with the usual vctr matchers.
 */
template <class ContainerType>
auto interleaved (const vctr::SplitComplexBase<ContainerType>& splitComplex)
{
    using Complex = typename vctr::SplitComplexBase<ContainerType>::complex_type;
    return vctr::Vector<Complex> (splitComplex.size(), [&] (size_t i) { return splitComplex[i]; });
}
} // namespace

/** Defines a few things needed in nearly every split complex test case:

    - Complex: The std::complex type matching the TestType
    - srcA, srcB: Two Vectors sized testVectorSize with random, nonzero interleaved complex values
    - splitA, splitB: Two SplitComplexVectors holding the same values as srcA and srcB
 */
#define VCTR_SPLIT_COMPLEX_TEST_DEFINES(testVectorSize)                                                        \
    using Complex = std::complex<TestType>;                                                                    \
    [[maybe_unused]] const auto srcA = UnitTestValues<Complex>::template vector<testVectorSize, 0> (true);     \
    [[maybe_unused]] const auto srcB = UnitTestValues<Complex>::template vector<testVectorSize, 1> (true);     \
    [[maybe_unused]] const vctr::SplitComplexVector<TestType> splitA (srcA);                                   \
    [[maybe_unused]] const vctr::SplitComplexVector<TestType> splitB (srcB);

//==============================================================================
// Construction
//==============================================================================

TEMPLATE_TEST_CASE ("SplitComplexVector Constructors", "[VCTR][SplitComplex][Constructor]", float, double)
{
    VCTR_SPLIT_COMPLEX_TEST_DEFINES (17)

    // Default constructor
    vctr::SplitComplexVector<TestType> empty;
    REQUIRE (empty.empty());
    REQUIRE (empty.size() == 0);

    // Size constructor initialises all values to zero
    vctr::SplitComplexVector<TestType> sized (5);
    REQUIRE (sized.size() == 5);
    REQUIRE_THAT (sized.real(), vctr::AllElementsEqualTo (TestType (0)));
    REQUIRE_THAT (sized.imag(), vctr::AllElementsEqualTo (TestType (0)));

    // Real and imaginary part constructor copies the parts into separate buffers
    const vctr::Vector<TestType> re { 1, 2, 3 };
    const vctr::Vector<TestType> im { 4, 5, 6 };
    vctr::SplitComplexVector<TestType> fromParts (re, im);
    REQUIRE_THAT (fromParts.real(), vctr::Equals (re));
    REQUIRE_THAT (fromParts.imag(), vctr::Equals (im));
    REQUIRE (fromParts.real().data() != re.data());
    REQUIRE (fromParts.real().data() != fromParts.imag().data());

    // Interleaved constructor deinterleaves the values
    vctr::SplitComplexVector<TestType> fromInterleaved (srcA);
    REQUIRE (fromInterleaved.size() == srcA.size());
    REQUIRE_THAT (interleaved (fromInterleaved), vctr::Equals (srcA));

    // Class template argument deduction from interleaved containers
    static_assert (std::same_as<decltype (vctr::SplitComplexBase (srcA)), vctr::SplitComplexVector<TestType>>);

    // Copy constructor
    auto copied = fromInterleaved;
    REQUIRE_THAT (interleaved (copied), vctr::Equals (srcA));
    REQUIRE (copied.real().data() != fromInterleaved.real().data());

    // Move constructor takes over the buffers
    const auto* realData = copied.real().data();
    const auto* imagData = copied.imag().data();
    auto moved = std::move (copied);
    REQUIRE_THAT (interleaved (moved), vctr::Equals (srcA));
    REQUIRE (moved.real().data() == realData);
    REQUIRE (moved.imag().data() == imagData);
}

TEMPLATE_TEST_CASE ("SplitComplexArray Constructors", "[VCTR][SplitComplex][Constructor]", float, double)
{
    using Complex = std::complex<TestType>;
    using SplitComplexArray = vctr::SplitComplexArray<TestType, 17>;

    constexpr auto srcA = UnitTestValues<Complex>::template array<17, 0>();

    // The size is fixed at compile time
    static_assert (! std::is_constructible_v<SplitComplexArray, size_t>);
    static_assert (! requires (SplitComplexArray& a) { a.resize (size_t (3)); });

    // Default constructor
    SplitComplexArray uninitialized;
    REQUIRE (uninitialized.size() == 17);
    REQUIRE (uninitialized.real().size() == 17);
    REQUIRE (uninitialized.imag().size() == 17);

    // Real and imaginary part constructor
    const vctr::Array<TestType, 2> re { TestType (1), TestType (2) };
    const vctr::Array<TestType, 2> im { TestType (3), TestType (4) };
    vctr::SplitComplexArray<TestType, 2> fromParts (re, im);
    REQUIRE (fromParts[0] == Complex (1, 3));
    REQUIRE (fromParts[1] == Complex (2, 4));

    // Interleaved constructor deinterleaves the values
    const SplitComplexArray fromInterleaved (srcA);
    REQUIRE_THAT (interleaved (fromInterleaved), vctr::Equals (srcA));

    // Class template argument deduction from interleaved containers
    static_assert (std::same_as<decltype (vctr::SplitComplexBase (srcA)), SplitComplexArray>);
}

TEMPLATE_TEST_CASE ("SplitComplex Conversion between container types", "[VCTR][SplitComplex][Constructor]", float, double)
{
    using Complex = std::complex<TestType>;
    using SplitComplexArray = vctr::SplitComplexArray<TestType, 17>;

    constexpr auto srcA = UnitTestValues<Complex>::template array<17, 0>();

    SplitComplexArray array (srcA);

    // Owning containers copy the data
    vctr::SplitComplexVector<TestType> vectorFromArray (array);
    REQUIRE_THAT (interleaved (vectorFromArray), vctr::Equals (srcA));
    REQUIRE (vectorFromArray.real().data() != array.real().data());

    const SplitComplexArray arrayFromVector (vectorFromArray);
    REQUIRE_THAT (interleaved (arrayFromVector), vctr::Equals (srcA));

    // Spans refer to the data
    vctr::SplitComplexSpan<TestType> span (array);
    REQUIRE (span.size() == array.size());
    REQUIRE (span.real().data() == array.real().data());
    REQUIRE (span.imag().data() == array.imag().data());

    const vctr::SplitComplexConstSpan<TestType> constSpan (arrayFromVector);
    REQUIRE (constSpan.real().data() == arrayFromVector.real().data());
    REQUIRE (constSpan.imag().data() == arrayFromVector.imag().data());

    // A vector constructed from a span copies the data
    vctr::SplitComplexVector<TestType> vectorFromSpan (span);
    REQUIRE_THAT (interleaved (vectorFromSpan), vctr::Equals (srcA));
    REQUIRE (vectorFromSpan.real().data() != array.real().data());
}

//==============================================================================
// Spans
//==============================================================================

TEMPLATE_TEST_CASE ("SplitComplexSpan", "[VCTR][SplitComplex][Span]", float, double)
{
    VCTR_SPLIT_COMPLEX_TEST_DEFINES (17)

    auto vector = splitA;

    // Writing through a mutable span modifies the viewed container
    vctr::SplitComplexSpan<TestType> span (vector);
    span.set (2, Complex (42, -42));
    REQUIRE (vector[2] == Complex (42, -42));

    // A span can view two separate real valued containers
    vctr::Vector<TestType> re { 1, 2 };
    vctr::Vector<TestType> im { 3, 4 };
    vctr::SplitComplexSpan<TestType> partsSpan (re, im);
    partsSpan.set (0, 10, 20);
    REQUIRE (re[0] == TestType (10));
    REQUIRE (im[0] == TestType (20));

    // Assigning a span of the same type copies the viewed elements, it doesn't rebind the span
    auto target = splitB;
    vctr::SplitComplexSpan<TestType> targetSpan (target);
    vctr::SplitComplexSpan<TestType> sourceSpan (vector);

    targetSpan = sourceSpan;
    REQUIRE_THAT (interleaved (target), vctr::Equals (interleaved (vector)));
    REQUIRE (targetSpan.real().data() == target.real().data());
    REQUIRE (targetSpan.imag().data() == target.imag().data());

    // Same for a temporary span
    auto otherTarget = splitB;
    vctr::SplitComplexSpan<TestType> otherTargetSpan (otherTarget);
    otherTargetSpan = vctr::SplitComplexSpan<TestType> (vector);
    REQUIRE_THAT (interleaved (otherTarget), vctr::Equals (interleaved (vector)));
    REQUIRE (otherTargetSpan.real().data() == otherTarget.real().data());
}

TEMPLATE_TEST_CASE ("SplitComplexConstSpan", "[VCTR][SplitComplex][Span]", float, double)
{
    VCTR_SPLIT_COMPLEX_TEST_DEFINES (17)

    using ConstSpan = vctr::SplitComplexConstSpan<TestType>;

    // A const span is read only, even if the span itself is not const
    static_assert (! ConstSpan::isMutable);
    static_assert (std::same_as<typename ConstSpan::element_type, const TestType>);
    static_assert (std::same_as<decltype (std::declval<ConstSpan&>().real()), vctr::Span<const TestType>>);
    static_assert (! requires (ConstSpan& s) { s.set (size_t (0), Complex()); });
    static_assert (! requires (ConstSpan& s) { s.fill (Complex()); });
    static_assert (! requires (ConstSpan& s) { s = splitB; });
    static_assert (! requires (ConstSpan& s, const ConstSpan& other) { s = other; });

    // It can view const and non-const containers
    auto vector = splitA;
    ConstSpan fromMutable (vector);
    ConstSpan fromConst (splitA);

    REQUIRE (fromMutable.real().data() == vector.real().data());
    REQUIRE (fromConst.imag().data() == splitA.imag().data());
    REQUIRE_THAT (interleaved (fromConst), vctr::Equals (srcA));
}

//==============================================================================
// Member functions
//==============================================================================

TEMPLATE_TEST_CASE ("SplitComplex Member Functions", "[VCTR][SplitComplex][MemberFunctions]", float, double)
{
    using Complex = std::complex<TestType>;

    vctr::SplitComplexVector<TestType> v (3);

    // set and element access
    v.set (0, Complex (1, 2));
    v.set (1, 3, 4);
    REQUIRE (v[0] == Complex (1, 2));
    REQUIRE (v[1] == Complex (3, 4));
    REQUIRE (v.real()[1] == TestType (3));
    REQUIRE (v.imag()[1] == TestType (4));

    // fill
    v.fill (Complex (7, -7));
    REQUIRE_THAT (v.real(), vctr::AllElementsEqualTo (TestType (7)));
    REQUIRE_THAT (v.imag(), vctr::AllElementsEqualTo (TestType (-7)));

    // clear
    v.clear();
    REQUIRE_THAT (v.real(), vctr::AllElementsEqualTo (TestType (0)));
    REQUIRE_THAT (v.imag(), vctr::AllElementsEqualTo (TestType (0)));

    // resize
    v.resize (5);
    REQUIRE (v.size() == 5);
    REQUIRE (v.real().size() == 5);
    REQUIRE (v.imag().size() == 5);

    v.resize (0);
    REQUIRE (v.empty());
}

TEMPLATE_TEST_CASE ("SplitComplex Assignment", "[VCTR][SplitComplex][Assignment]", float, double)
{
    VCTR_SPLIT_COMPLEX_TEST_DEFINES (17)

    // From interleaved data
    vctr::SplitComplexVector<TestType> fromInterleaved (srcA.size());
    fromInterleaved = srcA;
    REQUIRE_THAT (interleaved (fromInterleaved), vctr::Equals (srcA));

    // From another split complex container
    vctr::SplitComplexVector<TestType> fromVector (splitB.size());
    fromVector = splitB;
    REQUIRE_THAT (interleaved (fromVector), vctr::Equals (srcB));

    vctr::SplitComplexArray<TestType, 17> fromOtherContainerType;
    fromOtherContainerType = splitA;
    REQUIRE_THAT (interleaved (fromOtherContainerType), vctr::Equals (srcA));

    // Assigning to a span writes to the viewed container
    vctr::SplitComplexVector<TestType> target (srcA.size());
    vctr::SplitComplexSpan<TestType> targetSpan (target);
    targetSpan = splitA;
    REQUIRE_THAT (interleaved (target), vctr::Equals (srcA));

    // Assigning an expression resizes a SplitComplexVector
    vctr::SplitComplexVector<TestType> resized;
    resized = splitA + splitB;
    REQUIRE_THAT (interleaved (resized), vctr::EqualsTransformedBy<addition> (srcA, srcB).withEpsilon());
}

//==============================================================================
// Math
//==============================================================================

TEMPLATE_TEST_CASE ("SplitComplex Binary Math", "[VCTR][SplitComplex][Expressions]", float, double)
{
    // An odd size larger than any SIMD register, so that both the SIMD loop and the scalar tail are used
    VCTR_SPLIT_COMPLEX_TEST_DEFINES (101)

    // clang-format off
    const vctr::SplitComplexVector<TestType> sum        = splitA + splitB;
    const vctr::SplitComplexVector<TestType> difference = splitA - splitB;
    const vctr::SplitComplexVector<TestType> product    = splitA * splitB;
    const vctr::SplitComplexVector<TestType> quotient   = splitA / splitB;
    const vctr::SplitComplexVector<TestType> conjProd   = vctr::multiplyConjugate (splitA, splitB);

    REQUIRE_THAT (interleaved (sum),        vctr::EqualsTransformedBy<addition>                    (srcA, srcB).withEpsilon().withMargin (margin<TestType>));
    REQUIRE_THAT (interleaved (difference), vctr::EqualsTransformedBy<subtraction>                 (srcA, srcB).withEpsilon().withMargin (margin<TestType>));
    REQUIRE_THAT (interleaved (product),    vctr::EqualsTransformedBy<multiplication>              (srcA, srcB).withEpsilon().withMargin (margin<TestType>));
    REQUIRE_THAT (interleaved (quotient),   vctr::EqualsTransformedBy<division>                    (srcA, srcB).withEpsilon().withMargin (margin<TestType>));
    REQUIRE_THAT (interleaved (conjProd),   vctr::EqualsTransformedBy<multiplicationWithConjugate> (srcA, srcB).withEpsilon().withMargin (margin<TestType>));
    // clang-format on
}

TEMPLATE_TEST_CASE ("SplitComplex Binary Math with less elements than a SIMD register", "[VCTR][SplitComplex][Expressions]", float, double)
{
    VCTR_SPLIT_COMPLEX_TEST_DEFINES (3)

    const vctr::SplitComplexVector<TestType> product = splitA * splitB;

    REQUIRE_THAT (interleaved (product), vctr::EqualsTransformedBy<multiplication> (srcA, srcB).withEpsilon().withMargin (margin<TestType>));
}

TEMPLATE_TEST_CASE ("SplitComplex Scalar Math", "[VCTR][SplitComplex][Expressions]", float, double)
{
    VCTR_SPLIT_COMPLEX_TEST_DEFINES (101)

    const auto realScalar = TestType (-2.5);
    const auto complexScalar = Complex (TestType (1.5), TestType (-2));

    const vctr::SplitComplexVector<TestType> scaledByReal = splitA * realScalar;
    const vctr::SplitComplexVector<TestType> scaledByComplex = splitA * complexScalar;

    REQUIRE_THAT (interleaved (scaledByReal), vctr::EqualsTransformedBy<multiplication> (srcA, Complex (realScalar)).withEpsilon().withMargin (margin<TestType>));
    REQUIRE_THAT (interleaved (scaledByComplex), vctr::EqualsTransformedBy<multiplication> (srcA, complexScalar).withEpsilon().withMargin (margin<TestType>));
}

TEMPLATE_TEST_CASE ("SplitComplex Unary Math", "[VCTR][SplitComplex][Expressions]", float, double)
{
    VCTR_SPLIT_COMPLEX_TEST_DEFINES (101)

    const vctr::SplitComplexVector<TestType> conj = vctr::splitComplexConjugate (splitA);
    REQUIRE_THAT (interleaved (conj), vctr::EqualsTransformedBy<conjugate> (srcA));

    const vctr::Vector<TestType> magSquared = vctr::magnitudeSquared (splitA);
    const vctr::Vector<TestType> expectedMagSquared (srcA.size(), [&] (size_t i) { return std::norm (srcA[i]); });
    REQUIRE_THAT (magSquared, vctr::Equals (expectedMagSquared).withEpsilon (vctr::detail::defaultEpsilon<TestType>));

    const vctr::Vector<TestType> mag = vctr::magnitude (splitA);
    const vctr::Vector<TestType> expectedMag (srcA.size(), [&] (size_t i) { return std::abs (srcA[i]); });
    REQUIRE_THAT (mag, vctr::Equals (expectedMag).withEpsilon (vctr::detail::defaultEpsilon<TestType>));
}

TEMPLATE_TEST_CASE ("SplitComplex Math with mixed container types", "[VCTR][SplitComplex][Expressions]", float, double)
{
    VCTR_SPLIT_COMPLEX_TEST_DEFINES (17)

    const vctr::SplitComplexArray<TestType, 17> arrayA (srcA);
    const vctr::SplitComplexConstSpan<TestType> spanB (splitB);

    // Array and const span operands, vector destination
    const vctr::SplitComplexVector<TestType> product = arrayA * spanB;
    REQUIRE_THAT (interleaved (product), vctr::EqualsTransformedBy<multiplication> (srcA, srcB).withEpsilon().withMargin (margin<TestType>));

    // Const span operands, span destination
    vctr::SplitComplexVector<TestType> target (srcA.size());
    vctr::SplitComplexSpan<TestType> targetSpan (target);
    targetSpan = vctr::SplitComplexConstSpan<TestType> (splitA) * spanB;
    REQUIRE_THAT (interleaved (target), vctr::EqualsTransformedBy<multiplication> (srcA, srcB).withEpsilon().withMargin (margin<TestType>));

    // Vector operands, array destination
    vctr::SplitComplexArray<TestType, 17> arrayDestination;
    arrayDestination = splitA * splitB;
    REQUIRE_THAT (interleaved (arrayDestination), vctr::EqualsTransformedBy<multiplication> (srcA, srcB).withEpsilon().withMargin (margin<TestType>));
}

/*  The destination of these assignments is also one of the sources. Since the real and the imaginary result both depend
    on the real and the imaginary source values, this only works if both results are evaluated before any of them is
    written back to the destination.
 */
TEMPLATE_TEST_CASE ("SplitComplex In Place Math", "[VCTR][SplitComplex][Expressions]", float, double)
{
    VCTR_SPLIT_COMPLEX_TEST_DEFINES (101)

    const auto requireInPlaceResult = [&] (auto&& assignInPlace, auto matcher) {
        auto x = splitA;
        assignInPlace (x);
        REQUIRE_THAT (interleaved (x), std::move (matcher).withEpsilon().withMargin (margin<TestType>));
    };

    // clang-format off
    requireInPlaceResult ([&] (auto& x) { x = x * splitB; },                        vctr::EqualsTransformedBy<multiplication>              (srcA, srcB));
    requireInPlaceResult ([&] (auto& x) { x = splitB * x; },                        vctr::EqualsTransformedBy<multiplication>              (srcA, srcB));
    requireInPlaceResult ([&] (auto& x) { x = x / splitB; },                        vctr::EqualsTransformedBy<division>                    (srcA, srcB));
    requireInPlaceResult ([&] (auto& x) { x = vctr::multiplyConjugate (x, splitB); }, vctr::EqualsTransformedBy<multiplicationWithConjugate> (srcA, srcB));
    requireInPlaceResult ([&] (auto& x) { x = x + x * splitB; },                    vctr::EqualsTransformedBy<multiplyAccumulate>          (srcA, srcB));
    requireInPlaceResult ([&] (auto& x) { x = x - x * splitB; },                    vctr::EqualsTransformedBy<multiplySubtract>            (srcA, srcB));
    requireInPlaceResult ([&] (auto& x) { x = x * x; },                             vctr::EqualsTransformedBy<square>                      (srcA));
    requireInPlaceResult ([&] (auto& x) { x = x * Complex (2, -1); },               vctr::EqualsTransformedBy<multiplication>              (srcA, Complex (2, -1)));
    requireInPlaceResult ([&] (auto& x) { x = vctr::splitComplexConjugate (x); },   vctr::EqualsTransformedBy<conjugate>                   (srcA));
    // clang-format on

    // Through a span viewing the destination
    auto x = splitA;
    vctr::SplitComplexSpan<TestType> xSpan (x);
    xSpan = xSpan + x * splitB;
    REQUIRE_THAT (interleaved (x), vctr::EqualsTransformedBy<multiplyAccumulate> (srcA, srcB).withEpsilon().withMargin (margin<TestType>));
}

TEMPLATE_TEST_CASE ("SplitComplex Compound Assignment Operators", "[VCTR][SplitComplex][Expressions]", float, double)
{
    VCTR_SPLIT_COMPLEX_TEST_DEFINES (101)

    const auto realScalar = TestType (-2.5);
    const auto complexScalar = Complex (TestType (1.5), TestType (-2));

    // Applies a compound assignment to a copy of splitA and checks the result
    const auto requireCompoundResult = [&] (auto&& compoundAssign, auto matcher) {
        auto x = splitA;
        compoundAssign (x);
        REQUIRE_THAT (interleaved (x), std::move (matcher).withEpsilon().withMargin (margin<TestType>));
    };

    // clang-format off
    requireCompoundResult ([&] (auto& x) { x *= splitB; },        vctr::EqualsTransformedBy<multiplication> (srcA, srcB));
    requireCompoundResult ([&] (auto& x) { x *= realScalar; },    vctr::EqualsTransformedBy<multiplication> (srcA, Complex (realScalar)));
    requireCompoundResult ([&] (auto& x) { x *= complexScalar; }, vctr::EqualsTransformedBy<multiplication> (srcA, complexScalar));

    requireCompoundResult ([&] (auto& x) { x /= splitB; },        vctr::EqualsTransformedBy<division>       (srcA, srcB));
    requireCompoundResult ([&] (auto& x) { x /= realScalar; },    vctr::EqualsTransformedBy<division>       (srcA, Complex (realScalar)));
    requireCompoundResult ([&] (auto& x) { x /= complexScalar; }, vctr::EqualsTransformedBy<division>       (srcA, complexScalar));

    requireCompoundResult ([&] (auto& x) { x += splitB; },        vctr::EqualsTransformedBy<addition>       (srcA, srcB));
    requireCompoundResult ([&] (auto& x) { x += realScalar; },    vctr::EqualsTransformedBy<addition>       (srcA, Complex (realScalar)));
    requireCompoundResult ([&] (auto& x) { x += complexScalar; }, vctr::EqualsTransformedBy<addition>       (srcA, complexScalar));

    requireCompoundResult ([&] (auto& x) { x -= splitB; },        vctr::EqualsTransformedBy<subtraction>    (srcA, srcB));
    requireCompoundResult ([&] (auto& x) { x -= realScalar; },    vctr::EqualsTransformedBy<subtraction>    (srcA, Complex (realScalar)));
    requireCompoundResult ([&] (auto& x) { x -= complexScalar; }, vctr::EqualsTransformedBy<subtraction>    (srcA, complexScalar));

    // Expressions as operands, including expressions that read this container
    requireCompoundResult ([&] (auto& x) { x += x * splitB; },    vctr::EqualsTransformedBy<multiplyAccumulate> (srcA, srcB));
    requireCompoundResult ([&] (auto& x) { x -= x * splitB; },    vctr::EqualsTransformedBy<multiplySubtract>   (srcA, srcB));
    requireCompoundResult ([&] (auto& x) { x *= x; },             vctr::EqualsTransformedBy<square>             (srcA));
    // clang-format on
}

TEMPLATE_TEST_CASE ("SplitComplex Compound Assignment Operators on Arrays and Spans", "[VCTR][SplitComplex][Expressions]", float, double)
{
    VCTR_SPLIT_COMPLEX_TEST_DEFINES (17)

    // Array destination, span operand
    vctr::SplitComplexArray<TestType, 17> array (srcA);
    array *= vctr::SplitComplexConstSpan<TestType> (splitB);
    REQUIRE_THAT (interleaved (array), vctr::EqualsTransformedBy<multiplication> (srcA, srcB).withEpsilon().withMargin (margin<TestType>));

    // Span destination writes to the viewed container
    auto vector = splitA;
    vctr::SplitComplexSpan<TestType> span (vector);
    span += splitB;
    REQUIRE_THAT (interleaved (vector), vctr::EqualsTransformedBy<addition> (srcA, srcB).withEpsilon());

    // A const span can't be modified
    using ConstSpan = vctr::SplitComplexConstSpan<TestType>;
    static_assert (! requires (ConstSpan& s) { s *= splitB; });
    static_assert (! requires (ConstSpan& s) { s /= Complex(); });
    static_assert (! requires (ConstSpan& s) { s += TestType(); });
    static_assert (! requires (ConstSpan& s) { s -= splitB; });
}
