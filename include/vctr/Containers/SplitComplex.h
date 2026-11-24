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
/** Maps a container type to the real valued container type a SplitComplexBase uses to store the real and imaginary
    parts. Used by the SplitComplexBase deduction guide.

    Containers holding std::complex values are mapped to the equivalent container holding the underlying real
    type, all other types are passed through unchanged.
 */
template <class T>
struct SplitComplexContainerType
{
    using Type = T;
};

/** Maps Vector<std::complex<T>> to Vector<T>, keeping the allocator. */
template <class T, template <class> class A>
struct SplitComplexContainerType<Vector<std::complex<T>, A>>
{
    using Type = Vector<T, A>;
};

/** Maps Array<std::complex<T>, e> to Array<T, e>.

    The storage extent of an Array depends on its element type, so it must not be taken over from the complex array.
 */
template <class T, size_t e, size_t s>
struct SplitComplexContainerType<Array<std::complex<T>, e, s>>
{
    using Type = Array<T, e>;
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
    /** The real valued type both component expressions produce. */
    using value_type = typename RealExpression::value_type;

    /** The expression producing the real parts. */
    RealExpression realExpression;

    /** The expression producing the imaginary parts. */
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

// Forward declaration, needed by the concepts below which are in turn needed by the SplitComplexBase declaration.
template <is::anyVctrWithRealFloatValueType ContainerTypeParam>
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
} // namespace is

/** A container for complex values that stores the real and imaginary parts in two separate, equally sized buffers
    instead of interleaved like std::complex does.

    This layout allows element-wise complex math to be evaluated without any dependencies between neighbouring
    elements, which is ideal for SIMD vectorisation. The math operations are available as allocation free free
    functions, see SplitComplexMath.h.

    The ContainerTypeParam decides about ownership and size: Use SplitComplexVector for an owning, resizable container,
    SplitComplexArray for an owning container with a compile time size and SplitComplexSpan for a non-owning view.
    Use SplitComplexSpan<const T> for a read-only view.

    Interleaved std::complex containers convert to split-complex containers through the constructor and assignment
    operator. To convert a split-complex container or expression back into interleaved values, use
    vctr::realImagToComplex.

    @tparam ContainerTypeParam   Any VCTR container holding a real floating point value_type, e.g. Vector<float>,
                                 Array<float, n> or Span<float>. A const qualifier on the container type is ignored, use a
                                 const value type to make the elements read-only.

    @ingroup Core
 */
template <is::anyVctrWithRealFloatValueType ContainerTypeParam>
class SplitComplexBase
{
public:
    //==============================================================================
    /** The type of the containers storing the real and imaginary parts. This is ContainerTypeParam without a const
        qualifier, so the members stay assignable and movable even if a const container type is passed.
     */
    using ContainerType = std::remove_const_t<ContainerTypeParam>;

    /** The real floating point type of the real and imaginary parts, e.g. float or double. */
    using value_type = std::remove_const_t<typename ContainerType::value_type>;

    /** The std::complex type matching value_type, returned by the element access functions. */
    using ComplexType = std::complex<value_type>;

    /** The element type of the underlying containers, which is const qualified for a SplitComplexSpan<const T>.

        Note that ContainerType::value_type can't be used for this, as VctrBase strips the const qualifier from it.
     */
    using element_type = std::remove_reference_t<decltype (*std::declval<ContainerType&>().data())>;

    /** True if the elements can be modified through this container, false for a SplitComplexSpan<const T>. */
    static constexpr bool isMutable = ! std::is_const_v<element_type>;

    //==============================================================================
    // Constructors
    //==============================================================================

    /** Creates a default constructed SplitComplexBase. Only available for owning containers.

        For a SplitComplexVector this creates an empty container, for a SplitComplexArray the elements
        are uninitialised, just like for a default constructed vctr::Array.
     */
    constexpr SplitComplexBase() requires std::default_initializable<ContainerType> = default;

    /** Creates a SplitComplexBase with the given size and all values initialised to zero. Only available for a SplitComplexVector. */
    explicit SplitComplexBase (size_t size)
    requires is::vctrVector<ContainerType>
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
    requires (! std::same_as<Other, ContainerTypeParam>) && std::constructible_from<ContainerType, Span<value_type>>
    constexpr SplitComplexBase (SplitComplexBase<Other>& other)
        : realParts (other.real()),
          imagParts (other.imag())
    { }

