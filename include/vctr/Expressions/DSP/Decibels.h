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

#if VCTR_USE_GCEM
template <is::constant C>
struct Log10ToLog2Constant
{
    static constexpr double value = double (C::value) / gcem::log2 (10.0);
};

template <is::constant C>
struct InvertedLog10ToLog2Constant
{
    static constexpr double value = gcem::log2 (10.0) / double (C::value);
};
#endif
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
                                                      ::vctr::detail::InvertedConstant<DecibelConstant>>,
                                Constant<10>>;

#if VCTR_USE_GCEM
template <size_t extent, class SrcType, class DecibelConstant, class MinDb>
using FastMagToDb = ClampByConstant<extent,
                                    MultiplyVecByConstant<extent,
                                                          FastLog2<extent, SrcType>,
                                                          ::vctr::detail::Log10ToLog2Constant<DecibelConstant>>,
                                    MinDb,
                                    DisabledConstant>;

template <size_t extent, class SrcType, class DecibelConstant>
using FastDbToMag = FastExp2<extent,
                             MultiplyVecByConstant<extent,
                                                   SrcType,
                                                   ::vctr::detail::InvertedLog10ToLog2Constant<DecibelConstant>>>;
#endif

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

#if VCTR_USE_GCEM
/** Converts the source magnitude into a decibel representation using fast approximations.

    The calculation is max (constant * log2 (src * log2 (10)), minDb), with constant being either 20 for dBFS or
    dBVoltage (the typical value when dealing with digital audio amplitudes) or 10 for dBPower. The log2 function
    is evaluated using the fastLog2 expression implementation, leading to an approximately 7x faster calculation
    compared to the usual magToDb expression at the expense of some numerical precision.

   @tparam DecibelConstant: Either vctr::dBFS, vctr::dBVoltage or vctr::dBPower.
   @tparam minDb: The lower threshold for the resulting dB value and thus the value for a magnitude of 0.

   @ingroup Expressions
 */
template <is::constant DecibelConstant, auto minDb = -100>
constexpr inline ExpressionChainBuilderWithRuntimeArgs<expressions::FastMagToDb, detail::RuntimeArgChain<std::tuple<>, std::tuple<>, std::tuple<>>, DecibelConstant, Constant<minDb>> fastMagToDb;

/** Converts the source decibel values into their magnitude representation using fast approximations.

    The calculation is pow (2, src * (log2 (10) / constant), with constant being either 20 for dBFS or dBVoltage
    (the typical value when dealing with digital audio amplitudes) or 10 for dBPower. The evaluation of 2 raised
    to the power of src * (log2 (10) / constant is evaluated using the fastExp2 expression implementation, leading to
    an approximately 5x faster calculation compared to the usual dbToMag expression at the expense of some numerical
    precision.

   @tparam DecibelConstant: Either vctr::dBFS, vctr::dBVoltage or vctr::dBPower.

   @ingroup Expressions
 */
template <is::constant DecibelConstant>
constexpr inline ExpressionChainBuilderWithRuntimeArgs<expressions::FastDbToMag, detail::RuntimeArgChain<std::tuple<>, std::tuple<>, std::tuple<>>, DecibelConstant> fastDbToMag;
#endif

} // namespace vctr
