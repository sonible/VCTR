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

#include <pago_unit_test_support/Catch/pago_VectorMatchers.h>
#include <vctr_test_utils/vctr_test_common.h>

namespace
{
template <class T>
constexpr T tolerance = std::is_same_v<T, float> ? T (1e-2) : T (1e-9);

struct TestData
{
    template <class FloatType, int seed, size_t numValues = 17>
    static constexpr vctr::Array<std::complex<FloatType>, numValues> makeArray()
    {
        return UnitTestValues<std::complex<FloatType>>::template array<numValues, seed, -10, 10> (true);
    }
};

template <class ContainerType>
auto makeInterleaved (const vctr::SplitComplexBase<ContainerType>& v)
{
    return vctr::Vector { v.size(), [&v] (size_t i) { return v[i]; }};
}

template <class T>
void requireClose (const vctr::SplitComplexBase<vctr::Vector<T>>& actual, const std::vector<std::complex<T>>& expected)
{
    REQUIRE (actual.size() == expected.size());

    for (size_t i = 0; i < expected.size(); ++i)
    {
        INFO ("index " << i);
        CHECK (std::abs (actual[i].real() - expected[i].real()) <= tolerance<T> * (T (1) + std::abs (expected[i].real())));
        CHECK (std::abs (actual[i].imag() - expected[i].imag()) <= tolerance<T> * (T (1) + std::abs (expected[i].imag())));
    }
}

template <class T>
vctr::Vector<std::complex<T>> testData (int seed)
{
    const vctr::Array a = UnitTestValues<std::complex<T>>::template array<17, 0, -10, 10> (true);
    const vctr::Array b = UnitTestValues<std::complex<T>>::template array<17, 1, -10, 10> (true);

    const auto& src = seed == 0 ? a : b;
    return vctr::Vector<std::complex<T>> (src.begin(), src.end());
}
} // namespace

TEMPLATE_TEST_CASE ("SplitComplex construction", "[VCTR][SplitComplex]", float, double)
{
    SECTION ("Default and sized")
    {
        vctr::SplitComplexVector<TestType> empty;
        REQUIRE (empty.empty());
        REQUIRE (empty.size() == 0);

        vctr::SplitComplexVector<TestType> sized (5);
        REQUIRE (sized.size() == 5);

        for (size_t i = 0; i < sized.size(); ++i)
            REQUIRE (sized[i] == std::complex<TestType> (0, 0));
    }

    SECTION ("From real and imaginary parts")
    {
        const vctr::Vector<TestType> re { 1, 2, 3 };
        const vctr::Vector<TestType> im { 4, 5, 6 };

        vctr::SplitComplexVector<TestType> v (re, im);

        REQUIRE (v.size() == 3);
        REQUIRE (v[1] == std::complex<TestType> (2, 5));
        REQUIRE_THAT (v.real(), vctr::Equals (re));
        REQUIRE_THAT (v.imag(), vctr::Equals (im));
        REQUIRE (v.real().data() != re.data());
    }

    SECTION ("Real and imaginary parts are stored in separate buffers")
    {
        vctr::SplitComplexVector<TestType> v (4);
        REQUIRE (v.real().data() != v.imag().data());
    }

    SECTION ("From interleaved data")
    {
        constexpr auto data = TestData::makeArray<float, 0>();
        const auto v = vctr::SplitComplexVector<float> (data);

        REQUIRE (v.size() == data.size());

        for (size_t i = 0; i < data.size(); ++i)
        {
            REQUIRE (v.real()[i] == data[i].real());
            REQUIRE (v.imag()[i] == data[i].imag());
        }

        REQUIRE (makeInterleaved (v) == data);
    }

    SECTION ("Copy and move")
    {
        const auto data = testData<TestType> (0);
        const auto v = makeSplit (data);

        auto copy = v;
        REQUIRE (makeInterleaved (copy) == data);
        REQUIRE (copy.real().data() != v.real().data());

        const auto* realPtr = copy.real().data();
        auto moved = std::move (copy);
        REQUIRE (makeInterleaved (moved) == data);
        REQUIRE (moved.real().data() == realPtr);
    }
}

