/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2023 by sonible GmbH.

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

TEST_CASE ("detail::previousMultipleOf", "[VCTR][internal]")
{
    REQUIRE (vctr::detail::previousMultipleOf<2> (0) == 0);
    REQUIRE (vctr::detail::previousMultipleOf<2> (1) == 0);
    REQUIRE (vctr::detail::previousMultipleOf<2> (2) == 2);
    REQUIRE (vctr::detail::previousMultipleOf<2> (3) == 2);

    REQUIRE (vctr::detail::previousMultipleOf<128> (0) == 0);
    REQUIRE (vctr::detail::previousMultipleOf<128> (1) == 0);
    REQUIRE (vctr::detail::previousMultipleOf<128> (50) == 0);
    REQUIRE (vctr::detail::previousMultipleOf<128> (128) == 128);
    REQUIRE (vctr::detail::previousMultipleOf<128> (150) == 128);
    REQUIRE (vctr::detail::previousMultipleOf<128> (257) == 256);
}

TEST_CASE ("detail::nextMultipleOf", "[VCTR][internal]")
{
    REQUIRE (vctr::detail::nextMultipleOf<2> (0) == 0);
    REQUIRE (vctr::detail::nextMultipleOf<2> (1) == 2);
    REQUIRE (vctr::detail::nextMultipleOf<2> (2) == 2);
    REQUIRE (vctr::detail::nextMultipleOf<2> (3) == 4);

    REQUIRE (vctr::detail::nextMultipleOf<128> (0) == 0);
    REQUIRE (vctr::detail::nextMultipleOf<128> (1) == 128);
    REQUIRE (vctr::detail::nextMultipleOf<128> (50) == 128);
    REQUIRE (vctr::detail::nextMultipleOf<128> (150) == 256);
}

TEST_CASE ("x64 CPU instruction set detection is consistent", "[VCTR][internal][cpuDetection]")
{
    const auto allFeatures = vctr::detail::makeSupportedX64InstructionSets (true, true, true, true, true);
    REQUIRE (allFeatures.sse4_1);
    REQUIRE (allFeatures.avx);
    REQUIRE (allFeatures.avx2);
    REQUIRE (allFeatures.fma);

    const auto noOSAVXSupport = vctr::detail::makeSupportedX64InstructionSets (true, true, true, true, false);
    REQUIRE (noOSAVXSupport.sse4_1);
    REQUIRE_FALSE (noOSAVXSupport.avx);
    REQUIRE_FALSE (noOSAVXSupport.avx2);
    REQUIRE_FALSE (noOSAVXSupport.fma);

    const auto noCPUAVXSupport = vctr::detail::makeSupportedX64InstructionSets (true, false, true, true, true);
    REQUIRE_FALSE (noCPUAVXSupport.avx);
    REQUIRE_FALSE (noCPUAVXSupport.avx2);
    REQUIRE_FALSE (noCPUAVXSupport.fma);

    const auto detected = vctr::Config::supportedCPUInstructionSets;
    REQUIRE_FALSE ((detected.fma && ! detected.avx));
    REQUIRE_FALSE ((detected.avx2 && ! detected.avx));
}

TEST_CASE ("x64 OS AVX state support detection", "[VCTR][internal][cpuDetection]")
{
    struct Case
    {
        bool osxsave;
        uint64_t xcr0;
        bool expected;
    };

    // clang-format off
    constexpr Case cases[] {
        { false, 0x0, false },
        { false, 0x2, false },
        { false, 0x4, false },
        { false, 0x6, false },
        { true,  0x0, false },
        { true,  0x2, false },
        { true,  0x4, false },
        { true,  0x6, true  }
    };
    // clang-format on

    for (const auto& c : cases)
    {
        INFO ("osxsave = " << c.osxsave << ", xcr0 = " << c.xcr0);
        REQUIRE (vctr::detail::osSupportsAVXState (c.osxsave, c.xcr0) == c.expected);

        // Unrelated XCR0 bits must neither enable nor disable AVX support
        REQUIRE (vctr::detail::osSupportsAVXState (c.osxsave, c.xcr0 | 0xe1) == c.expected);
    }
}

TEST_CASE ("x64 OS AVX-512 state support detection", "[VCTR][internal][cpuDetection]")
{
    struct Case
    {
        bool osxsave;
        uint64_t xcr0;
        bool expected;
    };

    // clang-format off
    constexpr Case cases[] {
        // All AVX-512 bits but no or incomplete AVX state
        { true,  0xe0, false },
        { true,  0xe2, false },
        { true,  0xe4, false },

        // AVX state, but one or more AVX-512 bits missing
        { true,  0x06, false },
        { true,  0x26, false },
        { true,  0x46, false },
        { true,  0x86, false },
        { true,  0x66, false },
        { true,  0xa6, false },
        { true,  0xc6, false },

        // Full state
        { true,  0xe6, true  },
        { false, 0xe6, false }
    };
    // clang-format on

    for (const auto& c : cases)
    {
        INFO ("osxsave = " << c.osxsave << ", xcr0 = " << c.xcr0);
        REQUIRE (vctr::detail::osSupportsAVX512State (c.osxsave, c.xcr0) == c.expected);

        // Unrelated XCR0 bits must neither enable nor disable AVX-512 support
        REQUIRE (vctr::detail::osSupportsAVX512State (c.osxsave, c.xcr0 | 0x219) == c.expected);
    }
}
