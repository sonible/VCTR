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
struct NeonRegister
{
    static constexpr NeonRegister broadcast (const T&) { return {}; }
};

#if VCTR_ARM

namespace detail
{

// clang-format off
template <CompareOp, class>
struct NeonCompare {};

template <> struct NeonCompare<CompareOp::less, float>            { static auto cmp (float32x4_t a, float32x4_t b) { return vcltq_f32 (a, b); } };
template <> struct NeonCompare<CompareOp::less, double>           { static auto cmp (float64x2_t a, float64x2_t b) { return vcltq_f64 (a, b); } };
template <> struct NeonCompare<CompareOp::lessOrEqual, float>     { static auto cmp (float32x4_t a, float32x4_t b) { return vcleq_f32 (a, b); } };
template <> struct NeonCompare<CompareOp::lessOrEqual, double>    { static auto cmp (float64x2_t a, float64x2_t b) { return vcleq_f64 (a, b); } };
template <> struct NeonCompare<CompareOp::greater, float>         { static auto cmp (float32x4_t a, float32x4_t b) { return vcgtq_f32 (a, b); } };
template <> struct NeonCompare<CompareOp::greater, double>        { static auto cmp (float64x2_t a, float64x2_t b) { return vcgtq_f64 (a, b); } };
template <> struct NeonCompare<CompareOp::greaterOrEqual, float>  { static auto cmp (float32x4_t a, float32x4_t b) { return vcgeq_f32 (a, b); } };
template <> struct NeonCompare<CompareOp::greaterOrEqual, double> { static auto cmp (float64x2_t a, float64x2_t b) { return vcgeq_f64 (a, b); } };
template <> struct NeonCompare<CompareOp::equal, float>           { static auto cmp (float32x4_t a, float32x4_t b) { return vceqq_f32 (a, b); } };
template <> struct NeonCompare<CompareOp::equal, double>          { static auto cmp (float64x2_t a, float64x2_t b) { return vceqq_f64 (a, b); } };
template <> struct NeonCompare<CompareOp::notEqual, float>        { static auto cmp (float32x4_t a, float32x4_t b) { return vmvnq_u32 (vceqq_f32 (a, b)); } };
template <> struct NeonCompare<CompareOp::notEqual, double>       { static auto cmp (float64x2_t a, float64x2_t b) { return vreinterpretq_u64_u32 (vmvnq_u32 (vreinterpretq_u32_u64 (vceqq_f64 (a, b)))); } };
// clang-format on
}

template <>
struct NeonRegister<float>
{
    static constexpr size_t numElements = 4;

    using NativeType = float32x4_t;
    float32x4_t value;

    //==============================================================================
    // Loading
    // clang-format off
    static NeonRegister load      (const float* d)  { return { vld1q_f32 (d) }; }
    static NeonRegister broadcast (float x)         { return { vdupq_n_f32 (x) }; }

    //==============================================================================
    // Storing
    void store (float* d) const { vst1q_f32 (d, value); }

    //==============================================================================
    // Generate Compare Masks
    template <CompareOp op>
    static NeonRegister compare (NeonRegister a, NeonRegister b) { return { vreinterpretq_f32_u32 (detail::NeonCompare<op, float>::cmp (a.value, b.value)) }; }

    //==============================================================================
    // Bit Operations
    /** Evaluates a & (! b) - so b is negated. */
    static NeonRegister bitwiseAndNot (NeonRegister a, NeonRegister b)                    { return { vreinterpretq_f32_u32 (vandq_u32 (vreinterpretq_u32_f32 (a.value), vmvnq_u32 (vreinterpretq_u32_f32 (b.value)))) }; }
    static NeonRegister bitwiseAnd    (NeonRegister a, NeonRegister b)                    { return { vreinterpretq_f32_u32 (vandq_u32 (vreinterpretq_u32_f32 (a.value), vreinterpretq_u32_f32 (b.value))) }; }
    static NeonRegister bitwiseBlend  (NeonRegister a, NeonRegister b, NeonRegister mask) { return { vbslq_f32 (vreinterpretq_u32_f32 (mask.value), b.value, a.value) }; }