TEMPLATE_TEST_CASE ("SplitComplex spans", "[VCTR][SplitComplex]", float, double)
{
    const auto data = testData<TestType> (0);
    auto v = makeSplit (data);

    SECTION ("Mutable span views the vector")
    {
        vctr::SplitComplexSpan<TestType> s (v);

        REQUIRE (s.size() == v.size());
        REQUIRE (s.real().data() == v.real().data());
        REQUIRE (s.imag().data() == v.imag().data());

        s.set (2, { 42, -42 });
        REQUIRE (v[2] == std::complex<TestType> (42, -42));
    }

    SECTION ("Const span views the vector")
    {
        const vctr::SplitComplexConstSpan<TestType> s (v);

        REQUIRE (s.real().data() == v.real().data());
        REQUIRE (s[3] == data[3]);
    }

    SECTION ("Const span from const vector")
    {
        const auto& cv = v;
        const vctr::SplitComplexConstSpan<TestType> s (cv);
        REQUIRE (s.imag().data() == v.imag().data());
    }

    SECTION ("Vector from span copies")
    {
        vctr::SplitComplexSpan<TestType> s (v);
        vctr::SplitComplexVector<TestType> copy (s);

        REQUIRE (makeInterleaved (copy) == data);
        REQUIRE (copy.real().data() != v.real().data());
    }

    SECTION ("Span from raw containers")
    {
        vctr::Vector<TestType> re { 1, 2 };
        vctr::Vector<TestType> im { 3, 4 };

        vctr::SplitComplexSpan<TestType> s (re, im);
        s.set (0, 10, 20);

        REQUIRE (re[0] == 10);
        REQUIRE (im[0] == 20);
    }
}

TEMPLATE_TEST_CASE ("SplitComplex element access and manipulation", "[VCTR][SplitComplex]", float, double)
{
    using Complex = std::complex<TestType>;

    vctr::SplitComplexVector<TestType> v (3);

    v.set (0, Complex (1, 2));
    v.set (1, 3, 4);
    REQUIRE (v[0] == Complex (1, 2));
    REQUIRE (v[1] == Complex (3, 4));

    v.fill (Complex (7, -7));
    REQUIRE (v[2] == Complex (7, -7));

    v.clear();
    REQUIRE (v[0] == Complex (0, 0));

    v.resize (5);
    REQUIRE (v.size() == 5);
    REQUIRE (v.imag().size() == 5);

    const auto data = testData<TestType> (1);
    vctr::SplitComplexVector<TestType> other (data.size());
    other.assignInterleaved (data);

    vctr::SplitComplexVector<TestType> target (data.size());
    target = other;
    REQUIRE (makeInterleaved (target) == data);

    vctr::Vector<Complex> interleaved (data.size());
    target.copyToInterleaved (interleaved);
    REQUIRE (std::equal (data.begin(), data.end(), interleaved.begin()));
}

TEMPLATE_TEST_CASE ("SplitComplex binary math", "[VCTR][SplitComplex]", float, double)
{
    using Complex = std::complex<TestType>;

    const auto dataA = testData<TestType> (0);
    const auto dataB = testData<TestType> (1);
    const auto a = makeSplit (dataA);
    const auto b = makeSplit (dataB);

    const auto expect = [&] (auto op) {
        std::vector<Complex> r (dataA.size());

        for (size_t i = 0; i < r.size(); ++i)
            r[i] = op (dataA[i], dataB[i]);

        return r;
    };

    vctr::SplitComplexVector<TestType> dst (a.size());

    SECTION ("add")
    {
        dst = a + b;

        vctr::Vector<std::complex<float>> orig = dataA + dataB;

        requireClose (dst, expect ([] (Complex x, Complex y) { return x + y; }));
    }

    SECTION ("sub")
    {
        dst = a - b;
        requireClose (dst, expect ([] (Complex x, Complex y) { return x - y; }));
    }

    SECTION ("mul")
    {
        dst = a * b;
        requireClose (dst, expect ([] (Complex x, Complex y) { return x * y; }));
    }

    SECTION ("mulConj")
    {
        dst = vctr::multiplyConjugate (a, b);
        requireClose (dst, expect ([] (Complex x, Complex y) { return x * std::conj (y); }));
    }

    SECTION ("div")
    {
        dst = a / b;
        requireClose (dst, expect ([] (Complex x, Complex y) { return x / y; }));
    }

    SECTION ("mulAdd")
    {
        auto acc = makeSplit (testData<TestType> (1));
        acc = acc + a * b;
        requireClose (acc, expect ([] (Complex x, Complex y) { return y + x * y; }));
    }

    SECTION ("mulSub")
    {
        auto acc = makeSplit (testData<TestType> (1));
        acc = acc - a * b;
        requireClose (acc, expect ([] (Complex x, Complex y) { return y - x * y; }));
    }

    SECTION ("Works with spans as operands and destination")
    {
        vctr::SplitComplexConstSpan<TestType> aSpan (a);
        vctr::SplitComplexConstSpan<TestType> bSpan (b);
        vctr::SplitComplexSpan<TestType> dstSpan (dst);

        dstSpan = aSpan * bSpan;
        requireClose (dst, expect ([] (Complex x, Complex y) { return x * y; }));
    }

    SECTION ("In place multiplication into first operand")
    {
        auto x = a;
        x = x * b;
        requireClose (x, expect ([] (Complex p, Complex q) { return p * q; }));
    }

    SECTION ("In place multiplication into second operand")
    {
        auto y = b;
        y = a * y;
        requireClose (y, expect ([] (Complex p, Complex q) { return p * q; }));
    }

    SECTION ("In place square")
    {
        auto x = a;
        x = x * x;
        requireClose (x, expect ([] (Complex p, Complex) { return p * p; }));
    }

    SECTION ("Matches std::complex for sizes not divisible by SIMD widths")
    {
        for (size_t n : { size_t (0), size_t (1), size_t (3), size_t (7), size_t (9) })
        {
            std::vector<Complex> sa (dataA.begin(), dataA.begin() + (ptrdiff_t) n);
            std::vector<Complex> sb (dataB.begin(), dataB.begin() + (ptrdiff_t) n);
            std::vector<Complex> expected (n);

            for (size_t i = 0; i < n; ++i)
                expected[i] = sa[i] * sb[i];

            vctr::SplitComplexVector<TestType> r (n);
            const auto saSplit = makeSplit (sa);
            const auto sbSplit = makeSplit (sb);
            r = saSplit * sbSplit;
            requireClose (r, expected);
        }
    }
}

