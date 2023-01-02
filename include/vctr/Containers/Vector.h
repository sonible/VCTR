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

namespace vctr
{

// clang-format off
/** The default allocator choice for non-arithmetic types is a simple std::allocator. */
template <class ElementType>
struct DefaultVectorAllocator : std::allocator<ElementType>
{
  typedef ElementType value_type;
  DefaultVectorAllocator() noexcept {}
  template <class U> DefaultVectorAllocator (const DefaultVectorAllocator<U>&) noexcept {}
};

/** The default allocator choice for arithmetic types is an AlignedAllocator. */
template <is::number ElementType>
struct DefaultVectorAllocator<ElementType> : AlignedAllocator<ElementType, Config::maxSIMDRegisterSize> {};
// clang-format on

namespace detail
{

template <class T, template <class> class Allocator>
struct StdVector
{
    using Type = std::vector<T, Allocator<T>>;
};

template <template <class> class Allocator>
struct StdVector<bool, Allocator>
{
    using Type = VectorBoolWorkaround<Allocator<std::byte>>;
};

template <class T, template <class> class Allocator>
using StdVectorType = typename StdVector<T, Allocator>::Type;

}

/** The heap-allocated container type.

    Wraps a std::vector with a suitable Allocator. For arithmetic types, including
    std::complex, the default allocator will be a vctr::AlignedAllocator with an alignment
    of 32 bytes to speed up access when using SIMD accelerated evaluations. For other types, it
    will be std::allocator. If you want to override this default choice, simply pass in another
    allocator. This might be handy when you deliberately want a std::vector with the default
    std::allocator as underlying type, e.g. like
    @code
    Vector<float, std::allocator> v;
    @endcode

    There is one special case with Vector<bool>: For this particular case we don't use
    std::vector<bool> internally but a std::vector<std::byte> again wrapped in a thin
    compatibility layer. This allows converting a Vector<bool> into a Span<bool> which creates
    an overall better integration in the VCTR ecosystem but obviously makes it impossible to
    directly reference the underlying std::vector<bool> like it is possible for other types.

    @tparam ElementType The type held by the Vector. May not be const.
    @tparam Allocator   The allocator as described above. You should not need to specify it in most cases.
 */
template <is::nonConst ElementType, template <class> class Allocator = DefaultVectorAllocator>
class Vector : public VctrBase<ElementType, detail::StdVectorType<ElementType, Allocator>, std::dynamic_extent>
{
private:
    using StdVectorType = detail::StdVectorType<ElementType, Allocator>;
    using Vctr = VctrBase<ElementType, StdVectorType, std::dynamic_extent>;

    template <class OtherAllocator>
    static constexpr bool isDifferentAllocatorTypeWithSameValueType = (! std::is_same_v<Allocator<ElementType>, OtherAllocator>) && std::is_same_v<ElementType, typename OtherAllocator::value_type>;

public:
    using value_type = typename Vctr::value_type;

    //==============================================================================

    /** Creates an empty Vector with size = 0. */
    constexpr Vector() = default;

    /** Creates an uninitialised vector of the desired size. */
    constexpr Vector (size_t size) : Vctr (StdVectorType (size)) {}

    /** Creates a Vector with all elements initialised to initialValue of the desired size. */
    constexpr Vector (size_t size, ElementType initialValue) : Vctr (StdVectorType (size, initialValue)) {}

    /** Creates a Vector from an initializer list. */
    constexpr Vector (std::initializer_list<ElementType> il) : Vctr (StdVectorType (il)) {}

    /** Creates a Vector by moving a pack of elements into it. */
    template <is::suitableInitializerForElementType<ElementType>... T>
    constexpr Vector (T&&... elements)
    {
        reserve (sizeof...(elements));
        (push_back (std::move (elements)), ...);
    }

    /** Creates a copy of the other Vector. */
    constexpr Vector (const Vector& other) : Vctr (StdVectorType (other.storage)) {}

    /** Moves the other Vector into this one. */
    constexpr Vector (Vector&& other) : Vctr (std::move (other.storage)) {}

    /** This constructor will create a Vector instance of the same size as OtherContainer
        and will copy its values into this vector.

        OtherContainer has to satisfy triviallyCopyableWithDataAndSize, that is
        - Its elements are trivially copyable
        - It supplies a size() member function returning the number of elements
        - It supplies a data() member function returning a raw pointer to the storage
     */
    template <is::triviallyCopyableWithDataAndSize OtherContainer>
    requires (! std::same_as<Vector, OtherContainer>)
    constexpr Vector (const OtherContainer& other)
    {
        Vctr::copyFrom (other.data(), other.size());
    }

    /** This constructor will create a Vector instance of the same size as OtherContainer
        and will copy its values into this vector.

        OtherContainer has to satisfy iteratorCopyable, that is
        - It supplies a begin() and end() member function returning iterators
        - It does not satisfy triviallyCopyableWithDataAndSize
     */
    template <is::iteratorCopyable OtherContainer>
    constexpr Vector (const OtherContainer& other) : Vector (other.begin(), other.end()) {}

    /** Copies size elements from the memory location pointed to by data. */
    constexpr Vector (const ElementType* data, size_t size)
    {
        Vctr::copyFrom (data, size);
    }

    /** Creates an Vector from an iterator and a sentinel by initialising the Vector with the content read from the iterator. */
    template <is::suitableInputIteratorForElementType<ElementType> Iterator, std::sentinel_for<Iterator> Sentinel>
    constexpr Vector (Iterator first, Sentinel last) : Vctr (StdVectorType (first, last)) {}

