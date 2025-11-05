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

#if __cpp_lib_bit_cast >= 201806L

template <class To, class From>
constexpr To bitCast (const From& from) noexcept { return std::bit_cast<To> (from); }

#elif VCTR_CLANG && __has_builtin (__builtin_bit_cast)

template <class To, class From>
constexpr To bitCast (const From& from) noexcept { return __builtin_bit_cast (To, from); }

#else

#error "Your compiler has no bit_cast support"

#endif
}
