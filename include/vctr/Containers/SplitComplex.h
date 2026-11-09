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

#include <complex>

namespace vctr
{

/** A container for complex values that stores the real and imaginary parts in two separate, equally sized buffers
    instead of interleaved like std::complex does.

    This layout allows element-wise complex math to be evaluated without any dependencies between neighbouring
    elements, which is ideal for SIMD vectorisation. The math operations are available as allocation free free
    functions, see SplitComplexMath.h.

    The ContainerType decides about ownership and size: Use SplitComplexVector for an owning, resizable container,
    SplitComplexArray for an owning container with a compile time size and SplitComplexSpan (or SplitComplexConstSpan)
    for a non-owning view.

    @tparam ContainerType   Any VCTR container holding a real floating point value_type, e.g. Vector<float>,
                            Array<float, n> or Span<float>.

    @ingroup Core
 */
template <class ContainerType>
requires is::anyVctr<ContainerType> && is::realFloatNumber<typename ContainerType::value_type>
class SplitComplexBase
{
public:
    //==============================================================================
    using value_type = std::remove_const_t<typename ContainerType::value_type>;
    using complex_type = std::complex<value_type>;

    //==============================================================================
    // Constructors
    //==============================================================================

    /** Creates a default constructed SplitComplexBase. Only available for owning containers.

        For a SplitComplexVector this creates an empty container, for a SplitComplexArray the elements
        are uninitialised, just like for a default constructed vctr::Array.
     */
    constexpr SplitComplexBase() requires std::default_initializable<ContainerType> = default;

    /** Creates a SplitComplexBase with the given size and all values initialised to zero. Only available for resizable containers. */
    explicit SplitComplexBase (size_t size)
    requires vctr::has::resize<ContainerType>
        : realParts (size, value_type (0)),
          imagParts (size, value_type (0))
    { }

    /** Creates a SplitComplexBase from a real and an imaginary part container of the same size.

        Owning containers copy the data, views refer to the passed containers.
     */
    template <class Real, class Imag>
    requires std::constructible_from<ContainerType, Real&&> && std::constructible_from<ContainerType, Imag&&>
    SplitComplexBase (Real&& realPart, Imag&& imagPart)
        : realParts (std::forward<Real> (realPart)),
          imagParts (std::forward<Imag> (imagPart))
    {
        VCTR_ASSERT (realParts.size() == imagParts.size());
    }

    /** Creates a SplitComplexBase by converting from another one, e.g. a SplitComplexSpan viewing a SplitComplexVector,
        or a SplitComplexVector holding a copy of a SplitComplexSpan.
     */
    template <class Other>
    requires (! std::same_as<Other, ContainerType>) && std::constructible_from<ContainerType, decltype (std::declval<SplitComplexBase<Other>&>().real())>
    SplitComplexBase (SplitComplexBase<Other>& other)
        : realParts (other.real()),
          imagParts (other.imag())
    {
    }

    /** @copydoc SplitComplexBase(SplitComplexBase<Other>&) */
    template <class Other>
    requires (! std::same_as<Other, ContainerType>) && std::constructible_from<ContainerType, decltype (std::declval<const SplitComplexBase<Other>&>().real())>
    SplitComplexBase (const SplitComplexBase<Other>& other)
        : realParts (other.real()),
          imagParts (other.imag())
    {
    }

    /** Creates a SplitComplexBase by deinterleaving a container of std::complex values. Only available for owning containers.

        For containers with a compile time size, the size of the interleaved container has to match.
     */
    template <has::sizeAndDataWithElementType<const complex_type> Interleaved>
    requires std::default_initializable<ContainerType>
    explicit SplitComplexBase (const Interleaved& interleaved)
    {
        if constexpr (vctr::has::resize<ContainerType>)
            resize (interleaved.size());

        assignInterleaved (interleaved);
    }

    //==============================================================================
    // Element access
    //==============================================================================

    /** Returns a read-only view of the real parts. */
    constexpr Span<const value_type> real() const { return Span<const value_type> (realParts); }