TEMPLATE_TEST_CASE ("SplitComplex unary and scalar math", "[VCTR][SplitComplex]", float, double)
{
    using Complex = std::complex<TestType>;

    const auto data = testData<TestType> (0);
    const auto a = makeSplit (data);
    vctr::SplitComplexVector<TestType> dst (a.size());

    SECTION ("conj")
    {
        dst = vctr::splitComplexConjugate (a);

        std::vector<Complex> expected;
        for (auto c : data)
            expected.push_back (std::conj (c));

        requireClose (dst, expected);
    }

    SECTION ("conj in place")
    {
        auto x = a;
        x = vctr::splitComplexConjugate (x);

        for (size_t i = 0; i < data.size(); ++i)
            REQUIRE (x[i] == std::conj (data[i]));
    }

    SECTION ("mulReal")
    {
        dst = a * TestType (-2.5);

        std::vector<Complex> expected;
        for (auto c : data)
            expected.push_back (c * TestType (-2.5));

        requireClose (dst, expected);
    }

    SECTION ("mulComplex")
    {
        const Complex f (TestType (1.5), TestType (-2));
        dst = a * f;

        std::vector<Complex> expected;
        for (auto c : data)
            expected.push_back (c * f);

        requireClose (dst, expected);
    }

    SECTION ("magnitudeSquared")
    {
        vctr::Vector<TestType> result (a.size());
        result = vctr::magnitudeSquared (a);

        for (size_t i = 0; i < data.size(); ++i)
            REQUIRE (std::abs (result[i] - std::norm (data[i])) <= tolerance<TestType> * (1 + std::norm (data[i])));
    }

    SECTION ("magnitude into a span")
    {
        vctr::Vector<TestType> result (a.size());
        result = vctr::magnitude (a);

        for (size_t i = 0; i < data.size(); ++i)
            REQUIRE (std::abs (result[i] - std::abs (data[i])) <= tolerance<TestType> * (1 + std::abs (data[i])));
    }
}