    //==============================================================================
    // Math
    static NeonRegister abs (NeonRegister x)                 { return { vabsq_f32 (x.value) }; }
    static NeonRegister floor (NeonRegister x)               { return { vrndmq_f32 (x.value) }; }
    static NeonRegister ceil (NeonRegister x)                { return { vrndpq_f32 (x.value) }; }
    static NeonRegister mul (NeonRegister a, NeonRegister b) { return { vmulq_f32 (a.value, b.value) }; }
    static NeonRegister div (NeonRegister a, NeonRegister b) { return { vdivq_f32 (a.value, b.value) }; }
    static NeonRegister add (NeonRegister a, NeonRegister b) { return { vaddq_f32 (a.value, b.value) }; }
    static NeonRegister sub (NeonRegister a, NeonRegister b) { return { vsubq_f32 (a.value, b.value) }; }
    static NeonRegister max (NeonRegister a, NeonRegister b) { return { vmaxq_f32 (a.value, b.value) }; }
    static NeonRegister min (NeonRegister a, NeonRegister b) { return { vminq_f32 (a.value, b.value) }; }
    static NeonRegister fma (NeonRegister a, NeonRegister b, NeonRegister c) { return { vfmaq_f32 (c.value, a.value, b.value) }; }
    static NeonRegister fms (NeonRegister a, NeonRegister b, NeonRegister c) { return { vfmsq_f32 (c.value, a.value, b.value) }; }

    //==============================================================================
    // Type conversion
    static NeonRegister<int32_t> convertToInt (NeonRegister x);
    static NeonRegister<int32_t> reinterpretAsInt (NeonRegister x);
    // clang-format on
};

template <>
struct NeonRegister<double>
{
    static constexpr size_t numElements = 2;

    using NativeType = float64x2_t;
    float64x2_t value;

    //==============================================================================
    // Loading
    // clang-format off
    static NeonRegister load      (const double* d)  { return { vld1q_f64 (d) }; }
    static NeonRegister broadcast (double x)         { return { vdupq_n_f64 (x) }; }

    //==============================================================================
    // Storing
    void store (double* d) const { vst1q_f64 (d, value); }

    //==============================================================================
    // Generate Compare Masks
    template <CompareOp op>
    static NeonRegister compare (NeonRegister a, NeonRegister b) { return { vreinterpretq_f64_u64 (detail::NeonCompare<op, double>::cmp (a.value, b.value)) }; }

    //==============================================================================
    // Bit Operations
    /** Evaluates a & (! b) - so b is negated. */
    static NeonRegister bitwiseAndNot (NeonRegister a, NeonRegister b)                    { return { vreinterpretq_f64_u32 (vandq_u32 (vreinterpretq_u32_f64 (a.value), vmvnq_u32 (vreinterpretq_u32_f64 (b.value)))) }; }
    static NeonRegister bitwiseAnd    (NeonRegister a, NeonRegister b)                    { return { vreinterpretq_f64_u32 (vandq_u32 (vreinterpretq_u32_f64 (a.value), vreinterpretq_u32_f64 (b.value))) }; }
    static NeonRegister bitwiseBlend  (NeonRegister a, NeonRegister b, NeonRegister mask) { return { vbslq_f64 (vreinterpretq_u64_f64 (mask.value), b.value, a.value) }; }