    /** Returns a read-only view of the imaginary parts. */
    constexpr Span<const value_type> imag() const { return Span<const value_type> (imagParts); }

    /** Returns a mutable view of the real parts. */
    constexpr auto real() { return Span<std::remove_pointer_t<decltype (realParts.data())>> (realParts); }

    /** Returns a mutable view of the imaginary parts. */
    constexpr auto imag() { return Span<std::remove_pointer_t<decltype (imagParts.data())>> (imagParts); }

    /** Returns the complex value at index i. */
    constexpr complex_type operator[] (size_t i) const { return { realParts[i], imagParts[i] }; }

    /** Sets the complex value at index i. */
    constexpr void set (size_t i, complex_type value)
    {
        realParts[i] = value.real();
        imagParts[i] = value.imag();
    }

    /** Sets the real and imaginary part at index i. */
    constexpr void set (size_t i, value_type realValue, value_type imagValue)
    {
        realParts[i] = realValue;
        imagParts[i] = imagValue;
    }

    //==============================================================================
    // Size and content manipulation
    //==============================================================================

    /** Returns the number of complex values. */
    constexpr size_t size() const { return realParts.size(); }

    /** Returns true if the container holds no values. */
    constexpr bool empty() const { return realParts.size() == 0; }

    /** Resizes both parts. Only available for resizable containers, e.g. a Vector. New elements are zero. */
    constexpr void resize (size_t newSize)
    requires vctr::has::resize<ContainerType>
    {
        realParts.resize (newSize);
        imagParts.resize (newSize);
    }

    /** Sets all elements to the given complex value. */
    constexpr void fill (complex_type value)
    {
        std::fill (realParts.begin(), realParts.end(), value.real());
        std::fill (imagParts.begin(), imagParts.end(), value.imag());
    }

    /** Sets all elements to zero. */
    constexpr void clear() { fill (complex_type (0, 0)); }

    /** Copies the elements of another SplitComplexBase of identical size. */
    template <class Other>
    constexpr void copyFrom (const SplitComplexBase<Other>& other)
    {
        VCTR_ASSERT (size() == other.size());
        std::copy (other.real().begin(), other.real().end(), realParts.begin());
        std::copy (other.imag().begin(), other.imag().end(), imagParts.begin());
    }

    /** Deinterleaves std::complex values into this container. The sizes must match. */
    template <has::sizeAndDataWithElementType<const complex_type> Interleaved>
    constexpr void assignInterleaved (const Interleaved& interleaved)
    {
        VCTR_ASSERT (size() == interleaved.size());

        const auto* src = interleaved.data();

        for (size_t i = 0; i < size(); ++i)
        {
            realParts[i] = src[i].real();
            imagParts[i] = src[i].imag();
        }
    }

    /** Interleaves the values into a container of std::complex values. The sizes must match. */
    template <has::sizeAndDataWithElementType<complex_type> Interleaved>
    constexpr void copyToInterleaved (Interleaved& interleaved) const
    {
        VCTR_ASSERT (size() == interleaved.size());

        auto* dst = interleaved.data();

        for (size_t i = 0; i < size(); ++i)
            dst[i] = (*this)[i];
    }

private:
    ContainerType realParts;
    ContainerType imagParts;
};

//==============================================================================
/** An owning split complex container. */
template <is::realFloatNumber T>
using SplitComplexVector = SplitComplexBase<Vector<T>>;

/** An owning split complex container with a compile time size. */
template <is::realFloatNumber T, size_t size>
using SplitComplexArray = SplitComplexBase<Array<T, size>>;

/** A non-owning, mutable view to split complex data. */
template <is::realFloatNumber T>
using SplitComplexSpan = SplitComplexBase<Span<T>>;

/** A non-owning, read-only view to split complex data. */
template <is::realFloatNumber T>
using SplitComplexConstSpan = SplitComplexBase<Span<const T>>;

} // namespace vctr
