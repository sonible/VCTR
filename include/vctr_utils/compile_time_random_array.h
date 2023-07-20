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

#pragma once

#include <array>
#include <cstdint>
#include <numbers>

#ifdef _MSC_VER
__pragma(warning(push))
__pragma(warning(disable:4244)) // conversion from 'long double' to 'T', possible loss of data
#endif
#include <gcem.hpp>
#ifdef _MSC_VER
__pragma(warning(pop))
#endif

/** This file contains methods that allow for the creations of randomly filled arrays at compile-time.
    The methods of interest are @see makeArrayWithNormalDistribution and @see makeArrayWithUniformDistribution.
    @code
        constexpr auto min = 0.0f;
        constexpr auto max = 1.0f;
        constexpr auto seed = 42;
        constexpr std::array<float, 100> uniformRandomData = vctr::random::makeArrayWithNormalDistribution<float, 100> (min, max, seed);

        constexpr auto mean = 42.0f;
        constexpr auto sigma = 1.0f;
        constexpr std::array<float, 100> normalRandomData = vctr::random::makeArrayWithNormalDistribution<float, 100> (mean, sigma, seed);
    @endcode
*/

namespace vctr::random::detail
{
/** Computes a uint32 integer from a string */
consteval auto getTimeFromString (const char* str, int offset)
{
    return static_cast<uint32_t> (str[offset] - '0') * 10 + static_cast<uint32_t> (str[offset + 1] - '0');
}

/** Computes a uint32 integer from the current system time when compiling. */
consteval auto getSeedFromCurrentTime()
{
    auto t = __TIME__;
    return getTimeFromString (t, 0) * 60 * 60 + getTimeFromString (t, 3) * 60 + getTimeFromString (t, 6);
}

/** This implements a linear congruential pseudo-random-number generator.
    It uses the same parameters as the std::minstd_rand generator namespace.
    https://www.cplusplus.com/reference/random/minstd_rand/
*/

consteval uint32_t uniformDistribution (uint32_t& previous)
{
    constexpr uint32_t lce_a = 48271;
    constexpr uint32_t lce_c = 0;
    constexpr uint32_t lce_m = std::numeric_limits<int32_t>::max();
    previous = ((lce_a * previous + lce_c) % lce_m);
    return previous;
}

/** Returns a uniformly distributed random value of type T between [min, max]. */
template <typename T>
consteval T uniformDistribution (T min, T max, uint32_t& seed)
{
    auto span = double (max - min);
    constexpr auto scale = double (std::numeric_limits<int32_t>::max());
    return T (double (uniformDistribution (seed)) / scale * (span)) + min;
}

/** Returns an std::array<T, size>, with uniformly distributed random values in between [min, max].
    The random-number generator will be initialized with the provided seed. Two subsequent calls to this method
    using the same seed will result in the same array.
*/
template <size_t size, typename T>
consteval auto makeArrayWithUniformDistribution (T min, T max, uint32_t seed)
{
    std::array<T, size> dst {};
    for (auto& el : dst)
        el = uniformDistribution (min, max, seed);

    return dst;
}

/** Returns an std::array<T, size>, filled with normally distributed random values with the provided mean and sigma.
    The random-number generator will be initialized with the provided seed. Two subsequent calls to this method
    using the same seed will result in the same array.
    This method implements the Box-Muller transform, used to translate uniform to normal distribution.
    https://www.baeldung.com/cs/uniform-to-normal-distribution
*/
template <size_t size, std::floating_point T>
consteval auto makeArrayWithNormalDistribution (T mean, T sigma, uint32_t seed)
{
    std::array<T, size> dst {};

    for (size_t i = 0; i < size - 1; i += 2)
    {
        auto u1 = T (0);
        auto u2 = T (0);

        // first two uniformly distributed values between [0.0, 1.0] are calculated
        // u1 must not be zero
        do
        {
            u1 = uniformDistribution (T (0), T (1), seed);
            u2 = uniformDistribution (T (0), T (1), seed);
        } while (u1 <= std::numeric_limits<T>::epsilon());

        // from these values two normally distributed values are calculated
        // these values are then transformed to a general gaussian value to achieve the desired mean and sigma
        // z1 = sqrt(-2 * ln(u1)) * cos (2 * PI * u2)
        // z1 = sigma * z1 + mean
        // z2 = sqrt(-2 * ln(u1)) * sin (2 * PI * u2)
        // z2 = sigma * z2 + mean

        auto mag = sigma * gcem::sqrt (T (-2) * gcem::log (u1));
        auto twoPiU2 = std::numbers::pi_v<T> * 2 * u2;
        dst[i] = mag * gcem::cos (twoPiU2) + mean;

        // in case of odd array sizes, the last value is simply discarded
        if (i + 1 < size)
            dst[i + 1] = mag * gcem::sin (twoPiU2) + mean;
    }

    return dst;
}
} // namespace vctr::random::detail

namespace vctr::random
{

/** Returns an std::array<T, size>, with uniformly distributed random values in between [min, max].
    As a seed for the random number generator the current time when compiling will be used + an optional
    offset value. Alternate the offset in case you want to generate multiple different compile time arrays,
    as multiple calls to this function in the same TU will always return the same array.
*/
template <size_t size, typename T>
consteval auto makeArrayWithUniformDistribution (T min, T max, uint32_t seedOffset = 0)
{
    return detail::makeArrayWithUniformDistribution<size, T> (min, max, detail::getSeedFromCurrentTime() + seedOffset);
}

/** Returns an std::array<T, size>, filled with normally distributed random values with the provided mean and sigma.
    As a seed for the random number generator the current time when compiling will be used.
*/
template <size_t size, std::floating_point T>
consteval auto makeArrayWithNormalDistribution (T mean, T sigma)
{
    return detail::makeArrayWithNormalDistribution<size, T> (mean, sigma, detail::getSeedFromCurrentTime());
}

} // namespace vctr::random
