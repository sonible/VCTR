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

namespace Catch
{
template <class ElementType, template <class> class Allocator>
struct StringMaker<vctr::Vector<ElementType, Allocator>>
{
    static std::string convert (const vctr::Vector<ElementType, Allocator>& vec)
    {
        std::ostringstream oss;
        oss << "vctr::Vector<" << vctr::typeName<ElementType>() << "> with size " << vec.size() << " and content " << vec;
        return oss.str();
    }
};

template <class ElementType, size_t extent>
struct StringMaker<vctr::Array<ElementType, extent>>
{
    static std::string convert (const vctr::Array<ElementType, extent>& vec)
    {
        std::ostringstream oss;
        oss << "vctr::Array<" << vctr::typeName<ElementType>() << ", " << extent << "> with size " << vec.size() << " and content " << vec;
        return oss.str();
    }
};

template <class ElementType, size_t extent>
struct StringMaker<vctr::Span<ElementType, extent>>
{
    static std::string convert (const vctr::Span<ElementType, extent>& vec)
    {
        std::ostringstream oss;

        if constexpr (extent == std::dynamic_extent)
        {
            oss << "vctr::Span<" << vctr::typeName<ElementType>() << ", std::dynamic_extent> with size " << vec.size() << " and content " << vec;
        }
        else
        {
            oss << "vctr::Span<" << vctr::typeName<ElementType>() << ", " << extent << "> with size " << vec.size() << " and content " << vec;
        }

        return oss.str();
    }
};
} // namespace Catch
