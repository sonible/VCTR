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
namespace detail
{
template <class T>
struct SplitComplexContainerType
{
    using Type = T;
};

template <class T, template <class> class A>
struct SplitComplexContainerType<Vector<std::complex<T>, A>>
{
    using Type = Vector<T, A>;
};

template <class T, size_t e, size_t s>
struct SplitComplexContainerType<Array<std::complex<T>, e, s>>
{
    using Type = Array<T, e, s>;
};

template <class E, class S, size_t extent, class I>
struct SplitComplexContainerType<VctrBase<E, S, extent, I>>
{
    using Type = VctrBase<E, S, extent, I>;
};

template <class T, size_t extent, class S>
struct SplitComplexContainerType<Span<T, extent, S>>
{
    using Type = Span<T, extent, S>;
};
} // namespace detail


/** An expression combining separate real and imaginary component expressions.

    @tparam RealExpression The expression producing the real parts.
    @tparam ImagExpression The expression producing the imaginary parts. Its value type must match RealExpression.

    @ingroup Expressions
 */
template <is::expression RealExpression, is::expression ImagExpression>
requires (std::same_as<typename RealExpression::value_type, typename ImagExpression::value_type>)
struct SplitComplexExpression
{
    using value_type = typename RealExpression::value_type;

    RealExpression realExpression;
    ImagExpression imagExpression;

    /** Returns the expression producing the real parts. */
    RealExpression& real() { return realExpression; }

