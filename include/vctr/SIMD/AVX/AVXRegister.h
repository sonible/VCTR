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
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister loadUnaligned (const float* d)                             { return { _mm256_loadu_ps (d) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister loadAligned   (const float* d)                             { return { _mm256_load_ps (d) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister broadcast     (float x)                                    { return { _mm256_broadcast_ss (&x) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister fromSSE       (SSERegister<float> a, SSERegister<float> b) { return { _mm256_set_m128 (a.value, b.value) }; }

    //==============================================================================
    // Storing
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void storeUnaligned (float* d) const { _mm256_storeu_ps (d, value); }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void storeAligned   (float* d) const { _mm256_store_ps  (d, value); }

    //==============================================================================
    // Generate Compare Masks
    template <CompareOp Op>
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister compare (AVXRegister a, AVXRegister b) { return { _mm256_cmp_ps (a.value, b.value, int (Op)) }; }

    //==============================================================================
    // Bit Operations
    /** Evaluates a & (! b) - so b is negated. */
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister bitwiseAndNot (AVXRegister a, AVXRegister b) { return { _mm256_andnot_ps (b.value, a.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister bitwiseAnd (AVXRegister a, AVXRegister b) { return { _mm256_and_ps (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister bitwiseBlend (AVXRegister a, AVXRegister b, AVXRegister mask) { return { _mm256_blendv_ps (a.value, b.value, mask.value) }; }

    //==============================================================================
    // Math
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister floor (AVXRegister x)              { return { _mm256_floor_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister ceil (AVXRegister x)               { return { _mm256_ceil_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister mul (AVXRegister a, AVXRegister b) { return { _mm256_mul_ps (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister add (AVXRegister a, AVXRegister b) { return { _mm256_add_ps (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister sub (AVXRegister a, AVXRegister b) { return { _mm256_sub_ps (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister div (AVXRegister a, AVXRegister b) { return { _mm256_div_ps (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister max (AVXRegister a, AVXRegister b) { return { _mm256_max_ps (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister min (AVXRegister a, AVXRegister b) { return { _mm256_min_ps (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister fma (AVXRegister a, AVXRegister b, AVXRegister c) { return { _mm256_fmadd_ps (a.value, b.value, c.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister fms (AVXRegister a, AVXRegister b, AVXRegister c) { return { _mm256_fnmadd_ps (a.value, b.value, c.value) }; }

#if VCTR_HAS_SVML
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister exp   (AVXRegister x) { return {_mm256_exp_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister exp2  (AVXRegister x) { return {_mm256_exp2_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister expm1 (AVXRegister x) { return {_mm256_expm1_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister log   (AVXRegister x) { return {_mm256_log_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister log1p (AVXRegister x) { return {_mm256_log1p_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister log10 (AVXRegister x) { return {_mm256_log10_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister logb  (AVXRegister x) { return {_mm256_logb_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister log2  (AVXRegister x) { return {_mm256_log2_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister sin   (AVXRegister x) { return {_mm256_sin_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister cos   (AVXRegister x) { return {_mm256_cos_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister tan   (AVXRegister x) { return {_mm256_tan_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister sinh  (AVXRegister x) { return {_mm256_sinh_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister cosh  (AVXRegister x) { return {_mm256_cosh_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister tanh  (AVXRegister x) { return {_mm256_tanh_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister asinh (AVXRegister x) { return {_mm256_asinh_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister acosh (AVXRegister x) { return {_mm256_acosh_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister atanh (AVXRegister x) { return {_mm256_atanh_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister pow   (AVXRegister x, AVXRegister y) { return {_mm256_pow_ps (x.value, y.value) }; }
#endif

    //==============================================================================
    // Type conversion
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister<int32_t> convertToInt (AVXRegister x);
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister<int32_t> reinterpretAsInt (AVXRegister x);
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
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister loadUnaligned (const double* d)                              { return { _mm256_loadu_pd (d) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister loadAligned   (const double* d)                              { return { _mm256_load_pd (d) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister broadcast     (double x)                                     { return { _mm256_broadcast_sd (&x) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister fromSSE       (SSERegister<double> a, SSERegister<double> b) { return { _mm256_set_m128d (a.value, b.value) }; }

    //==============================================================================
    // Storing
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void storeUnaligned (double* d) const { _mm256_storeu_pd (d, value); }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void storeAligned   (double* d) const { _mm256_store_pd (d, value); }

    //==============================================================================
    // Generate Compare Masks
    template <CompareOp Op>
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister compare (AVXRegister a, AVXRegister b) { return { _mm256_cmp_pd (a.value, b.value, int (Op)) }; }

    //==============================================================================
    // Bit Operations
    /** Evaluates a & (! b) - so b is negated. */
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister bitwiseAndNot (AVXRegister a, AVXRegister b) { return { _mm256_andnot_pd (b.value, a.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister bitwiseAnd (AVXRegister a, AVXRegister b) { return { _mm256_and_pd (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister bitwiseBlend (AVXRegister a, AVXRegister b, AVXRegister mask) { return { _mm256_blendv_pd (a.value, b.value, mask.value) }; }

    //==============================================================================
    // Math
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister floor (AVXRegister x)              { return { _mm256_floor_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister ceil (AVXRegister x)               { return { _mm256_ceil_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister mul (AVXRegister a, AVXRegister b) { return { _mm256_mul_pd (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister add (AVXRegister a, AVXRegister b) { return { _mm256_add_pd (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister sub (AVXRegister a, AVXRegister b) { return { _mm256_sub_pd (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister div (AVXRegister a, AVXRegister b) { return { _mm256_div_pd (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister max (AVXRegister a, AVXRegister b) { return { _mm256_max_pd (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister min (AVXRegister a, AVXRegister b) { return { _mm256_min_pd (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister fma (AVXRegister a, AVXRegister b, AVXRegister c) { return { _mm256_fmadd_pd (a.value, b.value, c.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister fms (AVXRegister a, AVXRegister b, AVXRegister c) { return { _mm256_fnmadd_pd (a.value, b.value, c.value) }; }
    
#if VCTR_HAS_SVML
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister exp   (AVXRegister x) { return {_mm256_exp_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister exp2  (AVXRegister x) { return {_mm256_exp2_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister expm1 (AVXRegister x) { return {_mm256_expm1_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister log   (AVXRegister x) { return {_mm256_log_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister log1p (AVXRegister x) { return {_mm256_log1p_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister log10 (AVXRegister x) { return {_mm256_log10_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister logb  (AVXRegister x) { return {_mm256_logb_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister log2  (AVXRegister x) { return {_mm256_log2_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister sin   (AVXRegister x) { return {_mm256_sin_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister cos   (AVXRegister x) { return {_mm256_cos_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister tan   (AVXRegister x) { return {_mm256_tan_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister sinh  (AVXRegister x) { return {_mm256_sinh_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister cosh  (AVXRegister x) { return {_mm256_cosh_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister tanh  (AVXRegister x) { return {_mm256_tanh_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister asinh (AVXRegister x) { return {_mm256_asinh_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister acosh (AVXRegister x) { return {_mm256_acosh_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister atanh (AVXRegister x) { return {_mm256_atanh_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("fma") static AVXRegister pow   (AVXRegister x, AVXRegister y) { return {_mm256_pow_pd (x.value, y.value) }; }
#endif

    //==============================================================================
    // Type conversion
    VCTR_FORCEDINLINE VCTR_TARGET ("avx512vl") VCTR_TARGET ("avx512dq") static AVXRegister<int64_t> convertToInt (AVXRegister x);
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister<int64_t> reinterpretAsInt (AVXRegister x);
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
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister loadUnaligned (const int32_t* d)                               { return { _mm256_loadu_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister loadAligned   (const int32_t* d)                               { return { _mm256_load_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister broadcast     (int32_t x)                                      { return { _mm256_set1_epi32 (x) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister fromSSE       (SSERegister<int32_t> a, SSERegister<int32_t> b) { return { _mm256_set_m128i (a.value, b.value) }; }

    //==============================================================================
    // Storing
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void storeUnaligned (int32_t* d) const { _mm256_storeu_si256 (reinterpret_cast<__m256i*> (d), value); }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void storeAligned   (int32_t* d) const { _mm256_store_si256  (reinterpret_cast<__m256i*> (d), value); }

    //==============================================================================
    // Bit Operations
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister bitwiseAnd (AVXRegister a, AVXRegister b) { return { _mm256_and_si256 (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister bitwiseOr (AVXRegister a, AVXRegister b) { return { _mm256_or_si256 (a.value, b.value) }; }
    // These are non AVX2 variants that might be used in functions that are not targeted AVX2 at the expense of slightly worse performance
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister bitwiseAndLegacy (AVXRegister a, AVXRegister b) { return { _mm256_castps_si256 (_mm256_and_ps (_mm256_castsi256_ps (a.value), _mm256_castsi256_ps (b.value))) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister bitwiseOrLegacy (AVXRegister a, AVXRegister b) { return { _mm256_castps_si256 (_mm256_or_ps (_mm256_castsi256_ps (a.value), _mm256_castsi256_ps (b.value))) }; }


    //==============================================================================
    // Math
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister abs (AVXRegister x)                { return { _mm256_abs_epi32 (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister add (AVXRegister a, AVXRegister b) { return { _mm256_add_epi32 (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister sub (AVXRegister a, AVXRegister b) { return { _mm256_sub_epi32 (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister max (AVXRegister a, AVXRegister b) { return { _mm256_max_epi32 (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister min (AVXRegister a, AVXRegister b) { return { _mm256_min_epi32 (a.value, b.value) }; }

    //==============================================================================
    // Type conversion
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister<float> convertToFp (AVXRegister x)     { return { _mm256_cvtepi32_ps (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister<float> reinterpretAsFp (AVXRegister x) { return { _mm256_castsi256_ps (x.value) }; }
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
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister loadUnaligned (const uint32_t* d)                                { return { _mm256_loadu_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister loadAligned   (const uint32_t* d)                                { return { _mm256_load_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister broadcast     (uint32_t x)                                       { return { _mm256_set1_epi32 ((int32_t) x) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister fromSSE       (SSERegister<uint32_t> a, SSERegister<uint32_t> b) { return { _mm256_set_m128i (a.value, b.value) }; }

    //==============================================================================
    // Storing
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void storeUnaligned (uint32_t* d) const { _mm256_storeu_si256 (reinterpret_cast<__m256i*> (d), value); }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void storeAligned   (uint32_t* d) const { _mm256_store_si256  (reinterpret_cast<__m256i*> (d), value); }

    //==============================================================================
    // Bit Operations

    //==============================================================================
    // Math
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister add (AVXRegister a, AVXRegister b) { return { _mm256_add_epi32 (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister sub (AVXRegister a, AVXRegister b) { return { _mm256_sub_epi32 (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister max (AVXRegister a, AVXRegister b) { return { _mm256_max_epu32 (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister min (AVXRegister a, AVXRegister b) { return { _mm256_min_epu32 (a.value, b.value) }; }
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
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister loadUnaligned (const int64_t* d)                               { return { _mm256_loadu_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister loadAligned   (const int64_t* d)                               { return { _mm256_load_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister broadcast     (int64_t x)                                      { return { _mm256_set1_epi64x (x) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister fromSSE       (SSERegister<int64_t> a, SSERegister<int64_t> b) { return { _mm256_set_m128i (a.value, b.value) }; }

    //==============================================================================
    // Storing
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void storeUnaligned (int64_t* d) const { _mm256_storeu_si256 (reinterpret_cast<__m256i*> (d), value); }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void storeAligned   (int64_t* d) const { _mm256_store_si256  (reinterpret_cast<__m256i*> (d), value); }

    //==============================================================================
    // Bit Operations
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister bitwiseAnd (AVXRegister a, AVXRegister b) { return { _mm256_and_si256 (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister bitwiseOr (AVXRegister a, AVXRegister b) { return { _mm256_or_si256 (a.value, b.value) }; }
    // These are non AVX2 variants that might be used in functions that are not targeted AVX2 at the expense of slightly worse performance
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister bitwiseAndLegacy (AVXRegister a, AVXRegister b) { return { _mm256_castpd_si256 (_mm256_and_pd (_mm256_castsi256_pd (a.value), _mm256_castsi256_pd (b.value))) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister bitwiseOrLegacy (AVXRegister a, AVXRegister b) { return { _mm256_castpd_si256 (_mm256_or_pd (_mm256_castsi256_pd (a.value), _mm256_castsi256_pd (b.value))) }; }

    //==============================================================================
    // Math
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister add (AVXRegister a, AVXRegister b) { return { _mm256_add_epi64 (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister sub (AVXRegister a, AVXRegister b) { return { _mm256_sub_epi64 (a.value, b.value) }; }

    //==============================================================================
    // Type conversion
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister<double> convertToFp (AVXRegister x)     { return { _mm256_cvtepi64_pd (x.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister<double> reinterpretAsFp (AVXRegister x) { return { _mm256_castsi256_pd (x.value) }; }
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
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister loadUnaligned (const uint64_t* d)                                { return { _mm256_loadu_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister loadAligned   (const uint64_t* d)                                { return { _mm256_load_si256 (reinterpret_cast<const __m256i*> (d)) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister broadcast     (uint64_t x)                                       { return { _mm256_set1_epi64x ((int64_t) x) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") static AVXRegister fromSSE       (SSERegister<uint64_t> a, SSERegister<uint64_t> b) { return { _mm256_set_m128i (a.value, b.value) }; }

    //==============================================================================
    // Storing
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void storeUnaligned (uint64_t* d) const { _mm256_storeu_si256 (reinterpret_cast<__m256i*> (d), value); }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void storeAligned   (uint64_t* d) const { _mm256_store_si256  (reinterpret_cast<__m256i*> (d), value); }

    //==============================================================================
    // Bit Operations

    //==============================================================================
    // Math
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister add (AVXRegister a, AVXRegister b) { return { _mm256_add_epi64 (a.value, b.value) }; }
    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") static AVXRegister sub (AVXRegister a, AVXRegister b) { return { _mm256_sub_epi64 (a.value, b.value) }; }
    // clang-format on
};

inline AVXRegister<int32_t> AVXRegister<float>::convertToInt (AVXRegister x)      { return { _mm256_cvtps_epi32 (x.value) }; }
inline AVXRegister<int32_t> AVXRegister<float>::reinterpretAsInt (AVXRegister x)  { return { _mm256_castps_si256 (x.value) }; }
inline AVXRegister<int64_t> AVXRegister<double>::convertToInt (AVXRegister x)      { return { _mm256_cvtpd_epi64 (x.value) }; }
inline AVXRegister<int64_t> AVXRegister<double>::reinterpretAsInt (AVXRegister x)  { return { _mm256_castpd_si256 (x.value) }; }
#endif

} // namespace vctr
