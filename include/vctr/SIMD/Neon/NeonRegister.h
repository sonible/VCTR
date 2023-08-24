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
    // Bit Operations
    static NeonRegister andNot (NeonRegister a, NeonRegister b) { return { vreinterpretq_f32_u32 (vandq_u32 (vreinterpretq_u32_f32 (a.value), vreinterpretq_u32_f32 (b.value))) }; }

    //==============================================================================
    // Math
    static NeonRegister abs (NeonRegister x)                 { return { vabsq_f32 (x.value) }; }
    static NeonRegister mul (NeonRegister a, NeonRegister b) { return { vmulq_f32 (a.value, b.value) }; }
    static NeonRegister add (NeonRegister a, NeonRegister b) { return { vaddq_f32 (a.value, b.value) }; }
    static NeonRegister sub (NeonRegister a, NeonRegister b) { return { vsubq_f32 (a.value, b.value) }; }
    static NeonRegister max (NeonRegister a, NeonRegister b) { return { vmaxq_f32 (a.value, b.value) }; }
    static NeonRegister min (NeonRegister a, NeonRegister b) { return { vminq_f32 (a.value, b.value) }; }
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
    // Bit Operations
    static NeonRegister andNot (NeonRegister a, NeonRegister b) { return { vreinterpretq_f64_u64 (vandq_u64 (vreinterpretq_u64_f64 (a.value), vreinterpretq_u64_f64 (b.value))) }; }

    //==============================================================================
    // Math
    static NeonRegister abs (NeonRegister x)                 { return { vabsq_f64 (x.value) }; }
    static NeonRegister mul (NeonRegister a, NeonRegister b) { return { vmulq_f64 (a.value, b.value) }; }
    static NeonRegister add (NeonRegister a, NeonRegister b) { return { vaddq_f64 (a.value, b.value) }; }
    static NeonRegister sub (NeonRegister a, NeonRegister b) { return { vsubq_f64 (a.value, b.value) }; }
    static NeonRegister max (NeonRegister a, NeonRegister b) { return { vmaxq_f64 (a.value, b.value) }; }
    static NeonRegister min (NeonRegister a, NeonRegister b) { return { vminq_f64 (a.value, b.value) }; }
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

    //==============================================================================
    // Math
    static NeonRegister abs (NeonRegister x)                 { return { vabsq_s32 (x.value) }; }
    static NeonRegister mul (NeonRegister a, NeonRegister b) { return { vmulq_s32 (a.value, b.value) }; }
    static NeonRegister add (NeonRegister a, NeonRegister b) { return { vaddq_s32 (a.value, b.value) }; }
    static NeonRegister sub (NeonRegister a, NeonRegister b) { return { vsubq_s32 (a.value, b.value) }; }
    static NeonRegister max (NeonRegister a, NeonRegister b) { return { vmaxq_s32 (a.value, b.value) }; }
    static NeonRegister min (NeonRegister a, NeonRegister b) { return { vminq_s32 (a.value, b.value) }; }
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

    //==============================================================================
    // Math
    static NeonRegister abs (NeonRegister x)                 { return { vabsq_s64 (x.value) }; }
    static NeonRegister add (NeonRegister a, NeonRegister b) { return { vaddq_s64 (a.value, b.value) }; }
    static NeonRegister sub (NeonRegister a, NeonRegister b) { return { vsubq_s64 (a.value, b.value) }; }
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

    //==============================================================================
    // Math
    static NeonRegister add (NeonRegister a, NeonRegister b) { return { vaddq_u64 (a.value, b.value) }; }
    static NeonRegister sub (NeonRegister a, NeonRegister b) { return { vsubq_u64 (a.value, b.value) }; }
    // clang-format on
};

#endif

} // namespace vctr
