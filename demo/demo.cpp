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

#include <vctr/vctr.h>
#include <iostream>

int main()
{
    // vctr::Array is a compile-time static sized stack container based on std::array.
    vctr::Array a { 1.0, 2.0, 3.0, 4.0 };

    // vctr::Vector is a dynamic sized heap container based on std::vector.
    vctr::Vector b { 5.0, 6.0, 7.0, 8.0 };

    // General purpose math operators are overloaded to generate vector math expression templates which work without
    // creating temporary vectors. It is just a lightweight struct that holds information on how to add a and b.
    auto sum = a + b;

    // We can pass all expressions and vector-like types to an output stream to print them
    std::cout << "sum =\t" << sum << std::endl; // should output [ 6, 8, 10, 12 ]

    // An expression can also be assigned to an Array or Vector which will be initialized with the expression result.
    // This also works without creating temporary vectors, it will even attempt to evaluate the result directly into
    // the destination memory
    vctr::Array c = sum;

    // This should output the same as above
    std::cout << "c = \t" << c << std::endl; // should output [ 6, 8, 10, 12 ]

    // Besides usual math operators there are also expressions that can be applied and chained using the << syntax.
    // Again, no temporary vectors created here, ln (c) ^ 2 is directly evaluated into the destination memory
    vctr::Vector d = vctr::square << vctr::ln << c;

    // Let's see what this produced
    std::cout << "d = \t" << d << std::endl; // should output [ 3.2104, 4.32408, 5.3019, 6.17476 ]

    // We can also operate on a sub-portion of the vector by creating a subSpan, which is a view to a portion of d.
    // Here we let it start at index 1 and let it have a length of 2 elements
    auto dSubSpan = d.subSpan (1, 2);

    // Let's subtract some value from this
    dSubSpan -= 6.34;

    // Now let's inspect d again
    std::cout << "d = \t" << d << std::endl; // should output [ 3.2104, -2.01592, -1.0381, 6.17476 ]

    // Let's assume there is some externally managed memory...
    constexpr auto n = 4;
    auto* x = new double[n];
    std::memset (x, 0, n * sizeof (double));

    //... then we can simply wrap a Span around it...
    vctr::Span xSpan (x, n);

    // ... and use all vctr functions to work on that memory
    xSpan = vctr::dbToMag<vctr::dBFS> << sum;

    // Looks good!
    std::cout << "x = \t[ " << x[0] << ", " << x[1] << ", " << x[2] << ", " << x[3] << " ]" << std::endl;

    delete[] x;

    return 0;
}
