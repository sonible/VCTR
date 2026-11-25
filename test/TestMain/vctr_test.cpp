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
#include <vctr_utils/compile_time_random_array.h>
#include <iostream>

int main (int argc, char** argv)
{
    // The random test data is generated at compile time, so Catch's --rng-seed has no effect on it. Print the seed
    // so a failing run can be reproduced by building with that seed. Printed to stderr to not break reporters
    // writing machine-readable output to stdout.
    std::cerr << "VCTR compile time random seed: " << vctr::random::getSeed()
              << " (reproduce by configuring with -D VCTR_TEST_RANDOM_SEED=" << vctr::random::getSeed() << ")\n";

    return Catch::Session().run (argc, argv);
}
