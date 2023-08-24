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

template <class T>
struct SSERegister
{
    static constexpr SSERegister broadcast (const T&) { return {}; }
};

#if VCTR_X64

template <>
struct SSERegister<float>
{
    static constexpr size_t numElements = 4;

    using NativeType = __m128;
    __m128 value;

    //==============================================================================
    // Loading
    // clang-format off
    VCTR_TARGET ("sse4.1") static SSERegister loadUnaligned (const float* d)  { return { _mm_loadu_ps (d) }; }
    VCTR_TARGET ("sse4.1") static SSERegister loadAligned   (const float* d)  { return { _mm_load_ps (d) }; }
    VCTR_TARGET ("sse4.1") static SSERegister broadcast     (float x)         { return { _mm_load1_ps (&x) }; }

    //==============================================================================
    // Storing
    VCTR_TARGET ("sse4.1") void storeUnaligned (float* d) const { _mm_storeu_ps (d, value); }
    VCTR_TARGET ("sse4.1") void storeAligned   (float* d) const { _mm_store_ps (d, value); }

    //==============================================================================
    // Bit Operations
    VCTR_TARGET ("sse4.1") static SSERegister andNot (SSERegister a, SSERegister b) { return { _mm_andnot_ps (a.value, b.value) }; }

    //==============================================================================
    // Math
    VCTR_TARGET ("sse4.1") static SSERegister mul (SSERegister a, SSERegister b) { return { _mm_mul_ps (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister div (SSERegister a, SSERegister b) { return { _mm_div_ps (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister add (SSERegister a, SSERegister b) { return { _mm_add_ps (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister sub (SSERegister a, SSERegister b) { return { _mm_sub_ps (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister max (SSERegister a, SSERegister b) { return { _mm_max_ps (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister min (SSERegister a, SSERegister b) { return { _mm_min_ps (a.value, b.value) }; }
    // clang-format on
};

template <>
struct SSERegister<double>
{
    static constexpr size_t numElements = 2;

    using NativeType = __m128d;
    __m128d value;

    //==============================================================================
    // Loading
    // clang-format off
    VCTR_TARGET ("sse4.1") static SSERegister loadUnaligned (const double* d)  { return { _mm_loadu_pd (d) }; }
    VCTR_TARGET ("sse4.1") static SSERegister loadAligned   (const double* d)  { return { _mm_load_pd (d) }; }
    VCTR_TARGET ("sse4.1") static SSERegister broadcast     (double x)         { return { _mm_load1_pd (&x) }; }

    //==============================================================================
    // Storing
    VCTR_TARGET ("sse4.1") void storeUnaligned (double* d) const { _mm_storeu_pd (d, value); }
    VCTR_TARGET ("sse4.1") void storeAligned   (double* d) const { _mm_store_pd (d, value); }

    //==============================================================================
    // Bit Operations
    VCTR_TARGET ("sse4.1") static SSERegister andNot (SSERegister a, SSERegister b) { return { _mm_andnot_pd (a.value, b.value) }; }

    //==============================================================================
    // Math
    VCTR_TARGET ("sse4.1") static SSERegister mul (SSERegister a, SSERegister b) { return { _mm_mul_pd (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister div (SSERegister a, SSERegister b) { return { _mm_div_pd (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister add (SSERegister a, SSERegister b) { return { _mm_add_pd (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister sub (SSERegister a, SSERegister b) { return { _mm_sub_pd (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister max (SSERegister a, SSERegister b) { return { _mm_max_pd (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister min (SSERegister a, SSERegister b) { return { _mm_min_pd (a.value, b.value) }; }
    // clang-format on
};

template <>
struct SSERegister<int32_t>
{
    static constexpr size_t numElements = 4;

    using NativeType = __m128i;
    __m128i value;

    //==============================================================================
    // Loading
    // clang-format off
    VCTR_TARGET ("sse4.1") static SSERegister loadUnaligned (const int32_t* d)  { return { _mm_loadu_si128 (reinterpret_cast<const __m128i*> (d)) }; }
    VCTR_TARGET ("sse4.1") static SSERegister loadAligned   (const int32_t* d)  { return { _mm_load_si128 (reinterpret_cast<const __m128i*> (d)) }; }
    VCTR_TARGET ("sse4.1") static SSERegister broadcast     (int32_t x)         { return { _mm_set1_epi32 (x) }; }

    //==============================================================================
    // Storing
    VCTR_TARGET ("sse4.1") void storeUnaligned (int32_t* d) const { _mm_storeu_si128 (reinterpret_cast<__m128i*> (d), value); }
    VCTR_TARGET ("sse4.1") void storeAligned (int32_t* d)   const { _mm_store_si128 (reinterpret_cast<__m128i*> (d), value); }

    //==============================================================================
    // Bit Operations

    //==============================================================================
    // Math
    VCTR_TARGET ("sse4.1") static SSERegister abs (SSERegister x)                { return { _mm_abs_epi32 (x.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister add (SSERegister a, SSERegister b) { return { _mm_add_epi32 (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister sub (SSERegister a, SSERegister b) { return { _mm_sub_epi32 (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister max (SSERegister a, SSERegister b) { return { _mm_max_epi32 (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister min (SSERegister a, SSERegister b) { return { _mm_min_epi32 (a.value, b.value) }; }
    // clang-format on
};

template <>
struct SSERegister<uint32_t>
{
    static constexpr size_t numElements = 4;

    using NativeType = __m128i;
    __m128i value;

    //==============================================================================
    // Loading
    // clang-format off
    VCTR_TARGET ("sse4.1") static SSERegister loadUnaligned (const uint32_t* d)  { return { _mm_loadu_si128 (reinterpret_cast<const __m128i*> (d)) }; }
    VCTR_TARGET ("sse4.1") static SSERegister loadAligned   (const uint32_t* d)  { return { _mm_load_si128 (reinterpret_cast<const __m128i*> (d)) }; }
    VCTR_TARGET ("sse4.1") static SSERegister broadcast     (uint32_t x)         { return { _mm_set1_epi32 ((int32_t) x) }; }

    //==============================================================================
    // Storing
    VCTR_TARGET ("sse4.1") void storeUnaligned (uint32_t* d) const { _mm_storeu_si128 (reinterpret_cast<__m128i*> (d), value); }
    VCTR_TARGET ("sse4.1") void storeAligned (uint32_t* d)   const { _mm_store_si128 (reinterpret_cast<__m128i*> (d), value); }

    //==============================================================================
    // Bit Operations

    //==============================================================================
    // Math
    VCTR_TARGET ("sse4.1") static SSERegister add (SSERegister a, SSERegister b) { return { _mm_add_epi32 (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister sub (SSERegister a, SSERegister b) { return { _mm_sub_epi32 (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister max (SSERegister a, SSERegister b) { return { _mm_max_epu32 (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister min (SSERegister a, SSERegister b) { return { _mm_min_epu32 (a.value, b.value) }; }
    // clang-format on
};

template <>
struct SSERegister<int64_t>
{
    static constexpr size_t numElements = 2;

    using NativeType = __m128i;
    __m128i value;

    //==============================================================================
    // Loading
    // clang-format off
    VCTR_TARGET ("sse4.1") static SSERegister loadUnaligned (const int64_t* d)  { return { _mm_loadu_si128 (reinterpret_cast<const __m128i*> (d)) }; }
    VCTR_TARGET ("sse4.1") static SSERegister loadAligned   (const int64_t* d)  { return { _mm_load_si128 (reinterpret_cast<const __m128i*> (d)) }; }
    VCTR_TARGET ("sse4.1") static SSERegister broadcast     (int64_t x)         { return { _mm_set1_epi64x (x) }; }

    //==============================================================================
    // Storing
    VCTR_TARGET ("sse4.1") void storeUnaligned (int64_t* d) const { _mm_storeu_si128 (reinterpret_cast<__m128i*> (d), value); }
    VCTR_TARGET ("sse4.1") void storeAligned (int64_t* d)   const { _mm_store_si128 (reinterpret_cast<__m128i*> (d), value); }

    //==============================================================================
    // Bit Operations

    //==============================================================================
    // Math
    VCTR_TARGET ("sse4.1") static SSERegister add (SSERegister a, SSERegister b) { return { _mm_add_epi64 (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister sub (SSERegister a, SSERegister b) { return { _mm_sub_epi64 (a.value, b.value) }; }
    // clang-format on
};

template <>
struct SSERegister<uint64_t>
{
    static constexpr size_t numElements = 2;

    using NativeType = __m128i;
    __m128i value;

    //==============================================================================
    // Loading
    // clang-format off
    VCTR_TARGET ("sse4.1") static SSERegister loadUnaligned (const uint64_t* d)  { return { _mm_loadu_si128 (reinterpret_cast<const __m128i*> (d)) }; }
    VCTR_TARGET ("sse4.1") static SSERegister loadAligned   (const uint64_t* d)  { return { _mm_load_si128 (reinterpret_cast<const __m128i*> (d)) }; }
    VCTR_TARGET ("sse4.1") static SSERegister broadcast     (uint64_t x)         { return { _mm_set1_epi64x ((int64_t) x) }; }

    //==============================================================================
    // Storing
    VCTR_TARGET ("sse4.1") void storeUnaligned (uint64_t* d) const { _mm_storeu_si128 (reinterpret_cast<__m128i*> (d), value); }
    VCTR_TARGET ("sse4.1") void storeAligned (uint64_t* d)   const { _mm_store_si128 (reinterpret_cast<__m128i*> (d), value); }

    //==============================================================================
    // Bit Operations

    //==============================================================================
    // Math
    VCTR_TARGET ("sse4.1") static SSERegister add (SSERegister a, SSERegister b) { return { _mm_add_epi64 (a.value, b.value) }; }
    VCTR_TARGET ("sse4.1") static SSERegister sub (SSERegister a, SSERegister b) { return { _mm_sub_epi64 (a.value, b.value) }; }
    // clang-format on
};

#endif

} // namespace vctr
