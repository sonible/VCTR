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
struct AVXRegister
{
    static constexpr AVXRegister broadcast (const T&) { return {}; }
};

#if VCTR_X64

template <>
struct AVXRegister<float>
{
    static constexpr size_t numElements = 8;

    using NativeType = __m256;
    __m256 value;

    //==============================================================================
    // Loading
    // clang-format off
    VCTR_TARGET ("avx") static AVXRegister loadUnaligned (const float* d)                             { return { _mm256_loadu_ps (d) }; }
    VCTR_TARGET ("avx") static AVXRegister loadAligned   (const float* d)                             { return { _mm256_load_ps (d) }; }
    VCTR_TARGET ("avx") static AVXRegister broadcast     (float x)                                    { return { _mm256_broadcast_ss (&x) }; }
    VCTR_TARGET ("avx") static AVXRegister fromSSE       (SSERegister<float> a, SSERegister<float> b) { return { _mm256_set_m128 (a.value, b.value) }; }

    //==============================================================================
    // Storing
    VCTR_TARGET ("avx") void storeUnaligned (float* d) const { _mm256_storeu_ps (d, value); }
    VCTR_TARGET ("avx") void storeAligned   (float* d) const { _mm256_store_ps  (d, value); }

    //==============================================================================
    // Bit Operations
    VCTR_TARGET ("avx") static AVXRegister andNot (AVXRegister a, AVXRegister b) { return { _mm256_andnot_ps (a.value, b.value) }; }

    //==============================================================================
    // Math
    VCTR_TARGET ("avx") static AVXRegister mul (AVXRegister a, AVXRegister b) { return { _mm256_mul_ps (a.value, b.value) }; }
    VCTR_TARGET ("avx") static AVXRegister add (AVXRegister a, AVXRegister b) { return { _mm256_add_ps (a.value, b.value) }; }
    VCTR_TARGET ("avx") static AVXRegister sub (AVXRegister a, AVXRegister b) { return { _mm256_sub_ps (a.value, b.value) }; }
    VCTR_TARGET ("avx") static AVXRegister div (AVXRegister a, AVXRegister b) { return { _mm256_div_ps (a.value, b.value) }; }
    VCTR_TARGET ("avx") static AVXRegister max (AVXRegister a, AVXRegister b) { return { _mm256_max_ps (a.value, b.value) }; }
    VCTR_TARGET ("avx") static AVXRegister min (AVXRegister a, AVXRegister b) { return { _mm256_min_ps (a.value, b.value) }; }
    // clang-format on
};

template <>
struct AVXRegister<double>
{
    static constexpr size_t numElements = 4;

    using NativeType = __m256d;
    __m256d value;

    //==============================================================================
    // Loading
    // clang-format off
    VCTR_TARGET ("avx") static AVXRegister loadUnaligned (const double* d)                              { return { _mm256_loadu_pd (d) }; }
    VCTR_TARGET ("avx") static AVXRegister loadAligned   (const double* d)                              { return { _mm256_load_pd (d) }; }
    VCTR_TARGET ("avx") static AVXRegister broadcast     (double x) { return                            { _mm256_broadcast_sd (&x) }; }
    VCTR_TARGET ("avx") static AVXRegister fromSSE       (SSERegister<double> a, SSERegister<double> b) { return { _mm256_set_m128d (a.value, b.value) }; }

    //==============================================================================
    // Storing
    VCTR_TARGET ("avx") void storeUnaligned (double* d) const { _mm256_storeu_pd (d, value); }
    VCTR_TARGET ("avx") void storeAligned   (double* d) const { _mm256_store_pd (d, value); }

    //==============================================================================
    // Bit Operations
    VCTR_TARGET ("avx") static AVXRegister andNot (AVXRegister a, AVXRegister b) { return { _mm256_andnot_pd (a.value, b.value) }; }

