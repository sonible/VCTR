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

namespace vctr::detail
{
template <is::constant C>
struct InvertedConstant
{
    static constexpr double value = 1.0 / double (C::value);
};

} // namespace vctr::detail

namespace vctr::expressions
{

template <size_t extent, class SrcType, class DecibelConstant, class MinDb>
using MagToDb = ClampByConstant<extent,
                                MultiplyVecByConstant<extent,
                                                      Log10<extent, SrcType>,
                                                      DecibelConstant>,
                                MinDb,
                                DisabledConstant>;

template <size_t extent, class SrcType, class DecibelConstant>
using DBToMag = PowConstantBase<extent,
                                MultiplyVecByConstant<extent,
                                                      SrcType,
                                                      detail::InvertedConstant<DecibelConstant>>,
                                Constant<10>>;

} // namespace vctr::expressions

namespace vctr
{

/** Decibel constant dbFS to be passed to dbToMag or magToDb.

    @ingroup Expressions
 */
struct dBFS : Constant<20> {};

/** Decibel constant dbVoltage to be passed to dbToMag or magToDb.

    @ingroup Expressions
 */
struct dBVoltage : Constant<20> {};

/** Decibel constant dbPower to be passed to dbToMag or magToDb.

    @ingroup Expressions
 */
struct dBPower : Constant<10> {};

/** Converts the source magnitude into a decibel representation.

    The calculation is max (constant * log10 (src), minDb), with constant being either 20 for dBFS or dBVoltage
    (the typical value when dealing with digital audio amplitudes) or 10 for dBPower.

   @tparam DecibelConstant: Either vctr::dBFS, vctr::dBVoltage or vctr::dBPower.
   @tparam minDb: The lower threshold for the resulting dB value and thus the value for a magnitude of 0.

   @ingroup Expressions
 */
template <is::constant DecibelConstant, auto minDb = -100>
constexpr inline ExpressionChainBuilderWithRuntimeArgs<expressions::MagToDb, detail::RuntimeArgChain<std::tuple<>, std::tuple<>, std::tuple<>>, DecibelConstant, Constant<minDb>> magToDb;

/** Converts the source decibel values into their magnitude representation.

    The calculation is pow (10, src * (1 / constant), with constant being either 20 for dBFS or dBVoltage
    (the typical value when dealing with digital audio amplitudes) or 10 for dBPower.

   @tparam DecibelConstant: Either vctr::dBFS, vctr::dBVoltage or vctr::dBPower.

   @ingroup Expressions
 */
template <is::constant DecibelConstant>
constexpr inline ExpressionChainBuilderWithRuntimeArgs<expressions::DBToMag, detail::RuntimeArgChain<std::tuple<>, std::tuple<>, std::tuple<>>, DecibelConstant> dbToMag;

} // namespace vctr
