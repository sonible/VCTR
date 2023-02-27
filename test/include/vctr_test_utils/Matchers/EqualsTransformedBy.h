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

#ifdef DOXYGEN

/** Checks if a reference vector, element-wise transformed by function fn has the same element values as the vector it
    is matched against.
*/
template <T (*fn) (T), is::anyVctr ReferenceVec>
auto EqualsTransformedBy (const ReferenceVec& vec);

/** Checks if two vectors, element-wise transformed by function fn have the same element values as the vector it is
    matched against
*/
template <T (*fn) (T, T), is::anyVctr ReferenceVec>
auto EqualsTransformedBy (const ReferenceVec& vecA, const ReferenceVec& vecB);

/** Checks if a vector and a scalar, element-wise transformed by function fn have the same element values as the vector
    it is matched against
*/
template <T (*fn) (T, T), is::anyVctr ReferenceVec>
auto EqualsTransformedBy (const ReferenceVec& vec, T scalar);

/** Checks if a vector and a scalar, element-wise transformed by function fn have the same element values as the vector
    it is matched against
*/
template <T (*fn) (T, T), is::anyVctr ReferenceVec>
auto EqualsTransformedBy (T scalar, const ReferenceVec& vec);

#else

namespace detail
{
template <class RetValueType, class SrcValueType, is::anyVctr ReferenceVec, RetValueType (*transformingFn) (SrcValueType)>
struct UnaryEqualsTransformedMatcher : Catch::Matchers::MatcherGenericBase
{
    UnaryEqualsTransformedMatcher (const ReferenceVec& vec)
        : reference (vec) {}

    template <is::anyVctr Vec>
    bool match (const Vec& vec) const
    {
        if (vec.size() != reference.size())
            return false;

        for (size_t i = 0; i < vec.size(); ++i)
        {
            auto v = transformingFn (reference[i]);
            if (Approx<RetValueType> (v).margin (margin).epsilon (epsilon) != vec[i])
                return false;
        }

        return true;
    }

    // clang-format off
        auto withMargin (double m) &&                                                                       { margin = m; return std::move (*this); }
        auto withEpsilon (double e = std::numeric_limits<vctr::RealType<RetValueType>>::epsilon() * 100) && { epsilon = e; return std::move (*this); }
    // clang-format on

    std::string describe() const override
    {
        std::ostringstream os;
        os << "Equals: " << vctr::functionName<RetValueType, SrcValueType, transformingFn>() << " (" << reference << ")";
        return os.str();
    }

private:
    const ReferenceVec& reference;

    double margin = 0.0;
    double epsilon = 0.0;
};

template <class ValueType, is::anyVctr ReferenceVecA, is::anyVctr ReferenceVecB, ValueType (*transformingFn) (ValueType, ValueType)>
struct BinaryEqualsTransformedMatcher : Catch::Matchers::MatcherGenericBase
{
    BinaryEqualsTransformedMatcher (const ReferenceVecA& vecA, const ReferenceVecB& vecB)
        : referenceA (vecA),
          referenceB (vecB) {}

    template <is::anyVctr Vec>
    bool match (const Vec& vec) const
    {
        if (referenceA.size() != referenceB.size())
            return false;

        if (vec.size() != referenceA.size())
            return false;

        for (size_t i = 0; i < vec.size(); ++i)
        {
            auto v = transformingFn (referenceA[i], referenceB[i]);
            if (Approx<ValueType> (v).margin (margin).epsilon (epsilon) != vec[i])
                return false;
        }

        return true;
    }

    // clang-format off
        auto withMargin (double m) &&                                                                    { margin = m; return std::move (*this); }
        auto withEpsilon (double e = std::numeric_limits<vctr::RealType<ValueType>>::epsilon() * 100) && { epsilon = e; return std::move (*this); }
    // clang-format on

    std::string describe() const override
    {
        std::ostringstream os;
        os << "Equals: " << vctr::functionName<ValueType, ValueType, ValueType, transformingFn>() << " (" << referenceA
           << ", " << referenceB << ")";
        return os.str();
    }

private:
    const ReferenceVecA& referenceA;
    const ReferenceVecB& referenceB;