    //==============================================================================
    // Math
    VCTR_TARGET ("avx") static AVXRegister mul (AVXRegister a, AVXRegister b) { return { _mm256_mul_pd (a.value, b.value) }; }
    VCTR_TARGET ("avx") static AVXRegister add (AVXRegister a, AVXRegister b) { return { _mm256_add_pd (a.value, b.value) }; }
    VCTR_TARGET ("avx") static AVXRegister sub (AVXRegister a, AVXRegister b) { return { _mm256_sub_pd (a.value, b.value) }; }
    VCTR_TARGET ("avx") static AVXRegister div (AVXRegister a, AVXRegister b) { return { _mm256_div_pd (a.value, b.value) }; }
    VCTR_TARGET ("avx") static AVXRegister max (AVXRegister a, AVXRegister b) { return { _mm256_max_pd (a.value, b.value) }; }
    VCTR_TARGET ("avx") static AVXRegister min (AVXRegister a, AVXRegister b) { return { _mm256_min_pd (a.value, b.value) }; }
    // clang-format on
};

template <>
struct AVXRegister<int32_t>
{
    static constexpr size_t numElements = 8;

    using NativeType = __m256i;
    __m256i value;

    //==============================================================================
    // Loading
    // clang-format off
    VCTR_TARGET ("avx") static AVXRegister loadUnaligned (const int32_t* d)                               { return { _mm256_loadu_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_TARGET ("avx") static AVXRegister loadAligned   (const int32_t* d)                               { return { _mm256_load_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_TARGET ("avx") static AVXRegister broadcast     (int32_t x)                                      { return { _mm256_set1_epi32 (x) }; }
    VCTR_TARGET ("avx") static AVXRegister fromSSE       (SSERegister<int32_t> a, SSERegister<int32_t> b) { return { _mm256_set_m128i (a.value, b.value) }; }

    //==============================================================================
    // Storing
    VCTR_TARGET ("avx") void storeUnaligned (int32_t* d) const { _mm256_storeu_si256 (reinterpret_cast<__m256i*> (d), value); }
    VCTR_TARGET ("avx") void storeAligned   (int32_t* d) const { _mm256_store_si256  (reinterpret_cast<__m256i*> (d), value); }

    //==============================================================================
    // Bit Operations

    //==============================================================================
    // Math
    VCTR_TARGET ("avx2") static AVXRegister abs (AVXRegister x)                { return { _mm256_abs_epi32 (x.value) }; }
    VCTR_TARGET ("avx2") static AVXRegister add (AVXRegister a, AVXRegister b) { return { _mm256_add_epi32 (a.value, b.value) }; }
    VCTR_TARGET ("avx2") static AVXRegister sub (AVXRegister a, AVXRegister b) { return { _mm256_sub_epi32 (a.value, b.value) }; }
    VCTR_TARGET ("avx2") static AVXRegister max (AVXRegister a, AVXRegister b) { return { _mm256_max_epi32 (a.value, b.value) }; }
    VCTR_TARGET ("avx2") static AVXRegister min (AVXRegister a, AVXRegister b) { return { _mm256_min_epi32 (a.value, b.value) }; }
    // clang-format on
};

template <>
struct AVXRegister<uint32_t>
{
    static constexpr size_t numElements = 8;

    using NativeType = __m256i;
    __m256i value;

    //==============================================================================
    // Loading
    // clang-format off
    VCTR_TARGET ("avx") static AVXRegister loadUnaligned (const uint32_t* d)                                { return { _mm256_loadu_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_TARGET ("avx") static AVXRegister loadAligned   (const uint32_t* d)                                { return { _mm256_load_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_TARGET ("avx") static AVXRegister broadcast     (uint32_t x)                                       { return { _mm256_set1_epi32 ((int32_t) x) }; }
    VCTR_TARGET ("avx") static AVXRegister fromSSE       (SSERegister<uint32_t> a, SSERegister<uint32_t> b) { return { _mm256_set_m128i (a.value, b.value) }; }

    //==============================================================================
    // Storing
    VCTR_TARGET ("avx") void storeUnaligned (uint32_t* d) const { _mm256_storeu_si256 (reinterpret_cast<__m256i*> (d), value); }
    VCTR_TARGET ("avx") void storeAligned   (uint32_t* d) const { _mm256_store_si256  (reinterpret_cast<__m256i*> (d), value); }

    //==============================================================================
    // Bit Operations

    //==============================================================================
    // Math
    VCTR_TARGET ("avx2") static AVXRegister add (AVXRegister a, AVXRegister b) { return { _mm256_add_epi32 (a.value, b.value) }; }
    VCTR_TARGET ("avx2") static AVXRegister sub (AVXRegister a, AVXRegister b) { return { _mm256_sub_epi32 (a.value, b.value) }; }
    VCTR_TARGET ("avx2") static AVXRegister max (AVXRegister a, AVXRegister b) { return { _mm256_max_epu32 (a.value, b.value) }; }
    VCTR_TARGET ("avx2") static AVXRegister min (AVXRegister a, AVXRegister b) { return { _mm256_min_epu32 (a.value, b.value) }; }
    // clang-format on
};

template <>
struct AVXRegister<int64_t>
{
    static constexpr size_t numElements = 4;

    using NativeType = __m256i;
    __m256i value;

    //==============================================================================
    // Loading
    // clang-format off
    VCTR_TARGET ("avx") static AVXRegister loadUnaligned (const int64_t* d)                               { return { _mm256_loadu_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_TARGET ("avx") static AVXRegister loadAligned   (const int64_t* d)                               { return { _mm256_load_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_TARGET ("avx") static AVXRegister broadcast     (int64_t x)                                      { return { _mm256_set1_epi64x (x) }; }
    VCTR_TARGET ("avx") static AVXRegister fromSSE       (SSERegister<int64_t> a, SSERegister<int64_t> b) { return { _mm256_set_m128i (a.value, b.value) }; }

    //==============================================================================
    // Storing
    VCTR_TARGET ("avx") void storeUnaligned (int64_t* d) const { _mm256_storeu_si256 (reinterpret_cast<__m256i*> (d), value); }
    VCTR_TARGET ("avx") void storeAligned   (int64_t* d) const { _mm256_store_si256  (reinterpret_cast<__m256i*> (d), value); }

    //==============================================================================
    // Bit Operations

    //==============================================================================
    // Math
    VCTR_TARGET ("avx2") static AVXRegister add (AVXRegister a, AVXRegister b) { return { _mm256_add_epi64 (a.value, b.value) }; }
    VCTR_TARGET ("avx2") static AVXRegister sub (AVXRegister a, AVXRegister b) { return { _mm256_sub_epi64 (a.value, b.value) }; }
    // clang-format on
};

template <>
struct AVXRegister<uint64_t>
{
    static constexpr size_t numElements = 4;

    using NativeType = __m256i;
    __m256i value;

    //==============================================================================
    // Loading
    // clang-format off
    VCTR_TARGET ("avx") static AVXRegister loadUnaligned (const uint64_t* d)                                { return { _mm256_loadu_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_TARGET ("avx") static AVXRegister loadAligned   (const uint64_t* d)                                { return { _mm256_load_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_TARGET ("avx") static AVXRegister broadcast     (uint64_t x)                                       { return { _mm256_set1_epi64x ((int64_t) x) }; }
    VCTR_TARGET ("avx") static AVXRegister fromSSE       (SSERegister<uint64_t> a, SSERegister<uint64_t> b) { return { _mm256_set_m128i (a.value, b.value) }; }

    //==============================================================================
    // Storing
    VCTR_TARGET ("avx") void storeUnaligned (uint64_t* d) const { _mm256_storeu_si256 (reinterpret_cast<__m256i*> (d), value); }
    VCTR_TARGET ("avx") void storeAligned   (uint64_t* d) const { _mm256_store_si256  (reinterpret_cast<__m256i*> (d), value); }

    //==============================================================================
    // Bit Operations

    //==============================================================================
    // Math
    VCTR_TARGET ("avx2") static AVXRegister add (AVXRegister a, AVXRegister b) { return { _mm256_add_epi64 (a.value, b.value) }; }
    VCTR_TARGET ("avx2") static AVXRegister sub (AVXRegister a, AVXRegister b) { return { _mm256_sub_epi64 (a.value, b.value) }; }
    // clang-format on
};

#endif

} // namespace vctr
