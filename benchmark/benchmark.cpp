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

#include <catch2/catch_session.hpp>
#include <random>
#include <vctr/vctr.h>

int main (int argc, char** argv) { return Catch::Session().run (argc, argv); }

#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <chrono>

// ===============================================================
TEST_CASE ("Decibels")
{
    vctr::Vector<float> src, dst;

    auto fillVectorRandomly = [] (auto& v) {
        std::random_device rd;
        std::mt19937 mt (rd());

        std::uniform_real_distribution<float> dist (0.0f, 10.0f);

        for (size_t i = 0; i < v.size(); ++i)
            v[i] = dist (mt);
    };

    // ===============================================================
    auto expression = [&]() -> const vctr::Vector<float>&
    {
        dst = vctr::magToDb<vctr::dBFS> << src;
        return dst;
    };

    auto expressionNotAccelerated = [&]() -> const vctr::Vector<float>&
    {
        dst = vctr::dontUseAcceleration << vctr::magToDb<vctr::dBFS> << src;
        return dst;
    };

    auto forLoop = [&]() -> const vctr::Vector<float>&
    {
        const auto s = src.size();

        for (size_t i = 0; i < s; ++i)
            dst[i] = std::max (-100.0f, 20.0f * std::log10 (src[i]));

        return dst;
    };

    auto accelerate = [&]() -> const vctr::Vector<float>&
    {
        const auto s = src.size();
        const auto si = int (s);
        const auto factor = 20.0f;
        const auto thresh = -100.0f;

        vvlog10f (dst.data(), src.data(), &si);
        vDSP_vsmul (dst.data(), 1, &factor, dst.data(), 1, s);
        vDSP_vthr (dst.data(), 1, &thresh, dst.data(), 1, s);

        return dst;
    };

    // ===============================================================
    auto size = GENERATE (128, 256, 1024, 4096, 16384);

    SECTION (std::to_string (size))
    {
        src.resize (size);
        dst.resize (size);

        fillVectorRandomly (src);

        BENCHMARK ("magToDb expression")
        {
            return expression();
        };

        BENCHMARK ("magToDb expression not accelerated")
        {
            return expressionNotAccelerated();
        };

        BENCHMARK ("for loop")
        {
            return forLoop();
        };

        BENCHMARK ("Direct accelerate calls")
        {
            return accelerate();
        };
    }
}

// ===============================================================
TEST_CASE ("Multiplication")
{
    vctr::Vector<float> srcA, srcB, dst;

    auto fillVectorRandomly = [] (auto& v) {
        std::random_device rd;
        std::mt19937 mt (rd());

        std::uniform_real_distribution<float> dist (0.0f, 10.0f);

        for (size_t i = 0; i < v.size(); ++i)
            v[i] = dist (mt);
    };

    // ===============================================================
    auto accelerate = [&]() -> const vctr::Vector<float>&
    {
        dst = vctr::usePlatformVectorOps << (srcA * srcB);
        return dst;
    };

    auto avx = [&]() -> const vctr::Vector<float>&
    {
        dst = vctr::useAVX << (srcA * srcB);
        return dst;
    };

    auto sse = [&]() -> const vctr::Vector<float>&
    {
        dst = vctr::useSSE << (srcA * srcB);
        return dst;
    };

    auto notAccelerated = [&]() -> const vctr::Vector<float>&
    {
        dst = vctr::dontUseAcceleration << (srcA * srcB);
        return dst;
    };

    // ===============================================================
    auto size = GENERATE (128, 256, 1024, 4096, 16384);

    SECTION (std::to_string (size))
    {
        srcA.resize (size);
        srcB.resize (size);
        dst.resize (size);

        fillVectorRandomly (srcA);
        fillVectorRandomly (srcB);

        BENCHMARK ("Accelerate")
        {
            return accelerate();
        };

        BENCHMARK ("AVX")
        {
            return avx();
        };

        BENCHMARK ("SSE")
        {
            return sse();
        };

        BENCHMARK ("Not accelerated")
        {
            return notAccelerated();
        };
    }
}