    double margin = 0.0;
    double epsilon = 0.0;
};

template <class ValueType, is::anyVctr ReferenceVec, ValueType (*transformingFn) (ValueType, ValueType)>
struct BinaryScalarEqualsTransformedMatcher : Catch::Matchers::MatcherGenericBase
{
    BinaryScalarEqualsTransformedMatcher (const ReferenceVec& vec, ValueType scalar)
        : referenceVec (vec),
          referenceScalar (scalar),
          swapReferences (false)
    {}

    BinaryScalarEqualsTransformedMatcher (ValueType scalar, const ReferenceVec& vec)
        : referenceVec (vec),
          referenceScalar (scalar),
          swapReferences (true)
    {}

    template <is::anyVctr Vec>
    bool match (const Vec& vec) const
    {
        if (vec.size() != referenceVec.size())
            return false;

        for (size_t i = 0; i < vec.size(); ++i)
        {
            auto v = swapReferences ? transformingFn (referenceScalar, referenceVec[i])
                                    : transformingFn (referenceVec[i], referenceScalar);

            if (Approx<ValueType> (v).margin (margin).epsilon (epsilon) != vec[i])
                return false;
        }

        return true;
    }

    auto withMargin (double m) &&
    {
        margin = m;
        return std::move (*this);
    }
    auto withEpsilon (double e = std::numeric_limits<vctr::RealType<ValueType>>::epsilon() * 100) &&
    {
        epsilon = e;
        return std::move (*this);
    }

    std::string describe() const override
    {
        std::ostringstream os;
        os << "Equals: " << vctr::functionName<ValueType, ValueType, ValueType, transformingFn>() << " (" << referenceVec
           << ", " << referenceScalar << ")";
        return os.str();
    }

private:
    const ReferenceVec& referenceVec;
    const ValueType referenceScalar;
    const bool swapReferences;

    double margin = 0.0;
    double epsilon = 0.0;
};
} // namespace detail

