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

#include <complex>
#include "TypeTraitsAndConcepts/GenericConcepts.h"
#include "TypeTraitsAndConcepts/NumericValueConcepts.h"

/** The main namespace of the VCTR project */
namespace vctr
{

template <class ElementType, class StorageType, size_t extent, class StorageInfoType>
class VctrBase;

template <class ElementType>
struct DefaultVectorAllocator;

template <typename ElementType, size_t alignmentInBytes>
requires (alignmentInBytes >= alignof (ElementType) && is::powerOfTwoInt<alignmentInBytes>)
class AlignedAllocator;

template <is::nonConst ElementType, template <class> class Allocator>
class Vector;

template <class ElementType, size_t extent, size_t storageExtent>
requires (extent != std::dynamic_extent && extent <= storageExtent)
class Array;

template <class ElementType, size_t extent, class StorageInfoType>
class Span;

struct ExpressionTemplateBase;

template <template <size_t, class...> class ExpressionType, class... ExtraParameters>
struct ExpressionChainBuilder;

template <class T>
struct AVXRegister;

template <class T>
struct SSERegister;

template <class T>
struct NeonRegister;
} // namespace vctr
