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

template <class T>
double defaultEpsilon = 0.0;

template <is::number T>
double defaultEpsilon<T> = std::numeric_limits<vctr::RealType<T>>::epsilon() * 100.0;

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

            if constexpr (vctr::is::number<RetValueType>)
            {
                if (Approx<RetValueType> (v).margin (margin).epsilon (epsilon) != vec[i])
                    return false;
            }
            else
            {
                if (v != vec[i])
                    return false;
            }
        }

        return true;
    }

    auto withMargin (double m) &&
    {
        margin = m;
        return std::move (*this);
    }

    auto withEpsilon (double e = defaultEpsilon<RetValueType>) &&
    {
        epsilon = e;
        return std::move (*this);
    }

    std::string describe() const override
    {
        Vector<RetValueType> refResult (reference.size());

        std::transform (reference.begin(), reference.end(), refResult.begin(), transformingFn);

        std::ostringstream os;
        os << "Equals: " << vctr::functionName<RetValueType, SrcValueType, transformingFn>() << " (" << reference << ") = " << refResult;
        return os.str();
    }

private:
    const ReferenceVec& reference;

    double margin = 0.0;
    double epsilon = 0.0;
};

template <class RetValueType, class SrcValueType, is::anyVctr ReferenceVecA, is::anyVctr ReferenceVecB, RetValueType (*transformingFn) (SrcValueType, SrcValueType)>
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
            if (Approx<RetValueType> (v).margin (margin).epsilon (epsilon) != vec[i])
                return false;
        }

        return true;
    }

    auto withMargin (double m) &&
    {
        margin = m;
        return std::move (*this);
    }

    auto withEpsilon (double e = defaultEpsilon<RetValueType>) &&
    {
        epsilon = e;
        return std::move (*this);
    }

    std::string describe() const override
    {
        Vector<RetValueType> refResult (referenceA.size());

        std::transform (referenceA.begin(), referenceA.end(), referenceB.begin(), refResult.begin(), transformingFn);

        std::ostringstream os;
        os << "Equals: " << vctr::functionName<RetValueType, SrcValueType, SrcValueType, transformingFn>() << " (" << referenceA
           << ", " << referenceB << ") = " << refResult;
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
          scalarBeforeVec (false),
          fnWithBoundScalar ([=] (ValueType v) { return transformingFn (v, scalar); })
    {}

    BinaryScalarEqualsTransformedMatcher (ValueType scalar, const ReferenceVec& vec)
        : referenceVec (vec),
          referenceScalar (scalar),
          scalarBeforeVec (true),
          fnWithBoundScalar ([=] (ValueType v) { return transformingFn (scalar, v); })
    {}

    template <is::anyVctr Vec>
    bool match (const Vec& vec) const
    {
        if (vec.size() != referenceVec.size())
            return false;

        for (size_t i = 0; i < vec.size(); ++i)
        {
            auto v = fnWithBoundScalar (referenceVec[i]);

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
    auto withEpsilon (double e = defaultEpsilon<ValueType>) &&
    {
        epsilon = e;
        return std::move (*this);
    }

    std::string describe() const override
    {
        Vector<ValueType> refResult (referenceVec.size());

        std::transform (referenceVec.begin(), referenceVec.end(), refResult.begin(), fnWithBoundScalar);

        std::ostringstream os;
        os << "Equals: " << vctr::functionName<ValueType, ValueType, ValueType, transformingFn>() << " (";

        if (scalarBeforeVec)
            os << referenceScalar << ", " << referenceVec;
        else
            os << referenceVec << ", " << referenceScalar;

        os << ") = " << refResult;

        return os.str();
    }

private:
    const ReferenceVec& referenceVec;
    const ValueType referenceScalar;
    const bool scalarBeforeVec;
    const std::function<ValueType (ValueType)> fnWithBoundScalar;

    double margin = 0.0;
    double epsilon = 0.0;
};
} // namespace detail

#define VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE(T)                                                                                  \
                                                                                                                                      \
    template <T (*fn) (T), is::anyVctr ReferenceVec>                                                                                  \
    requires std::same_as<T, std::remove_const_t<typename ReferenceVec::value_type>>                                                  \
    auto EqualsTransformedBy (const ReferenceVec& vec)                                                                                \
    {                                                                                                                                 \
        return detail::UnaryEqualsTransformedMatcher<T, T, ReferenceVec, fn> (vec);                                                   \
    }                                                                                                                                 \
                                                                                                                                      \
    template <T (*fn) (const std::complex<T>&), is::anyVctr ReferenceVec>                                                             \
    requires std::floating_point<T> &&                                                                                                \
             std::same_as<std::complex<T>, std::remove_const_t<typename ReferenceVec::value_type>>                                    \
    auto EqualsTransformedBy (const ReferenceVec& vec)                                                                                \
    {                                                                                                                                 \
        return detail::UnaryEqualsTransformedMatcher<T, const std::complex<T>&, ReferenceVec, fn> (vec);                              \
    }                                                                                                                                 \
                                                                                                                                      \
    template <T (*fn) (std::complex<T>), is::anyVctr ReferenceVec>                                                                    \
    requires std::floating_point<T> &&                                                                                                \
             std::same_as<std::complex<T>, std::remove_const_t<typename ReferenceVec::value_type>>                                    \
    auto EqualsTransformedBy (const ReferenceVec& vec)                                                                                \
    {                                                                                                                                 \
        return detail::UnaryEqualsTransformedMatcher<T, std::complex<T>, ReferenceVec, fn> (vec);                                     \
    }                                                                                                                                 \
                                                                                                                                      \
    template <std::complex<T> (*fn) (std::complex<T>), is::anyVctr ReferenceVec>                                                      \
    requires std::floating_point<T> &&                                                                                                \
             std::same_as<std::complex<T>, std::remove_const_t<typename ReferenceVec::value_type>>                                    \
    auto EqualsTransformedBy (const ReferenceVec& vec)                                                                                \
    {                                                                                                                                 \
        return detail::UnaryEqualsTransformedMatcher<std::complex<T>, std::complex<T>, ReferenceVec, fn> (vec);                       \
    }                                                                                                                                 \
                                                                                                                                      \
    template <T (*fn) (T, T), is::anyVctr ReferenceVec>                                                                               \
    requires std::same_as<T, std::remove_const_t<typename ReferenceVec::value_type>>                                                  \
    auto EqualsTransformedBy (const ReferenceVec& vecA, const ReferenceVec& vecB)                                                     \
    {                                                                                                                                 \
        return detail::BinaryEqualsTransformedMatcher<T, T, ReferenceVec, ReferenceVec, fn> (vecA, vecB);                             \
    }                                                                                                                                 \
                                                                                                                                      \
    template <T (*fn) (T, T), is::anyVctr ReferenceVec>                                                                               \
    requires std::same_as<T, std::remove_const_t<typename ReferenceVec::value_type>>                                                  \
    auto EqualsTransformedBy (const ReferenceVec& vec, T scalar)                                                                      \
    {                                                                                                                                 \
        return detail::BinaryScalarEqualsTransformedMatcher<T, ReferenceVec, fn> (vec, scalar);                                       \
    }                                                                                                                                 \
                                                                                                                                      \
    template <T (*fn) (T, T), is::anyVctr ReferenceVec>                                                                               \
    requires std::same_as<T, std::remove_const_t<typename ReferenceVec::value_type>>                                                  \
    auto EqualsTransformedBy (T scalar, const ReferenceVec& vec)                                                                      \
    {                                                                                                                                 \
        return detail::BinaryScalarEqualsTransformedMatcher<T, ReferenceVec, fn> (scalar, vec);                                       \
    }                                                                                                                                 \
                                                                                                                                      \
    template <std::complex<T> (*fn) (std::complex<T>, std::complex<T>), is::anyVctr ReferenceVec>                                     \
    requires std::floating_point<T> &&                                                                                                \
             std::same_as<std::complex<T>, std::remove_const_t<typename ReferenceVec::value_type>>                                    \
    auto EqualsTransformedBy (const ReferenceVec& vecA, const ReferenceVec& vecB)                                                     \
    {                                                                                                                                 \
        return detail::BinaryEqualsTransformedMatcher<std::complex<T>, std::complex<T>, ReferenceVec, ReferenceVec, fn> (vecA, vecB); \
    }                                                                                                                                 \
                                                                                                                                      \
    template <std::complex<T> (*fn) (std::complex<T>, std::complex<T>), is::anyVctr ReferenceVec>                                     \
    requires std::floating_point<T> &&                                                                                                \
             std::same_as<std::complex<T>, std::remove_const_t<typename ReferenceVec::value_type>>                                    \
    auto EqualsTransformedBy (const ReferenceVec& vec, std::complex<T> scalar)                                                        \
    {                                                                                                                                 \
        return detail::BinaryScalarEqualsTransformedMatcher<std::complex<T>, ReferenceVec, fn> (vec, scalar);                         \
    }                                                                                                                                 \
                                                                                                                                      \
    template <std::complex<T> (*fn) (std::complex<T>, std::complex<T>), is::anyVctr ReferenceVec>                                     \
    requires std::floating_point<T> &&                                                                                                \
             std::same_as<std::complex<T>, std::remove_const_t<typename ReferenceVec::value_type>>                                    \
    auto EqualsTransformedBy (std::complex<T> scalar, const ReferenceVec& vec)                                                        \
    {                                                                                                                                 \
        return detail::BinaryScalarEqualsTransformedMatcher<std::complex<T>, ReferenceVec, fn> (scalar, vec);                         \
    }

#define VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING(SrcT, DstT)                                        \
                                                                                                                \
    template <DstT (*fn) (SrcT), is::anyVctr ReferenceVec>                                                      \
    requires std::same_as<SrcT, std::remove_const_t<typename ReferenceVec::value_type>>                         \
    auto EqualsTransformedBy (const ReferenceVec& vec)                                                          \
    {                                                                                                           \
        return detail::UnaryEqualsTransformedMatcher<DstT, SrcT, ReferenceVec, fn> (vec);                       \
    }                                                                                                           \
                                                                                                                \
    template <DstT (*fn) (SrcT, SrcT), is::anyVctr ReferenceVec>                                                \
    requires std::same_as<SrcT, std::remove_const_t<typename ReferenceVec::value_type>>                         \
    auto EqualsTransformedBy (const ReferenceVec& vecA, const ReferenceVec& vecB)                               \
    {                                                                                                           \
        return detail::BinaryEqualsTransformedMatcher<DstT, SrcT, ReferenceVec, ReferenceVec, fn> (vecA, vecB); \
    }

VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (float)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (double)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (int16_t)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (int32_t)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (uint32_t)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (int64_t)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE (uint64_t)

VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (float, double)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (int16_t, double)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (int32_t, double)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (uint32_t, double)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (int64_t, double)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (uint64_t, double)

VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (double, int16_t)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (float, int16_t)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (int64_t, int16_t)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (int32_t, int16_t)

VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (double, std::string)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (float, std::string)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (int64_t, std::string)
VCTR_DEFINE_EQUAL_TRANSFORMED_BY_FOR_TYPE_CONVERTING (int32_t, std::string)

#endif
} // namespace vctr
