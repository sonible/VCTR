/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2022 by sonible GmbH.

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

namespace vctr::detail
{

/** An internal helper class to get around the std::vector<bool> specialisation limits.

    std::vector<bool> behaves differently to the default implementation as it acts like a
    dynamic bitfield, allowing the implementation to store 8 bits into a byte in order
    to be more space efficient. The downside of this approach is that direct memory access
    via pointers and references is no longer possible with this design. This makes it
    especially impossible to create a std::span / vctr::Span like view to a std::vector<bool>.
    In order to fit better into the whole VCTR ecosystem we decided against wrapping a
    std::vector<bool> inside a vctr::Vector<bool> and use this workaround class instead.
    It uses a std::vector<std::byte> to store the bool values and shadows all
    pointer/reference related member functions with functions that return bool pointers
    or references which are based on reinterpret casts to the std::byte pointers/references.
 */
template <class Allocator>
class VectorBoolWorkaround : public std::vector<std::byte, Allocator>
{
private:
    //==============================================================================
    using StdVectorType = std::vector<std::byte, Allocator>;

    static bool&       toBool (std::byte& element)       { return *reinterpret_cast<bool*> (&element); }
    static const bool& toBool (const std::byte& element) { return *reinterpret_cast<const bool*> (&element); }
    static bool*       toBool (std::byte* ptr)           { return reinterpret_cast<bool*> (ptr); }
    static const bool* toBool (const std::byte* ptr)     { return reinterpret_cast<const bool*> (ptr); }

public:
    constexpr VectorBoolWorkaround (std::initializer_list<bool> il)
    {
        StdVectorType::reserve (il.size());
        for (bool e : il)
            push_back (e);
    }

    //==============================================================================
    constexpr bool&       operator[] (size_t i)       { return toBool (StdVectorType::operator[] (i)); }
    constexpr const bool& operator[] (size_t i) const { return toBool (StdVectorType::operator[] (i)); }
    constexpr bool&       at (size_t i)               { return toBool (StdVectorType::at (i)); }
    constexpr const bool& at (size_t i) const         { return toBool (StdVectorType::at (i)); }
    constexpr bool&       front()                     { return toBool (StdVectorType::front()); }
    constexpr const bool& front() const               { return toBool (StdVectorType::front()); }
    constexpr bool&       back()                      { return toBool (StdVectorType::back()); }
    constexpr const bool& back() const                { return toBool (StdVectorType::back()); }
    constexpr void        push_back (bool v)          { StdVectorType::push_back (std::byte (v)); }

    bool*       data()        { return toBool (StdVectorType::data()); }
    const bool* data() const  { return toBool (StdVectorType::data()); }
    bool*       begin()       { return data(); }
    const bool* begin() const { return data(); }
    bool*       end()         { return data() + StdVectorType::size(); }
    const bool* end() const   { return data() + StdVectorType::size(); }
};
}