    /** Returns the expression producing the imaginary parts. */
    ImagExpression& imag() { return imagExpression; }
};

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
template <is::anyVctrWithRealFloatValueType ContainerType>
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
    constexpr SplitComplexBase (Real&& realPart, Imag&& imagPart)
        : realParts (std::forward<Real> (realPart)),
          imagParts (std::forward<Imag> (imagPart))
    {
        VCTR_ASSERT (realParts.size() == imagParts.size());
    }

    /** Creates a SplitComplexBase by converting from another one, e.g. a SplitComplexSpan viewing a SplitComplexVector,
        or a SplitComplexVector holding a copy of a SplitComplexSpan.
     */
    template <class Other>
    requires (! std::same_as<Other, ContainerType>) && std::constructible_from<ContainerType, Span<value_type>>
    constexpr SplitComplexBase (SplitComplexBase<Other>& other)
        : realParts (other.real()),
          imagParts (other.imag())
    { }

    /** @copydoc SplitComplexBase(SplitComplexBase<Other>&) */
    template <class Other>
    requires (! std::same_as<Other, ContainerType>) && std::constructible_from<ContainerType, Span<const value_type>>
    constexpr SplitComplexBase (const SplitComplexBase<Other>& other)
        : realParts (other.real()),
          imagParts (other.imag())
    { }

    /** Creates a SplitComplexBase by deinterleaving a container of std::complex values. Only available for owning containers.

        Allows implicit conversion from an interleaved container.
        For containers with a compile time size, the size of the interleaved container has to match.
     */
    template <is::anyVctrWithComplexFloatValueType Interleaved>
    SplitComplexBase (const Interleaved& interleaved)
    {
        if constexpr (vctr::has::resize<ContainerType>)
            resize (interleaved.size());

        *this = interleaved;
    }

    /** Creates a split-complex container from a split-complex expression.

        The real and imaginary component expressions are assigned to the corresponding parts of this container.
     */
    template <is::expression RealExpression, is::expression ImagExpression>
    constexpr SplitComplexBase (SplitComplexExpression<RealExpression, ImagExpression>&& expressions)
    {
        *this = expressions;
    }

    //==============================================================================
    // Assignment
    //==============================================================================

    /** Assigns the result of a split-complex expression to this container.

        The underlying containers may resize if their size differs from the expression size.
     */
    template <is::expression RealExpression, is::expression ImagExpression>
    constexpr void operator= (const SplitComplexExpression<RealExpression, ImagExpression>& expressions)
    {
        real() = expressions.realExpression;
        imag() = expressions.imagExpression;
    }

    /** Copies the elements of another SplitComplexBase of identical size. */
    template <class Other>
    requires (std::convertible_to<typename SplitComplexBase<Other>::value_type, value_type>)
    constexpr void operator= (const SplitComplexBase<Other>& other)
    {
        VCTR_ASSERT (size() == other.size());

        real() = other.real();
        imag() = other.imag();
    }

    template <is::anyVctrWithComplexFloatValueType Interleaved>
    constexpr void operator= (const Interleaved& interleaved)
    {
        VCTR_ASSERT (size() == interleaved.size());

        const auto* src = interleaved.data();

        for (size_t i = 0; i < size(); ++i)
        {
            realParts[i] = src[i].real();
            imagParts[i] = src[i].imag();
        }
    }

    //==============================================================================
    // Element access
    //==============================================================================

    /** Returns a read-only view of the real parts. */
    constexpr Span<const value_type> real() const { return Span<const value_type> (realParts); }

    /** Returns a read-only view of the imaginary parts. */
    constexpr Span<const value_type> imag() const { return Span<const value_type> (imagParts); }

    /** Returns a mutable view of the real parts. */
    constexpr Span<value_type> real() { return Span<value_type> (realParts); }

    /** Returns a mutable view of the imaginary parts. */
    constexpr Span<value_type> imag() { return Span<value_type> (imagParts); }

    /** Returns the complex value at index i. */
    constexpr complex_type operator[] (size_t i) const { return { realParts[i], imagParts[i] }; }

    /** Sets the real and imaginary part at index i. */
    constexpr void set (size_t i, value_type realValue, value_type imagValue)
    {
        realParts[i] = realValue;
        imagParts[i] = imagValue;
    }

    /** Sets the complex value at index i. */
    constexpr void set (size_t i, complex_type value) { set (i, value.real(), value.imag()); }

    //==============================================================================
    // Size and content manipulation
    //==============================================================================

    /** Returns the number of complex values. */
    constexpr size_t size() const { return real().size(); }

    /** Returns true if the container holds no values. */
    constexpr bool empty() const { return real().size() == 0; }

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
        real().fill (value.real());
        imag().fill (value.imag());
    }

    /** Sets all elements to zero. */
    constexpr void clear() { fill (complex_type (0, 0)); }

    //==============================================================================
    // Math operators.
    //==============================================================================

    /** Multiplies this by a vector or expression in place. */
    template <is::anyVctrOrExpression V>
    constexpr void operator*= (const V& v)
    {   
        auto& self = *this;
        self = self * v;
    }

    ///** Multiplies this by a constant in place. */
    //constexpr void operator*= (value_type c);

    ///** Divides this by a vector or expression in place. */
    //template <is::anyVctrOrExpression V>
    //constexpr void operator/= (const V& v);

    ///** Divides this by a constant in place. */
    //constexpr void operator/= (value_type c);

    ///** Adds a vector or expression to this in place.

    //    In the special case where a multiplication expression is assigned for which none of the operands
    //    are expressions themselves, this will try to use accelerated multiply accumulate operations to
    //    evaluate the expression.

    //    Example:
    //    @code
    //    vctr::Array<float, 10> a, b, c;

    //    // This code tries to use an accelerated multiply accumulate operation as it can access the values of a and b right away
    //    c += a * b;
    //    c += a * 42.0f;

    //    // Cannot directly evaluate the accelerated operation as vctr::square (b) would have to be evaluated and stored to
    //    // some temporary memory location first.
    //    c += a * vctr::square (b);
    //    @endcode
    // */
    //template <is::anyVctrOrExpression V>
    //constexpr void operator+= (const V& v);

    ///** Adds a constant to this in place. */
    //constexpr void operator+= (value_type c);

    ///** Subtracts a vector or expression from this in place. */
    //template <is::anyVctrOrExpression V>
    //constexpr void operator-= (const V& v);

    ///** Subtracts a constant from this in place. */
    //constexpr void operator-= (value_type c);

private:
    ContainerType realParts;
    ContainerType imagParts;
};

/** Deduces the real storage type from an interleaved complex container. */
template <class Interleaved>
SplitComplexBase (const Interleaved&) -> SplitComplexBase<typename detail::SplitComplexContainerType<Interleaved>::Type>;

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