    //==============================================================================
    // Math
    static NeonRegister abs (NeonRegister x)                 { return { vabsq_f64 (x.value) }; }
    static NeonRegister floor (NeonRegister x)               { return { vrndmq_f64 (x.value) }; }
    static NeonRegister ceil (NeonRegister x)                { return { vrndpq_f64 (x.value) }; }
    static NeonRegister mul (NeonRegister a, NeonRegister b) { return { vmulq_f64 (a.value, b.value) }; }
    static NeonRegister div (NeonRegister a, NeonRegister b) { return { vdivq_f64 (a.value, b.value) }; }
    static NeonRegister add (NeonRegister a, NeonRegister b) { return { vaddq_f64 (a.value, b.value) }; }
    static NeonRegister sub (NeonRegister a, NeonRegister b) { return { vsubq_f64 (a.value, b.value) }; }
    static NeonRegister max (NeonRegister a, NeonRegister b) { return { vmaxq_f64 (a.value, b.value) }; }
    static NeonRegister min (NeonRegister a, NeonRegister b) { return { vminq_f64 (a.value, b.value) }; }
    static NeonRegister fma (NeonRegister a, NeonRegister b, NeonRegister c) { return { vfmaq_f64 (c.value, a.value, b.value) }; }
    static NeonRegister fms (NeonRegister a, NeonRegister b, NeonRegister c) { return { vfmsq_f64 (c.value, a.value, b.value) }; }

    //==============================================================================
    // Type conversion
    static NeonRegister<int64_t> convertToInt (NeonRegister x);
    static NeonRegister<int64_t> reinterpretAsInt (NeonRegister x);
    // clang-format on
};

template <>
struct NeonRegister<int32_t>
{
    static constexpr size_t numElements = 4;

    using NativeType = int32x4_t;
    int32x4_t value;

    //==============================================================================
    // Loading
    // clang-format off
    static NeonRegister load      (const int32_t* d)  { return { vld1q_s32 (d) }; }
    static NeonRegister broadcast (int32_t x)         { return { vdupq_n_s32 (x) }; }

    //==============================================================================
    // Storing
    void store (int32_t* d) const { vst1q_s32 (d, value); }

    //==============================================================================
    // Bit Operations
    static NeonRegister bitwiseAnd (NeonRegister a, NeonRegister b) { return { vandq_s32 (a.value, b.value) }; }
    static NeonRegister bitwiseOr  (NeonRegister a, NeonRegister b) { return { vorrq_s32 (a.value, b.value) }; }

    //==============================================================================
    // Math
    static NeonRegister abs (NeonRegister x)                 { return { vabsq_s32 (x.value) }; }
    static NeonRegister mul (NeonRegister a, NeonRegister b) { return { vmulq_s32 (a.value, b.value) }; }
    static NeonRegister add (NeonRegister a, NeonRegister b) { return { vaddq_s32 (a.value, b.value) }; }
    static NeonRegister sub (NeonRegister a, NeonRegister b) { return { vsubq_s32 (a.value, b.value) }; }
    static NeonRegister max (NeonRegister a, NeonRegister b) { return { vmaxq_s32 (a.value, b.value) }; }
    static NeonRegister min (NeonRegister a, NeonRegister b) { return { vminq_s32 (a.value, b.value) }; }

    //==============================================================================
    // Type conversion
    static NeonRegister<float> convertToFp (NeonRegister x)     { return { vcvtq_f32_s32 (x.value) }; }
    static NeonRegister<float> reinterpretAsFp (NeonRegister x) { return { vreinterpretq_f32_s32 (x.value) }; }
    // clang-format on
};

template <>
struct NeonRegister<uint32_t>
{
    static constexpr size_t numElements = 4;

    using NativeType = uint32x4_t;
    uint32x4_t value;

    //==============================================================================
    // Loading
    // clang-format off
    static NeonRegister load      (const uint32_t* d)  { return { vld1q_u32 (d) }; }
    static NeonRegister broadcast (uint32_t x)         { return { vdupq_n_u32 (x) }; }

    //==============================================================================
    // Storing
    void store (uint32_t* d) const { vst1q_u32 (d, value); }

    //==============================================================================
    // Bit Operations
    static NeonRegister bitwiseAnd (NeonRegister a, NeonRegister b) { return { vandq_u32 (a.value, b.value) }; }
    static NeonRegister bitwiseOr  (NeonRegister a, NeonRegister b) { return { vorrq_u32 (a.value, b.value) }; }

