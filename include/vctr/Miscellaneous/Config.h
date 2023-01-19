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

//==============================================================================
// Platform defines
//==============================================================================
#if defined(_WIN32) || defined(_WIN64)
#define VCTR_WINDOWS 1
#elif defined(LINUX) || defined(__linux__)
#define VCTR_LINUX 1
#elif defined(__APPLE_CPP__) || defined(__APPLE_CC__)
#define VCTR_APPLE 1
#elif defined(__wasm__)
#define VCTR_WASM 1
#endif

#ifndef VCTR_WINDOWS
#define VCTR_WINDOWS 0
#endif

#ifndef VCTR_LINUX
#define VCTR_LINUX 0
#endif

#ifndef VCTR_APPLE
#define VCTR_APPLE 0
#endif

#ifndef VCTR_WASM
#define VCTR_WASM 0
#endif

//==============================================================================
// CPU architecture defines
//==============================================================================
#if defined(__aarch64__) || defined(_M_ARM64)
#define VCTR_ARM 1
#define VCTR_X64 0
#else
#define VCTR_ARM 0
#define VCTR_X64 1
#endif

//==============================================================================
// Compiler defines
//==============================================================================
#if defined(__clang__)
#define VCTR_CLANG 1
#elif defined(__GNUC__)
#define VCTR_GCC 1
#elif defined(_MSC_VER)
#define VCTR_MSVC 1
#endif

#ifndef VCTR_CLANG
#define VCTR_CLANG 0
#endif

#ifndef VCTR_GCC
#define VCTR_GCC 0
#endif

#ifndef VCTR_MSVC
#define VCTR_MSVC 0
#endif

//==============================================================================
// Build type defines
//==============================================================================
#if defined(DEBUG) || defined(_DEBUG) || ! (defined(NDEBUG) || defined(_NDEBUG))
#define VCTR_DEBUG 1
#else
#define VCTR_DEBUG 0
#endif

//==============================================================================
// User supplied defines
//==============================================================================

/** Define this to 1 in case IPP is available for x64 builds. Define it to 0 if you don't want to use IPP.

    If not defined, it will decide whether to use IPP or not depending on the result of __has_include (<ipp.h>)
 */
#ifndef VCTR_USE_IPP
#if __has_include(<ipp.h>)
#define VCTR_USE_IPP 1
#else
#define VCTR_USE_IPP 0
#endif
#endif

/** Define this to 1 in case gcem (https://github.com/kthohr/gcem) is available. The minimum required version is gcem 1.16.0.

    If not defined, it will decide whether to use IPP or not depending on the result of __has_include (<gcem.hpp>)
 */
#ifndef VCTR_USE_GCEM
#if __has_include(<gcem.hpp>)
#define VCTR_USE_GCEM 1
#else
#define VCTR_USE_GCEM 0
#endif
#endif

namespace vctr
{

enum class CPUInstructionSet
{
    sse4_1,
    avx,
    avx2,

    neon,

    fallback
};

#if VCTR_WINDOWS

#include <intrin.h>

namespace detail
{
/** Implementation inspired by https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?redirectedfrom=MSDN&view=msvc-170 */
class X64InstructionSets
{
private:
    struct CPUFeatureFinder
    {
        CPUFeatureFinder()
        {
            //int cpuInfo[4] = {-1};
            std::array<int, 4> cpui;
            std::vector<std::array<int, 4>> data, extdata;

            // Calling __cpuid with 0x0 as the function_id argument
            // gets the number of the highest valid function ID.
            __cpuid (cpui.data(), 0);
            auto nIds = cpui[0];

            for (int i = 0; i <= nIds; ++i)
            {
                __cpuidex (cpui.data(), i, 0);
                data.push_back (cpui);
            }

            // load bitset with flags for function 0x00000001
            if (nIds >= 1)
            {
                f_1_ECX_ = data[1][2];
                f_1_EDX_ = data[1][3];
            }

            // load bitset with flags for function 0x00000007
            if (nIds >= 7)
            {
                f_7_EBX_ = data[7][1];
                f_7_ECX_ = data[7][2];
            }

            // Calling __cpuid with 0x80000000 as the function_id argument
            // gets the number of the highest valid extended ID.
            __cpuid (cpui.data(), 0x80000000);
            auto nExIds = cpui[0];

            for (int i = 0x80000000; i <= nExIds; ++i)
            {
                __cpuidex (cpui.data(), i, 0);
                extdata.push_back (cpui);
            }

            // load bitset with flags for function 0x80000001
            if (nExIds >= 0x80000001)
            {
                f_81_ECX_ = extdata[1][2];
                f_81_EDX_ = extdata[1][3];
            }
        };

