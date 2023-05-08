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

/*******************************************************************************
 VCTR can optionally be used as a JUCE Module. The block below describes the
 properties of this module, and is read by the Projucer to automatically generate
 project code that uses it.

 BEGIN_JUCE_MODULE_DECLARATION
  ID:                 vctr
  vendor:             sonible
  version:            1.0.0
  name:               VCTR
  description:        A powerful C++ 20 wrapper around your favourite standard library containers.
  website:            https://github.com/sonible/vctr
  license:            MIT
  minimumCppStandard: 20
  dependencies:
  OSXFrameworks:      Accelerate
  iOSFrameworks:      Accelerate
 END_JUCE_MODULE_DECLARATION
*******************************************************************************/

#pragma once

#include <utility>
#include <vector>
#include <array>
#include <span>
#include <concepts>
#include <cmath>
#include <ostream>
#include <bit>
#include <bitset>
#include <iterator>
#include <memory>
#include <cstring>
#include <ranges>
#include <algorithm>
#include <numeric>
#include <optional>
#include <functional>

#ifdef jassert
#define VCTR_ASSERT(e) jassert (e)
#else
#include <cassert>
#define VCTR_ASSERT(e) assert (e)
#endif

// Also includes Concepts/ElementConcepts.h internally
#include "vctr_forward_declarations.h"

#include "Miscellaneous/Config.h"

#include "Miscellaneous/CompilerSpecificAttributes.h"

#if VCTR_X64
#include <immintrin.h>
#endif

#if VCTR_ARM
#include <arm_neon.h>
#endif

#if VCTR_APPLE
#include <Accelerate/Accelerate.h>
#endif

#if VCTR_USE_IPP
#include <ippcore.h>
#include <ipps.h>
#include <ippvm.h>
#endif

#if VCTR_USE_GCEM
#include <gcem.hpp>

#if ((GCEM_VERSION_MAJOR < 1) || (GCEM_VERSION_MAJOR == 1 && GCEM_VERSION_MINOR < 16))
#error "VCTR needs gcem version 1.16.0 or greater"
#endif

#endif

//==============================================================================
// Doxygen group definitions
//==============================================================================
/**
    @defgroup Core
    The three central container and view classes of VCTR.

    @defgroup Expressions
    An overview of all expression chain builders. Expression chain builders are used
    to build of chains of vector expressions using the << operator. Example:
    @code
    auto expression = vctr::ln << vctr::abs << (src - 3);
    @endcode
    Will create an expression that evaluates the natural logarithm of the absolute value
    of the difference of the source vector src and the constant value 3. This expression
    can then be assigned to an existing Vector, Array or Span of a matching extent or be
    used to initialize a new Vector or Array.
 */

#include "TypeTraitsAndConcepts/ContainerAndExpressionConcepts.h"
#include "TypeTraitsAndConcepts/FunctionConcepts.h"

#include "Containers/VectorBoolWorkaround.h"

#include "SIMD/SIMDHelpers.h"
#include "SIMD/SSE/SSERegister.h"
#include "SIMD/AVX/AVXRegister.h"
#include "SIMD/Neon/NeonRegister.h"

#include "PlatformVectorOps/PlatformVectorOpsHelpers.h"
#include "PlatformVectorOps/AppleAccelerate.h"
#include "PlatformVectorOps/IntelIPP.h"

#include "TypeTraitsAndConcepts/Traits.h"

#include "Expressions/ExpressionTemplate.h"

#include "Miscellaneous/Range.h"

#include "Containers/VctrBase.h"
#include "Containers/Span.h"
#include "Miscellaneous/AlignedAllocator.h"
#include "Containers/Vector.h"
#include "Containers/Array.h"

#include "Expressions/ReductionExpression.h"
#include "Expressions/ExpressionChainBuilder.h"

#include "Expressions/Filter/NoAcceleration.h"
#include "Expressions/Filter/SIMDFilter.h"
#include "Expressions/Filter/PlatformVectorOpsFilter.h"

#include "Expressions/Generic/Map.h"
#include "Expressions/Generic/TransformedBy.h"
#include "Expressions/Generic/TransformedByStaticCast.h"

#include "Expressions/BasicMath/Abs.h"
#include "Expressions/BasicMath/Clamp.h"
#include "Expressions/BasicMath/Cube.h"
#include "Expressions/BasicMath/Sqrt.h"
#include "Expressions/BasicMath/Square.h"
#include "Expressions/BasicMath/Add.h"
#include "Expressions/BasicMath/Subtract.h"
#include "Expressions/BasicMath/Multiply.h"
#include "Expressions/BasicMath/Divide.h"
#include "Expressions/BasicMath/Max.h"
#include "Expressions/BasicMath/Min.h"
#include "Expressions/BasicMath/Mean.h"
#include "Expressions/BasicMath/Sum.h"
#include "Expressions/BasicMath/NormalizeSum.h"

#include "Expressions/Complex/Angle.h"
#include "Expressions/Complex/Conjugate.h"
#include "Expressions/Complex/Imag.h"
#include "Expressions/Complex/Real.h"
#include "Expressions/Complex/PowerSpectrum.h"

#include "Expressions/Exp/Exp.h"
#include "Expressions/Exp/Ln.h"
#include "Expressions/Exp/Log2.h"
#include "Expressions/Exp/Log10.h"
#include "Expressions/Exp/Pow.h"

#include "Expressions/Trigonometric/Sin.h"
#include "Expressions/Trigonometric/Cos.h"
#include "Expressions/Trigonometric/Tan.h"
#include "Expressions/Trigonometric/Sinh.h"
#include "Expressions/Trigonometric/Cosh.h"
#include "Expressions/Trigonometric/Tanh.h"
#include "Expressions/Trigonometric/Asin.h"
#include "Expressions/Trigonometric/Acos.h"
#include "Expressions/Trigonometric/Atan.h"
#include "Expressions/Trigonometric/Asinh.h"
#include "Expressions/Trigonometric/Acosh.h"
#include "Expressions/Trigonometric/Atanh.h"

#include "Expressions/DSP/Decibels.h"

#include "Generators/Linspace.h"

#include "Miscellaneous/StdOstreamOperator.h"

//==============================================================================
// tpp files go here
//==============================================================================
#include "Containers/VctrBase.tpp"