    /** @copydoc SplitComplexBase(SplitComplexBase<Other>&) */
    template <class Other>
    requires (! std::same_as<Other, ContainerTypeParam>) && std::constructible_from<ContainerType, Span<const value_type>>
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
        if constexpr (is::vctrVector<ContainerType>)
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

    /** Copy constructor. Owning containers copy the data, views refer to the same data as the other view. */
    constexpr SplitComplexBase (const SplitComplexBase&) = default;

    /** Move constructor. A SplitComplexVector takes over the buffers of the other container. */
    constexpr SplitComplexBase (SplitComplexBase&&) = default;

    /** Copy and move assignment of owning containers, e.g. a SplitComplexVector or SplitComplexArray. */
    constexpr SplitComplexBase& operator= (const SplitComplexBase&)
    requires (! is::view<ContainerType>) = default;

    /** @copydoc operator=(const SplitComplexBase&) */
    constexpr SplitComplexBase& operator= (SplitComplexBase&&)
    requires (! is::view<ContainerType>) = default;

    /** Copies the elements viewed by another span of the same type into the elements viewed by this span.

        Like vctr::Span, a SplitComplexSpan never rebinds on assignment. This makes assigning a span of the same type
        behave like assigning any other split complex container. Both spans need to have the same size.
        A read-only SplitComplexSpan<const T> can't be assigned to.
     */
    constexpr SplitComplexBase& operator= (const SplitComplexBase& other)
    requires is::view<ContainerType> && isMutable
    {
        VCTR_ASSERT (size() == other.size());

        realParts = other.real();
        imagParts = other.imag();
        return *this;
    }

    /** @copydoc operator=(const SplitComplexBase&) */
    constexpr SplitComplexBase& operator= (SplitComplexBase&& other)
    requires is::view<ContainerType> && isMutable
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
        if constexpr (is::vctrVector<ContainerType>)
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

    /** Deinterleaves a container of std::complex values of identical size into this container. */
    template <is::anyVctrWithComplexFloatValueType Interleaved>
    requires isMutable
    constexpr void operator= (const Interleaved& interleaved)
    {
        VCTR_ASSERT (size() == interleaved.size());

        const auto* src = interleaved.data();
        const auto n = size();

        for (size_t i = 0; i < n; ++i)
        {
            realParts[i] = src[i].real();
            imagParts[i] = src[i].imag();
        }
    }

    //==============================================================================
    // Element access
    //==============================================================================

    /** Returns a view of the real parts.

        The constness of the elements is deduced from the underlying container: It is read-only for a const owning
        container or a SplitComplexSpan<const T>. Like vctr::Span, a const SplitComplexSpan<T> still returns a mutable view.
     */
    constexpr auto real() const { return Span (realParts); }

    /** Returns a view of the imaginary parts. See real() const for the constness of the returned view. */
    constexpr auto imag() const { return Span (imagParts); }

    /** Returns a view of the real parts. It is mutable unless this is a SplitComplexSpan<const T>. */
    constexpr auto real() { return Span (realParts); }

    /** Returns a view of the imaginary parts. It is mutable unless this is a SplitComplexSpan<const T>. */
    constexpr auto imag() { return Span (imagParts); }

    /** Returns the complex value at index i. */
    constexpr ComplexType operator[] (size_t i) const { return { realParts[i], imagParts[i] }; }

    /** Sets the real and imaginary part at index i. */
    constexpr void set (size_t i, value_type realValue, value_type imagValue)
    requires isMutable
    {
        realParts[i] = realValue;
        imagParts[i] = imagValue;
    }

    /** Sets the complex value at index i. */
    constexpr void set (size_t i, ComplexType value)
    requires isMutable
    {
        set (i, value.real(), value.imag());
    }

    //==============================================================================
    // Size and content manipulation
    //==============================================================================

    /** Returns the number of complex values. */
    constexpr size_t size() const { return realParts.size(); }

    /** Returns true if the container holds no values. */
    constexpr bool empty() const { return realParts.empty(); }

    /** Resizes both parts. Only available for a SplitComplexVector. New elements are zero. */
    constexpr void resize (size_t newSize)
    requires is::vctrVector<ContainerType>
    {
        realParts.resize (newSize);
        imagParts.resize (newSize);
    }