        std::bitset<32> f_1_ECX_;
        std::bitset<32> f_1_EDX_;
        std::bitset<32> f_7_EBX_;
        std::bitset<32> f_7_ECX_;
        std::bitset<32> f_81_ECX_;
        std::bitset<32> f_81_EDX_;
    };

    inline static const CPUFeatureFinder cpuFeatures;

public:
    static bool hasFMA() { return cpuFeatures.f_1_ECX_[12]; }
    static bool hasSSE41() { return cpuFeatures.f_1_ECX_[19]; }
    static bool hasSSE42() { return cpuFeatures.f_1_ECX_[20]; }
    static bool hasAVX() { return cpuFeatures.f_1_ECX_[28]; }
    static bool hasAVX2() { return cpuFeatures.f_7_EBX_[5]; }
    static bool hasAVX512F() { return cpuFeatures.f_7_EBX_[16]; }

private:
};
} // namespace detail

inline CPUInstructionSet getHighestSupportedCPUInstructionSet()
{
    if (detail::X64InstructionSets::hasAVX2())
        return CPUInstructionSet::avx2;

    if (detail::X64InstructionSets::hasAVX())
        return CPUInstructionSet::avx;

    if (detail::X64InstructionSets::hasSSE41())
        return CPUInstructionSet::sse4_1;

    return CPUInstructionSet::fallback;
}

#elif VCTR_ARM

inline CPUInstructionSet getHighestSupportedCPUInstructionSet()
{
    return CPUInstructionSet::neon;
}

#else

inline CPUInstructionSet getHighestSupportedCPUInstructionSet()
{
    __builtin_cpu_init();

    if (__builtin_cpu_supports ("avx2"))
        return CPUInstructionSet::avx2;

    if (__builtin_cpu_supports ("avx"))
        return CPUInstructionSet::avx;

    if (__builtin_cpu_supports ("sse4.1"))
        return CPUInstructionSet::sse4_1;

    return CPUInstructionSet::fallback;
}

#endif

namespace detail
{

/** Returns the number of settings that are defined to true */
template <bool... settings>
consteval size_t trueCount()
{
    return (size_t (settings) + ...);
}

} // namespace detail

struct Config
{
    static const inline auto highestSupportedCPUInstructionSet = getHighestSupportedCPUInstructionSet();

    static const inline auto supportsAVX2 = highestSupportedCPUInstructionSet == CPUInstructionSet::avx2;

    static const inline auto supportsAVX = highestSupportedCPUInstructionSet == CPUInstructionSet::avx2 || highestSupportedCPUInstructionSet == CPUInstructionSet::avx;

    //==============================================================================
    // Platform config
    //==============================================================================
    static constexpr bool platformWindows = VCTR_WINDOWS;

    static constexpr bool platformApple = VCTR_APPLE;

    static constexpr bool platformLinux = VCTR_LINUX;

    static constexpr bool platformWasm = VCTR_WASM;

    static_assert (detail::trueCount<platformWindows, platformApple, platformLinux, platformWasm>() == 1, "Unsupported platform or platform detection error");

    //==============================================================================
    // CPU architecture config
    //==============================================================================
    static constexpr bool archARM = VCTR_ARM;

    static constexpr bool archX64 = VCTR_X64;

    static_assert (detail::trueCount<archARM, archX64>() == 1, "Unsupported architecture or architecture detection error");

    //==============================================================================
    // Compiler config
    //==============================================================================
    static constexpr bool compilerClang = VCTR_CLANG;

    static constexpr bool compilerGCC = VCTR_GCC;

    static constexpr bool compilerMSVC = VCTR_MSVC;

    static_assert (detail::trueCount<compilerClang, compilerGCC, compilerMSVC>() == 1, "Unsupported compiler or compiler detection error");

    //==============================================================================
    // User supplied config
    //==============================================================================
    static constexpr bool hasIPP = VCTR_USE_IPP && archX64;

    //==============================================================================
    // Auto generated config
    //==============================================================================
    static constexpr size_t maxSIMDRegisterSize = archX64 ? 32 : 16;
};

} // namespace vctr
