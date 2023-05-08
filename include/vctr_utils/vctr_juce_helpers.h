/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2023 by sonible GmbH.

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

#include "../vctr/vctr.h"
#include <juce_dsp/juce_dsp.h>

/** Helper functions to integrate vctr into dsp code written with JUCE (https://github.com/juce-framework/JUCE).

    It requires that the project links against the JUCE dsp module.
 */
namespace vctr::juce_helpers
{

/** Loads the first elements of src into a juce::SIMDRegister.

    Note: Only available if VCTR is used in a JUCE based project that uses the
    juce_dsp module.

    This function expects that the number of elements in src is exactly equal
    to juce::SIMDRegister<Src::value_type>::SIMDNumElements. Note that JUCE
    chooses either AVX or SSE registers on x64 platforms, depending on the
    AVX flags being passed to the compiler. In particular, this means that
    the function could expect an 8 element float vector as source when built
    with AVX flags while expecting a 4 element vector otherwise. So cross platform
    compatibility is best when not building with AVX flags. Also note that
    juce::dsp::SIMDRegister<double> does NOT wrap a float64x2_t type but some
    fallback type at the time of writing, so this function won't work for doubles
    on ARM.

    If you want to be more explicit about the SIMD register type, consider using
    the explicit SIMD related member functions of VctrBase.

    @see VctrBase::getNeon
    @see VctrBase::getSSE
    @see VctrBase::getAVX
 */
template <is::anyVctr Src>
auto toSIMDRegister (const Src& src)
{
    using SIMDRegister = juce::dsp::SIMDRegister<typename Src::value_type>;

    VCTR_ASSERT (src.size() == SIMDRegister::SIMDNumElements);

    if constexpr (Config::archARM)
    {
        return SIMDRegister { src.getNeon (0).value };
    }
    else
    {
        constexpr size_t registerSize = SIMDRegister::SIMDRegisterSize;

        if constexpr (registerSize == 32)
        {
            return SIMDRegister { src.getAVX (0).value };
        }

        return SIMDRegister { src.getSSE (0).value };
    }
}

/** Returns a Span that views a single channel of a juce::dsp::AudioBlock.

    This allows you to operate on audio channels with all available VCTR features.
    In case the SampleType of the block is const, the Span will give you read access,
    otherwise it will also allow you write access to the channel data.

    Note: If you are working with audio blocks that view memory allocated by JUCE,
    the allocated memory should be SIMD aligned. In that case consider using
    blockChannelAsSpanSIMDAligned, which will return a Span that expects the memory
    being SIMD aligned and might choose some better optimised versions of VCTR expressions
    without any runtime overhead. However, if you are accessing the audio buffer passed
    from a plugin host you cannot expect any guaranteed alignment. In this case, you should
    use this function.

    @see blockChannelAsSpanSIMDAligned
 */
template <class SampleType>
auto blockChannelAsSpan (const juce::dsp::AudioBlock<SampleType>& block, size_t channelIdx)
{
    auto numSamples = block.getNumSamples();
    return Span { block.getChannelPointer (channelIdx), numSamples };
}

/** Returns a Span that views a single channel of a juce::dsp::AudioBlock, expecting SIMD aligned memory.

    This does the same as blockChannelAsSpan, except for expecting the memory to be SIMD aligned. Prefer
    this whenever you are sure that the memory you are working on has proper alignment.

    @see blockChannelAsSpan
 */
template <class SampleType>
auto blockChannelAsSpanSIMDAligned (const juce::dsp::AudioBlock<SampleType>& block, size_t channelIdx)
{
    auto numSamples = block.getNumSamples();
    return Span { block.getChannelPointer (channelIdx), numSamples, simdAlignedSpanStorageInfo<SampleType>() };
}

}