    /** Sets all elements to the given complex value. */
    constexpr void fill (ComplexType value)
    requires isMutable
    {
        realParts.fill (value.real());
        imagParts.fill (value.imag());
    }

    /** Removes all elements, like std::vector::clear. Only available for a SplitComplexVector.

        To set all elements to zero while keeping the size, use fill ({}).
     */
    constexpr void clear()
    requires is::vctrVector<ContainerType>
    {
        realParts.clear();
        imagParts.clear();
    }

    //==============================================================================
    // Math operators.
    //==============================================================================
    // The compound assignment operators build the split-complex expressions defined in SplitComplexMath.h and
    // assign them to this container. The assignment evaluates the real and the imaginary parts in a single pass,
    // so it is safe that this container is also a source of the expression.

    /** Multiplies this by a split-complex vector or expression in place. */
    template <is::anySplitComplexVectorOrExpression V>
    constexpr void operator*= (const V& v)
    requires isMutable
    {
        const auto& self = *this;
        *this = self * v;
    }

    /** Multiplies this by a real constant in place. */
    constexpr void operator*= (value_type c)
    requires isMutable
    {
        const auto& self = *this;
        *this = self * c;
    }

    /** Multiplies this by a complex constant in place. */
    constexpr void operator*= (ComplexType c)
    requires isMutable
    {
        const auto& self = *this;
        *this = self * c;
    }

    /** Divides this by a split-complex vector or expression in place. */
    template <is::anySplitComplexVectorOrExpression V>
    constexpr void operator/= (const V& v)
    requires isMutable
    {
        const auto& self = *this;
        *this = self / v;
    }

    /** Divides this by a real constant in place. */
    constexpr void operator/= (value_type c)
    requires isMutable
    {
        const auto& self = *this;
        *this = self / c;
    }

    /** Divides this by a complex constant in place. */
    constexpr void operator/= (ComplexType c)
    requires isMutable
    {
        const auto& self = *this;
        *this = self / c;
    }

    /** Adds a split-complex vector or expression to this in place. */
    template <is::anySplitComplexVectorOrExpression V>
    constexpr void operator+= (const V& v)
    requires isMutable
    {
        const auto& self = *this;
        *this = self + v;
    }

    /** Adds a real constant to the real parts in place. */
    constexpr void operator+= (value_type c)
    requires isMutable
    {
        const auto& self = *this;
        *this = self + ComplexType (c);
    }

    /** Adds a complex constant to this in place. */
    constexpr void operator+= (ComplexType c)
    requires isMutable
    {
        const auto& self = *this;
        *this = self + c;
    }

    /** Subtracts a split-complex vector or expression from this in place. */
    template <is::anySplitComplexVectorOrExpression V>
    constexpr void operator-= (const V& v)
    requires isMutable
    {
        const auto& self = *this;
        *this = self - v;
    }

    /** Subtracts a real constant from the real parts in place. */
    constexpr void operator-= (value_type c)
    requires isMutable
    {
        const auto& self = *this;
        *this = self - ComplexType (c);
    }

    /** Subtracts a complex constant from this in place. */
    constexpr void operator-= (ComplexType c)
    requires isMutable
    {
        const auto& self = *this;
        *this = self - c;
    }

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

    /** The Neon version of assignSplitComplexExpression. Evaluates full Neon registers, then the remaining elements one at a time. */
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

    /** The AVX version of assignSplitComplexExpression, compiled with FMA support. Evaluates full AVX registers, then the
        remaining elements one at a time.
     */
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

    /** The SSE 4.1 version of assignSplitComplexExpression. Evaluates full SSE registers, then the remaining elements
        one at a time.
     */
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
    /** The container holding (or viewing) the real parts. */
    ContainerType realParts;

    /** The container holding (or viewing) the imaginary parts. It always has the same size as realParts. */
    ContainerType imagParts;
};

/** Deduces the real storage type from an interleaved complex container. */
template <class Interleaved>
SplitComplexBase (const Interleaved&) -> SplitComplexBase<typename detail::SplitComplexContainerType<Interleaved>::Type>;

/** Deduces a SplitComplexVector when constructing from a split-complex expression, like a Vector constructed from an
    expression.
 */
template <is::expression RealExpression, is::expression ImagExpression>
SplitComplexBase (SplitComplexExpression<RealExpression, ImagExpression>&&) -> SplitComplexBase<Vector<typename RealExpression::value_type>>;

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
} // namespace vctr
