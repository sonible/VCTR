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

// The storage extent of an Array depends on its element type, so it must not be taken over from the complex array
template <class T, size_t e, size_t s>
struct SplitComplexContainerType<Array<std::complex<T>, e, s>>
{
    using Type = Array<T, e>;
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
    constexpr RealExpression& real() { return realExpression; }

    /** @copydoc real() */
    constexpr const RealExpression& real() const { return realExpression; }

    /** Returns the expression producing the imaginary parts. */
    constexpr ImagExpression& imag() { return imagExpression; }

    /** @copydoc imag() */
    constexpr const ImagExpression& imag() const { return imagExpression; }
};

template <is::anyVctrWithRealFloatValueType ContainerType>
class SplitComplexBase;

namespace is
{
/** Evaluates to true if T derives from a specialization of SplitComplexBase. */
template <class T>
concept derivedFromSplitComplexBase = requires (T& d) { [] <class ContainerType> (const SplitComplexBase<ContainerType>&) {} (d); };

/** Evaluates to true if T derives from a specialization of SplitComplexExpression. */
template <class T>
concept derivedFromSplitComplexExpression = requires (T& d) { [] <expression... Ts> (const SplitComplexExpression<Ts...>&) {} (d); };

/** Evaluates to true if T is a split-complex container or split-complex expression. */
template <class T>
concept anySplitComplexVectorOrExpression = derivedFromSplitComplexBase<T> || derivedFromSplitComplexExpression<T>;

// TODO: write concept to make sure that value type is the same!
//template <class... Containers>
//concept 
} // namespace is

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

    /** The element type of the underlying containers, which is const qualified for a SplitComplexConstSpan.

        Note that ContainerType::value_type can't be used for this, as VctrBase strips the const qualifier from it.
     */
    using element_type = std::remove_reference_t<decltype (*std::declval<ContainerType&>().data())>;

    /** True if the elements can be modified through this container, false for a SplitComplexConstSpan. */
    static constexpr bool isMutable = ! std::is_const_v<element_type>;

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
    // Declaring the copy assignment operators below suppresses the implicit copy and move constructors, so they are
    // explicitly defaulted here.
    constexpr SplitComplexBase (const SplitComplexBase&) = default;
    constexpr SplitComplexBase (SplitComplexBase&&) = default;

    /** Copy and move assignment of owning containers, e.g. a SplitComplexVector or SplitComplexArray. */
    constexpr SplitComplexBase& operator= (const SplitComplexBase&) requires (! is::view<ContainerType>) = default;

    /** @copydoc operator=(const SplitComplexBase&) */
    constexpr SplitComplexBase& operator= (SplitComplexBase&&) requires (! is::view<ContainerType>) = default;

    /** Copies the elements viewed by another span of the same type into the elements viewed by this span.

        Like vctr::Span, a SplitComplexSpan never rebinds on assignment. This makes assigning a span of the same type
        behave like assigning any other split complex container. Both spans need to have the same size.
        A SplitComplexConstSpan can't be assigned to.
     */
    constexpr SplitComplexBase& operator= (const SplitComplexBase& other) requires is::view<ContainerType> && isMutable
    {
        VCTR_ASSERT (size() == other.size());

        realParts = other.real();
        imagParts = other.imag();
        return *this;
    }

    /** @copydoc operator=(const SplitComplexBase&) */
    constexpr SplitComplexBase& operator= (SplitComplexBase&& other) requires is::view<ContainerType> && isMutable
    {
        return *this = std::as_const (other);
    }


    /** Assigns the result of a split-complex expression to this container.

        The underlying containers may resize if their size differs from the expression size.
     */
    template <is::expression RealExpression, is::expression ImagExpression>
    requires isMutable
    constexpr void operator= (const SplitComplexExpression<RealExpression, ImagExpression>& expressions)
    {
        if constexpr (vctr::has::resize<ContainerType>)
            resize (expressions.realExpression.size());

        assignSplitComplexExpression (expressions.realExpression, expressions.imagExpression);
    }

    /** Copies the elements of another SplitComplexBase of identical size. */
    template <class Other>
    requires isMutable && (std::convertible_to<typename SplitComplexBase<Other>::value_type, value_type>)
    constexpr void operator= (const SplitComplexBase<Other>& other)
    {
        VCTR_ASSERT (size() == other.size());

        realParts = other.real();
        imagParts = other.imag();
    }

    template <is::anyVctrWithComplexFloatValueType Interleaved>
    requires isMutable
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

    /** Returns a view of the real parts. It is mutable unless this is a SplitComplexConstSpan. */
    constexpr Span<element_type> real() { return Span<element_type> (realParts); }

    /** Returns a view of the imaginary parts. It is mutable unless this is a SplitComplexConstSpan. */
    constexpr Span<element_type> imag() { return Span<element_type> (imagParts); }

    /** Returns the complex value at index i. */
    constexpr complex_type operator[] (size_t i) const { return { realParts[i], imagParts[i] }; }

    /** Sets the real and imaginary part at index i. */
    constexpr void set (size_t i, value_type realValue, value_type imagValue)
    requires isMutable
    {
        realParts[i] = realValue;
        imagParts[i] = imagValue;
    }

    /** Sets the complex value at index i. */
    constexpr void set (size_t i, complex_type value)
    requires isMutable
    {
        set (i, value.real(), value.imag());
    }

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
    requires isMutable
    {
        realParts.fill (value.real());
        imagParts.fill (value.imag());
    }

    /** Sets all elements to zero. */
    constexpr void clear()
    requires isMutable
    {
        fill (complex_type (0, 0));
    }

    //==============================================================================
    // Math operators.
    //==============================================================================
    /** Multiplies this by a split-complex vector or expression in place. */
    template <is::anySplitComplexVectorOrExpression V>
    constexpr void operator*= (const V& v) requires isMutable;

    /** Multiplies this by a real constant in place. */
    constexpr void operator*= (value_type c) requires isMutable;

    /** Multiplies this by a complex constant in place. */
    constexpr void operator*= (complex_type c) requires isMutable;

    /** Divides this by a split-complex vector or expression in place. */
    template <is::anySplitComplexVectorOrExpression V>
    constexpr void operator/= (const V& v) requires isMutable;

    /** Divides this by a real constant in place. */
    constexpr void operator/= (value_type c) requires isMutable;

    /** Divides this by a complex constant in place. */
    constexpr void operator/= (complex_type c) requires isMutable;

    /** Adds a split-complex vector or expression to this in place. */
    template <is::anySplitComplexVectorOrExpression V>
    constexpr void operator+= (const V& v) requires isMutable;

    /** Adds a real constant to the real parts in place. */
    constexpr void operator+= (value_type c) requires isMutable;

    /** Adds a complex constant to this in place. */
    constexpr void operator+= (complex_type c) requires isMutable;

    /** Subtracts a split-complex vector or expression from this in place. */
    template <is::anySplitComplexVectorOrExpression V>
    constexpr void operator-= (const V& v) requires isMutable;

    /** Subtracts a real constant from the real parts in place. */
    constexpr void operator-= (value_type c) requires isMutable;

    /** Subtracts a complex constant from this in place. */
    constexpr void operator-= (complex_type c) requires isMutable;