    //==============================================================================
    // Math
    static NeonRegister mul (NeonRegister a, NeonRegister b) { return { vmulq_u32 (a.value, b.value) }; }
    static NeonRegister add (NeonRegister a, NeonRegister b) { return { vaddq_u32 (a.value, b.value) }; }
    static NeonRegister sub (NeonRegister a, NeonRegister b) { return { vsubq_u32 (a.value, b.value) }; }
    static NeonRegister max (NeonRegister a, NeonRegister b) { return { vmaxq_u32 (a.value, b.value) }; }
    static NeonRegister min (NeonRegister a, NeonRegister b) { return { vminq_u32 (a.value, b.value) }; }
    // clang-format on
};

template <>
struct NeonRegister<int64_t>
{
    static constexpr size_t numElements = 2;

    using NativeType = int64x2_t;
    int64x2_t value;

    //==============================================================================
    // Loading
    // clang-format off
    static NeonRegister load      (const int64_t* d)  { return { vld1q_s64 (d) }; }
    static NeonRegister broadcast (int64_t x)         { return { vdupq_n_s64 (x) }; }

    //==============================================================================
    // Storing
    void store (int64_t* d) const { vst1q_s64 (d, value); }

    //==============================================================================
    // Bit Operations
    static NeonRegister bitwiseAnd (NeonRegister a, NeonRegister b) { return { vandq_s64 (a.value, b.value) }; }
    static NeonRegister bitwiseOr  (NeonRegister a, NeonRegister b) { return { vorrq_s64 (a.value, b.value) }; }

    //==============================================================================
    // Math
    static NeonRegister abs (NeonRegister x)                 { return { vabsq_s64 (x.value) }; }
    static NeonRegister add (NeonRegister a, NeonRegister b) { return { vaddq_s64 (a.value, b.value) }; }
    static NeonRegister sub (NeonRegister a, NeonRegister b) { return { vsubq_s64 (a.value, b.value) }; }

    //==============================================================================
    // Type conversion
    static NeonRegister<double> convertToFp (NeonRegister x)     { return { vcvtq_f64_s64 (x.value) }; }
    static NeonRegister<double> reinterpretAsFp (NeonRegister x) { return { vreinterpretq_f64_s64 (x.value) }; }
    // clang-format on
};

template <>
struct NeonRegister<uint64_t>
{
    static constexpr size_t numElements = 2;

    using NativeType = uint64x2_t;
    uint64x2_t value;

    //==============================================================================
    // Loading
    // clang-format off
    static NeonRegister load      (const uint64_t* d)  { return { vld1q_u64 (d) }; }
    static NeonRegister broadcast (uint64_t x)         { return { vdupq_n_u64 (x) }; }

    //==============================================================================
    // Storing
    void store (uint64_t* d) const { vst1q_u64 (d, value); }

    //==============================================================================
    // Bit Operations
    static NeonRegister bitwiseAnd (NeonRegister a, NeonRegister b) { return { vandq_u64 (a.value, b.value) }; }
    static NeonRegister bitwiseOr  (NeonRegister a, NeonRegister b) { return { vorrq_u64 (a.value, b.value) }; }

    //==============================================================================
    // Math
    static NeonRegister add (NeonRegister a, NeonRegister b) { return { vaddq_u64 (a.value, b.value) }; }
    static NeonRegister sub (NeonRegister a, NeonRegister b) { return { vsubq_u64 (a.value, b.value) }; }
    // clang-format on
};

inline NeonRegister<int32_t> NeonRegister<float>::convertToInt (NeonRegister<float> x)       { return { vcvtq_s32_f32 (x.value) }; }
inline NeonRegister<int32_t> NeonRegister<float>::reinterpretAsInt (NeonRegister<float> x)   { return { vreinterpretq_s32_f32 (x.value) }; }
inline NeonRegister<int64_t> NeonRegister<double>::convertToInt (NeonRegister<double> x)     { return { vcvtq_s64_f64 (x.value) }; }
inline NeonRegister<int64_t> NeonRegister<double>::reinterpretAsInt (NeonRegister<double> x) { return { vreinterpretq_s64_f64 (x.value) }; }

#endif

} // namespace vctr