#define VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE(T)                                                                 \
                                                                                                                     \
    template <T (*fn) (T), is::anyVctr ReferenceVec>                                                                 \
    requires std::same_as<T, std::remove_const_t<typename ReferenceVec::value_type>>                                 \
    auto EqualsTransformedBy (const ReferenceVec& vec)                                                               \
    {                                                                                                                \
        return detail::UnaryEqualsTransformedMatcher<T, T, ReferenceVec, fn> (vec);                                  \
    }                                                                                                                \
                                                                                                                     \
    template <T (*fn) (const std::complex<T>&), is::anyVctr ReferenceVec>                                            \
    requires std::floating_point<T> &&                                                                               \
             std::same_as<std::complex<T>, std::remove_const_t<typename ReferenceVec::value_type>>                   \
    auto EqualsTransformedBy (const ReferenceVec& vec)                                                               \
    {                                                                                                                \
        return detail::UnaryEqualsTransformedMatcher<T, const std::complex<T>&, ReferenceVec, fn> (vec);             \
    }                                                                                                                \
                                                                                                                     \
    template <T (*fn) (std::complex<T>), is::anyVctr ReferenceVec>                                                   \
    requires std::floating_point<T> &&                                                                               \
             std::same_as<std::complex<T>, std::remove_const_t<typename ReferenceVec::value_type>>                   \
    auto EqualsTransformedBy (const ReferenceVec& vec)                                                               \
    {                                                                                                                \
        return detail::UnaryEqualsTransformedMatcher<T, std::complex<T>, ReferenceVec, fn> (vec);                    \
    }                                                                                                                \
                                                                                                                     \
    template <std::complex<T> (*fn) (std::complex<T>), is::anyVctr ReferenceVec>                                     \
    requires std::floating_point<T> &&                                                                               \
             std::same_as<std::complex<T>, std::remove_const_t<typename ReferenceVec::value_type>>                   \
    auto EqualsTransformedBy (const ReferenceVec& vec)                                                               \
    {                                                                                                                \
        return detail::UnaryEqualsTransformedMatcher<std::complex<T>, std::complex<T>, ReferenceVec, fn> (vec);      \
    }                                                                                                                \
                                                                                                                     \
    template <T (*fn) (T, T), is::anyVctr ReferenceVec>                                                              \
    requires std::same_as<T, std::remove_const_t<typename ReferenceVec::value_type>>                                 \
    auto EqualsTransformedBy (const ReferenceVec& vecA, const ReferenceVec& vecB)                                    \
    {                                                                                                                \
        return detail::BinaryEqualsTransformedMatcher<T, ReferenceVec, ReferenceVec, fn> (vecA, vecB);               \
    }                                                                                                                \
                                                                                                                     \
    template <T (*fn) (T, T), is::anyVctr ReferenceVec>                                                              \
    requires std::same_as<T, std::remove_const_t<typename ReferenceVec::value_type>>                                 \
    auto EqualsTransformedBy (const ReferenceVec& vec, T scalar)                                                     \
    {                                                                                                                \
        return detail::BinaryScalarEqualsTransformedMatcher<T, ReferenceVec, fn> (vec, scalar);                      \
    }                                                                                                                \
                                                                                                                     \
    template <T (*fn) (T, T), is::anyVctr ReferenceVec>                                                              \
    requires std::same_as<T, std::remove_const_t<typename ReferenceVec::value_type>>                                 \
    auto EqualsTransformedBy (T scalar, const ReferenceVec& vec)                                                     \
    {                                                                                                                \
        return detail::BinaryScalarEqualsTransformedMatcher<T, ReferenceVec, fn> (scalar, vec);                      \
    }                                                                                                                \
                                                                                                                     \
    template <std::complex<T> (*fn) (std::complex<T>, std::complex<T>), is::anyVctr ReferenceVec>                    \
    requires std::floating_point<T> &&                                                                               \
             std::same_as<std::complex<T>, std::remove_const_t<typename ReferenceVec::value_type>>                   \
    auto EqualsTransformedBy (const ReferenceVec& vecA, const ReferenceVec& vecB)                                    \
    {                                                                                                                \
        return detail::BinaryEqualsTransformedMatcher<std::complex<T>, ReferenceVec, ReferenceVec, fn> (vecA, vecB); \
    }                                                                                                                \
                                                                                                                     \
    template <std::complex<T> (*fn) (std::complex<T>, std::complex<T>), is::anyVctr ReferenceVec>                    \
    requires std::floating_point<T> &&                                                                               \
             std::same_as<std::complex<T>, std::remove_const_t<typename ReferenceVec::value_type>>                   \
    auto EqualsTransformedBy (const ReferenceVec& vec, std::complex<T> scalar)                                       \
    {                                                                                                                \
        return detail::BinaryScalarEqualsTransformedMatcher<std::complex<T>, ReferenceVec, fn> (vec, scalar);        \
    }                                                                                                                \
                                                                                                                     \
    template <std::complex<T> (*fn) (std::complex<T>, std::complex<T>), is::anyVctr ReferenceVec>                    \
    requires std::floating_point<T> &&                                                                               \
             std::same_as<std::complex<T>, std::remove_const_t<typename ReferenceVec::value_type>>                   \
    auto EqualsTransformedBy (std::complex<T> scalar, const ReferenceVec& vec)                                       \
    {                                                                                                                \
        return detail::BinaryScalarEqualsTransformedMatcher<std::complex<T>, ReferenceVec, fn> (scalar, vec);        \
    }

#define VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING(SrcT, DstT)                  \
                                                                                          \
    template <DstT (*fn) (SrcT), is::anyVctr ReferenceVec>                                \
    requires std::same_as<SrcT, std::remove_const_t<typename ReferenceVec::value_type>>   \
    auto EqualsTransformedBy (const ReferenceVec& vec)                                    \
    {                                                                                     \
        return detail::UnaryEqualsTransformedMatcher<DstT, SrcT, ReferenceVec, fn> (vec); \
    }

VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (float)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (double)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (int32_t)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (uint32_t)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (int64_t)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (uint64_t)

VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (int32_t, double)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (uint32_t, double)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (int64_t, double)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (uint64_t, double)

#endif
} // namespace vctr
