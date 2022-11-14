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

#include <catch2/catch_approx.hpp>
#include <complex>

namespace vctr
{

/** A simple extension of Catch::Approx that also works with std::complex */
template <class T>
class Approx : public Catch::Approx
{
public:
    explicit Approx (T v)
        : Catch::Approx (v)
    {}
};

template <class T>
class Approx<std::complex<T>>
{
public:
    explicit Approx (const std::complex<T>& v)
        : real (v.real()),
          imag (v.imag())
    {}

    bool operator== (const std::complex<T>& other)
    {
        return real == other.real() && imag == other.imag();
    }

    bool operator== (T other)
    {
        return real == other.real() && imag == 0.0;
    }

    bool operator!= (const std::complex<T>& other)
    {
        return real != other.real() && imag != other.imag();
    }

    bool operator!= (T other)
    {
        return real != other.real() && imag != 0.0;
    }

    Approx& margin (double newMargin)
    {
        real.margin (newMargin);
        imag.margin (newMargin);
        return *this;
    }

    Approx& epsilon (double newEpsilon)
    {
        real.epsilon (newEpsilon);
        imag.epsilon (newEpsilon);
        return *this;
    }

private:
    Catch::Approx real;
    Catch::Approx imag;
};

} // namespace vctr