    /** Creates a vector of the given size and initialises all elements by calling initializerFunction with the
        element's index.

        @tparam Fn must be a function that takes a size_t argument and returns a suitable element type.
     */
    template <is::suitableInitializerFunctionForElementType<ElementType> Fn>
    constexpr Vector (size_t size, Fn&& initializerFunction)
    {
        reserve (size);
        for (size_t i = 0; i < size; ++i)
            push_back (initializerFunction (i));
    }

    /** Creates a vector from an expression. */
    template <is::expression Expression>
    constexpr Vector (Expression&& e)
    {
        resize (e.size());
        Vctr::assignExpressionTemplate (std::forward<Expression> (e));
    }

    /** Assigns the result of an expression to this vector.

        It may resize the vector if it does not match the expression size.
     */
    template <is::expression E>
    constexpr void operator= (const E& expression)
    {
        resize (expression.size());
        Vctr::assignExpressionTemplate (expression);
    }

    //==============================================================================
    /** Conversion operator that allows us to assign this vector to a std::vector with different allocator
        type.

        Note that values will be copied here and that the destination vector might allocate memory.
     */
    template <class OtherAllocator>
    requires isDifferentAllocatorTypeWithSameValueType<OtherAllocator>
    operator std::vector<ElementType, OtherAllocator>() const
    {
        return std::vector<ElementType, OtherAllocator> (Vctr::begin(), Vctr::end());
    }

    /** Changes the size of this vector, potentially allocating memory.

        This is a standard interface function forwarded to std::vector::resize().
     */
    constexpr void resize (size_t i) { Vctr::storage.resize (i); }

    /** Pre-allocates memory for the given number of elements.

        This is a standard interface function forwarded to std::vector::reserve().
     */
    constexpr void reserve (size_t i) { Vctr::storage.reserve (i); }

    /** Adjusts the allocated memory to the actual number of elements.

        This is a standard interface function forwarded to std::vector::shrink_to_fit().
     */
    constexpr void shrink_to_fit() { Vctr::storage.shrink_to_fit(); }

    /** Adds an element to the end of the vector.

        This is a standard interface function forwarded to std::vector::push_back()
        but it adds a return value.

        @returns a reference to the just added element.
     */
    constexpr ElementType& push_back (ElementType&& newElement) { Vctr::storage.push_back (std::move (newElement)); return Vctr::storage.back(); }

    /** Adds an element to the end of the vector.

        This is a standard interface function forwarded to std::vector::push_back()
        but it adds a return value.

        @returns a reference to the just added element.
     */
    constexpr ElementType& push_back (const ElementType& newElement) { Vctr::storage.push_back (newElement); return Vctr::storage.back(); }

    /** Constructs an element in-place at the end of the vector

        This is a standard interface function forwarded to std::vector::emplace_back().
     */
    template <class... Args>
    constexpr void emplace_back (Args&&... args) { Vctr::storage.emplace_back (std::forward<Args> (args)...); }

    /** Erase all elements from the vector.

        This is a standard interface function forwarded to std::vector::clear().
     */
    constexpr void clear() noexcept { Vctr::storage.clear(); }

    /** Returns the number of elements the vector can currently hold without re-allocation. */
    constexpr size_t capacity() const noexcept { return Vctr::storage.capacity(); }

    /** Returns the allocator associated with the vector.

        This is a standard interface function forwarded to std::vector::get_allocator().
     */
    constexpr Allocator<ElementType> get_allocator() const noexcept { return Vctr::storage.get_allocator(); }

    /** Returns the maximum number of elements the container is able to hold. */
    constexpr size_t max_size() const noexcept { return Vctr::storage.max_size(); }
};

/** A handy shortcut for Vector<std::unique_ptr<OwnedElementType>>. */
template <class OwnedElementType>
class OwnedVector : public Vector<std::unique_ptr<OwnedElementType>>
{
private:
    using VectorType = Vector<std::unique_ptr<OwnedElementType>>;

public:
    OwnedVector() = default;

    /** Creates an OwnedVector from an initializer list of raw pointers that will be owned by the Vector. */
    OwnedVector (std::initializer_list<OwnedElementType*> elementsToOwn)
    {
        VectorType::reserve (elementsToOwn.size());
        for (auto* ptr : elementsToOwn)
            VectorType::emplace_back (ptr);
    }

    /** Forwards every other constructor call to the base class constructor. */
    template <class... Args>
    OwnedVector (Args&&... args) : VectorType (std::forward<Args> (args)...) {}
};

//==============================================================================
// Deduction guides
//==============================================================================

template <is::triviallyCopyableWithDataAndSize OtherContainer>
Vector (const OtherContainer& other) -> Vector<ValueType<OtherContainer>>;

template <is::iteratorCopyable OtherContainer>
Vector (const OtherContainer& other) -> Vector<ValueType<OtherContainer>>;

template <std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
Vector (Iterator first, Sentinel last) -> Vector<std::iter_value_t<Iterator>>;

template <is::suitableInitializerFunction Fn>
Vector (size_t size, Fn&& initializerFunction) -> Vector<std::invoke_result_t<Fn, size_t>>;

template <is::expression Expression>
Vector (Expression&& e) -> Vector<ValueType<Expression>>;

template <is::uniquePtr FirstArg, is::uniquePtr... Args>
requires (are::same<FirstArg, Args...>)
OwnedVector (FirstArg&&, Args&&...) -> OwnedVector<typename FirstArg::element_type>;

} // namespace vctr