private:
    //==============================================================================
    /** Evaluates the real and the imaginary expression in a single pass and writes the results to realParts and imagParts.

        The destination may also be one of the sources, e.g. x = x * y, and both component expressions usually read
        the real and the imaginary parts of their sources. Evaluating the two expressions one after another would
        therefore overwrite the real parts before the imaginary expression has read them. To avoid that without
        allocating a temporary buffer, both expressions are evaluated for the same index (or the same SIMD block of
        indices) before any of the two results are stored. This is safe as long as the expressions are element-wise,
        which is true for all split-complex operations.

        Like VctrBase::assignExpressionTemplate, this picks the best available SIMD instruction set at runtime and
        handles the remaining elements that don't fill a full SIMD register in a scalar loop.
     */
    template <class RealExpression, class ImagExpression>
    constexpr void assignSplitComplexExpression (const RealExpression& re, const ImagExpression& im)
    {
        VCTR_ASSERT (re.size() == size());
        VCTR_ASSERT (im.size() == size());

        if (! std::is_constant_evaluated())
        {
            if constexpr (has::getNeon<RealExpression> && has::getNeon<ImagExpression>)
            {
                assignSplitComplexExpressionNeon (re, im);
                return;
            }

            if constexpr (has::getAVX<RealExpression> && has::getAVX<ImagExpression>)
            {
                if (Config::supportedCPUInstructionSets.fma)
                {
                    assignSplitComplexExpressionFMA (re, im);
                    return;
                }
            }

            if constexpr (has::getSSE<RealExpression> && has::getSSE<ImagExpression>)
            {
                if (Config::supportedCPUInstructionSets.sse4_1)
                {
                    assignSplitComplexExpressionSSE4_1 (re, im);
                    return;
                }
            }
        }

        assignSplitComplexExpressionScalar (re, im, 0);
    }

    /** Evaluates the elements from startIdx to the end one at a time. Also used for the tail of the SIMD loops. */
    template <class RealExpression, class ImagExpression>
    constexpr void assignSplitComplexExpressionScalar (const RealExpression& re, const ImagExpression& im, size_t startIdx)
    {
        auto* r = realParts.data();
        auto* i = imagParts.data();
        const auto n = size();

        for (auto idx = startIdx; idx < n; ++idx)
        {
            const value_type realValue = re[idx];
            const value_type imagValue = im[idx];
            r[idx] = realValue;
            i[idx] = imagValue;
        }
    }

    template <class RealExpression, class ImagExpression>
    void assignSplitComplexExpressionNeon (const RealExpression& re, const ImagExpression& im)
    requires Config::archARM
    {
        constexpr auto inc = NeonRegister<value_type>::numElements;
        const auto nSIMD = detail::previousMultipleOf<inc> (size());

        re.prepareNeonEvaluation();
        im.prepareNeonEvaluation();
        auto* r = realParts.data();
        auto* i = imagParts.data();

        size_t idx = 0;
        for (; idx < nSIMD; idx += inc)
        {
            const auto realValues = re.getNeon (idx);
            const auto imagValues = im.getNeon (idx);
            realValues.store (r + idx);
            imagValues.store (i + idx);
        }

        assignSplitComplexExpressionScalar (re, im, idx);
    }

    template <class RealExpression, class ImagExpression>
    VCTR_TARGET ("fma")
    void assignSplitComplexExpressionFMA (const RealExpression& re, const ImagExpression& im)
    requires Config::archX64
    {
        constexpr auto inc = AVXRegister<value_type>::numElements;
        const auto nSIMD = detail::previousMultipleOf<inc> (size());

        re.prepareAVXEvaluation();
        im.prepareAVXEvaluation();
        auto* r = realParts.data();
        auto* i = imagParts.data();

        size_t idx = 0;
        for (; idx < nSIMD; idx += inc)
        {
            const auto realValues = re.getAVX (idx);
            const auto imagValues = im.getAVX (idx);
            realValues.storeUnaligned (r + idx);
            imagValues.storeUnaligned (i + idx);
        }

        assignSplitComplexExpressionScalar (re, im, idx);
    }

    template <class RealExpression, class ImagExpression>
    VCTR_TARGET ("sse4.1")
    void assignSplitComplexExpressionSSE4_1 (const RealExpression& re, const ImagExpression& im)
    requires Config::archX64
    {
        constexpr auto inc = SSERegister<value_type>::numElements;
        const auto nSIMD = detail::previousMultipleOf<inc> (size());

        re.prepareSSEEvaluation();
        im.prepareSSEEvaluation();
        auto* r = realParts.data();
        auto* i = imagParts.data();

        size_t idx = 0;
        for (; idx < nSIMD; idx += inc)
        {
            const auto realValues = re.getSSE (idx);
            const auto imagValues = im.getSSE (idx);
            realValues.storeUnaligned (r + idx);
            imagValues.storeUnaligned (i + idx);
        }

        assignSplitComplexExpressionScalar (re, im, idx);
    }

    //==============================================================================
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