TEMPLATE_TEST_CASE ("SplitComplexArray", "[VCTR][SplitComplex]", float, double)
{
    using Complex = std::complex<TestType>;
    using SCArray = vctr::SplitComplexArray<TestType, 17>;
    ;
    static_assert (! requires (SCArray& a) { a.resize (size_t (3)); });
    static_assert (! std::is_constructible_v<SCArray, size_t>);

    const auto dataA = testData<TestType> (0);
    const auto dataB = testData<TestType> (1);

    SECTION ("Construction and element access")
    {
        SCArray a;
        REQUIRE (a.size() == 17);
        REQUIRE (a.real().size() == 17);

        a.clear();
        REQUIRE (a[16] == Complex (0, 0));

        a.set (3, Complex (1, -1));
        REQUIRE (a[3] == Complex (1, -1));

        a.fill (Complex (2, 3));
        REQUIRE (a[0] == Complex (2, 3));
        REQUIRE (a.real().data() != a.imag().data());
    }

    SECTION ("From interleaved data and back")
    {
        const SCArray a (dataA);

        for (size_t i = 0; i < dataA.size(); ++i)
            REQUIRE (a[i] == dataA[i]);

        std::vector<Complex> back (a.size());
        a.copyToInterleaved (back);
        REQUIRE (back == dataA);
    }

    SECTION ("From real and imaginary arrays")
    {
        vctr::Array<TestType, 2> re { TestType (1), TestType (2) };
        vctr::Array<TestType, 2> im { TestType (3), TestType (4) };

        vctr::SplitComplexArray<TestType, 2> a (re, im);
        REQUIRE (a[1] == Complex (2, 4));
    }

    SECTION ("Conversion to and from vectors and spans")
    {
        const SCArray a (dataA);

        vctr::SplitComplexVector<TestType> v (a);
        REQUIRE (toStd (v) == dataA);

        SCArray fromVector (v);
        REQUIRE (fromVector[5] == dataA[5]);

        SCArray mutableArray (dataA);
        vctr::SplitComplexSpan<TestType> s (mutableArray);
        s.set (0, Complex (9, 9));
        REQUIRE (mutableArray[0] == Complex (9, 9));

        const vctr::SplitComplexConstSpan<TestType> cs (a);
        REQUIRE (cs.real().data() == a.real().data());

        SCArray copy;
        copy = v;
        REQUIRE (copy[7] == dataA[7]);
    }

    SECTION ("Math with arrays, vectors and spans mixed")
    {
        const SCArray a (dataA);
        const auto b = makeSplit (dataB);
        SCArray dst;

        dst = a * b;

        vctr::SplitComplexVector<TestType> result (dst);
        std::vector<Complex> expected (dataA.size());
        for (size_t i = 0; i < expected.size(); ++i)
            expected[i] = dataA[i] * dataB[i];

        requireClose (result, expected);

        SCArray inPlace (dataA);
        vctr::SplitComplexSpan<TestType> inPlaceSpan (inPlace);

        auto ex = inPlaceSpan + (inPlace * b);
        inPlaceSpan = ex;

        for (size_t i = 0; i < expected.size(); ++i)
            expected[i] = dataA[i] + dataA[i] * dataB[i];

        requireClose (vctr::SplitComplexVector<TestType> (inPlace), expected);
    }
}

TEMPLATE_TEST_CASE ("SplitComplex in place math on larger containers", "[VCTR][SplitComplex]", float, double)
{
    using Complex = std::complex<TestType>;

    // Odd size, larger than typical SIMD register widths
    constexpr size_t n = 3 * 256 + 5;

    std::vector<Complex> dataA (n), dataB (n);
    for (size_t i = 0; i < n; ++i)
    {
        dataA[i] = Complex (TestType (std::sin (0.1 * double (i))) * 5, TestType (std::cos (0.3 * double (i))) * 3);
        dataB[i] = Complex (TestType (1.5) + TestType (std::cos (0.2 * double (i))), TestType (std::sin (0.7 * double (i))));
    }

    const auto b = makeSplit (dataB);
    std::vector<Complex> expected (n);

    const auto check = [&] (auto fn, auto expectedOp) {
        auto x = makeSplit (dataA);
        fn (x);

        for (size_t i = 0; i < n; ++i)
            expected[i] = expectedOp (dataA[i], dataB[i]);

        requireClose (x, expected);
    };

    check ([&] (auto& x) { x = x * b; }, [] (Complex p, Complex q) { return p * q; });
    check ([&] (auto& x) { x = b * x; }, [] (Complex p, Complex q) { return q * p; });
    check ([&] (auto& x) { x = vctr::multiplyConjugate (x, b); }, [] (Complex p, Complex q) { return p * std::conj (q); });
    check ([&] (auto& x) { x = x / b; }, [] (Complex p, Complex q) { return p / q; });
    check ([&] (auto& x) { x = x + x * b; }, [] (Complex p, Complex q) { return p + p * q; });
    check ([&] (auto& x) { x = x - x * b; }, [] (Complex p, Complex q) { return p - p * q; });
    check ([&] (auto& x) { x = x * Complex (2, -1); }, [] (Complex p, Complex) { return p * Complex (2, -1); });
    check ([&] (auto& x) { x = x + b; }, [] (Complex p, Complex q) { return p + q; });
    check ([&] (auto& x) { x = x - b; }, [] (Complex p, Complex q) { return p - q; });
    check ([&] (auto& x) { x = x * TestType (3); }, [] (Complex p, Complex) { return p * TestType (3); });
    check ([&] (auto& x) { x = vctr::splitComplexConjugate (x); }, [] (Complex p, Complex) { return std::conj (p); });

    SECTION ("Non aliased large evaluation")
    {
        const auto a = makeSplit (dataA);
        vctr::SplitComplexVector<TestType> dst (n);
        dst = a * b;

        for (size_t i = 0; i < n; ++i)
            expected[i] = dataA[i] * dataB[i];

        requireClose (dst, expected);
    }
}