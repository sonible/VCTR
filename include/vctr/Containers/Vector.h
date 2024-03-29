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
struct DefaultVectorAllocator
{
    using DefaultVectorAllocatorType = std::allocator<ElementType>;
};

/** The default allocator choice for arithmetic types is an AlignedAllocator. */
template <is::number ElementType>
struct DefaultVectorAllocator<ElementType>
{
    using DefaultVectorAllocatorType = AlignedAllocator<ElementType, Config::maxSIMDRegisterSize>;
};
// clang-format on

namespace detail
{

template <class Allocator>
struct UnderlyingAllocatorExtractor
{
    using Type = Allocator;
};

template <class ElementType>
struct UnderlyingAllocatorExtractor<DefaultVectorAllocator<ElementType>>
{
    using Type = typename DefaultVectorAllocator<ElementType>::DefaultVectorAllocatorType;
};

template <class T, template <class> class Allocator>
struct StdVector
{
    using Type = std::vector<T, typename UnderlyingAllocatorExtractor<Allocator<T>>::Type>;
};

template <template <class> class Allocator>
struct StdVector<bool, Allocator>
{
    using Type = VectorBoolWorkaround<typename UnderlyingAllocatorExtractor<Allocator<std::byte>>::Type>;
};

template <class T, template <class> class Allocator>
using StdVectorType = typename StdVector<T, Allocator>::Type;

} // namespace detail

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

    @ingroup Core
 */
template <is::nonConst ElementType, template <class> class Allocator = DefaultVectorAllocator>
class Vector : public VctrBase<ElementType, detail::StdVectorType<ElementType, Allocator>, std::dynamic_extent>
{
private:
    using StdVectorType = detail::StdVectorType<ElementType, Allocator>;
    using Vctr = VctrBase<ElementType, StdVectorType, std::dynamic_extent>;

    template <class OtherAllocator>
    static constexpr bool isDifferentAllocatorTypeWithSameValueType = (! std::is_same_v<Allocator<ElementType>, OtherAllocator>) &&std::is_same_v<ElementType, typename OtherAllocator::value_type>;

    using ConstIterator = typename StdVectorType::const_iterator;

public:
    //==============================================================================
    using value_type = typename Vctr::value_type;

    //==============================================================================
    // Constructors
    //==============================================================================
    /** Creates an empty Vector with size = 0. */
    constexpr Vector() = default;

    /** Creates an empty Vector that uses the specified allocator instance. */
    template <std::convertible_to<Allocator<ElementType>> Alloc>
    Vector (Alloc&& allocator) : Vctr (StdVectorType (allocator)) {}

    /** Creates an uninitialised Vector of the desired size. */
    constexpr Vector (size_t size) : Vctr (StdVectorType (size)) {}

    /** Creates a Vector with all elements initialised to initialValue of the desired size. */
    constexpr Vector (size_t size, ElementType initialValue) : Vctr (StdVectorType (size, initialValue)) {}

    /** Creates a Vector from an initializer list. */
    constexpr Vector (std::initializer_list<ElementType> il) : Vctr (StdVectorType (il)) {}

    /** Creates a Vector by moving a pack of elements into it. */
    template <is::suitableInitializerForElementType<ElementType> First, std::same_as<First>... Other>
    requires (sizeof...(Other) > 0) && (! std::convertible_to<First, size_t>)
    constexpr Vector (First&& first, Other&&... other)
    {
        reserve (1 + sizeof...(other));
        push_back (std::forward<First> (first));
        (push_back (std::forward<Other> (other)), ...);
    }

    /** Creates a copy of the other Vector. */
    constexpr Vector (const Vector& other) : Vctr (StdVectorType (other.storage)) {}

    /** Moves the other Vector into this one. */
    constexpr Vector (Vector&& other) : Vctr (std::move (other.storage)) {}

    /** This constructor will create a Vector instance of the same size as OtherContainer
        and will copy its values into this Vector.

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
        and will copy its values into this Vector.

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
    template <is::inputIteratorToConstructValuesOfType<ElementType> Iterator, std::sentinel_for<Iterator> Sentinel>
    constexpr Vector (Iterator first, Sentinel last) : Vctr (StdVectorType (first, last)) {}

    /** Creates a Vector of the given size and initialises all elements by calling initializerFunction with the
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

    /** Creates a Vector from an expression. */
    template <is::expression Expression>
    constexpr Vector (Expression&& e)
    {
        resize (e.size());
        Vctr::assignExpressionTemplate (std::forward<Expression> (e));
    }

    //==============================================================================
    // Operators
    //==============================================================================
    /** Copies or moves the data of the source container to this Vector. This Vector will be resized to fit if needed. */
    template <has::sizeAndDataWithElementType<ElementType> Container>
    constexpr Vector& operator= (Container&& containerToCopyDataFrom)
    {
        if constexpr (has::begin<Container> &&
                      has::end<Container> &&
                      (! is::view<Container>) &&
                      (! std::is_reference_v<Container>))
        {
            if constexpr (std::same_as<Container, Vector>)
            {
                Vctr::storage = std::move (containerToCopyDataFrom.storage);
            }
            else if constexpr (std::same_as<Container, StdVectorType>)
            {
                Vctr::storage = std::move (containerToCopyDataFrom);
            }
            else
            {
                Vctr::storage.assign (std::make_move_iterator (containerToCopyDataFrom.begin()), std::make_move_iterator (containerToCopyDataFrom.end()));
            }
        }
        else
        {
            Vctr::copyFrom (containerToCopyDataFrom.data(), containerToCopyDataFrom.size());
        }

        return *this;
    }

    /** Copy assignment operator. */
    constexpr Vector& operator= (const Vector& other)
    {
        // Forwards to the generic operator above. This is needed since the compiler does not recognize
        // the generic implementation as a copy assignment operator.
        return *this = other.storage;
    }

    /** Assigns elements from the intializer list to this Vector. This Vector will be resized to fit if needed. */
    constexpr Vector& operator= (std::initializer_list<ElementType> elementsToAssign)
    {
        Vctr::storage.assign (std::move (elementsToAssign));
        return *this;
    }

    /** Assigns the result of an expression to this Vector.

        It may resize the Vector if it does not match the expression size.
     */
    template <is::expression E>
    constexpr void operator= (const E& expression)
    {
        resize (expression.size());
        Vctr::assignExpressionTemplate (expression);
    }

    //==============================================================================
    // Accessing the underlying vector.
    //==============================================================================
    /** Returns a reference to the underlying storage.

        The returned type is std::vector, possibly with a custom allocator type.
     */
    constexpr StdVectorType& getUnderlyingVector() { return Vctr::storage; }

    /** Returns a reference to the underlying storage.

       The returned type is std::vector, possibly with a custom allocator type.
     */
    constexpr const StdVectorType& getUnderlyingVector() const { return Vctr::storage; }

    /** Moves the underlying storage out of this wrapper class.

       The returned type is std::vector, possibly with a custom allocator type.
     */
    constexpr StdVectorType&& moveUnderlyingVector() && { return std::move (Vctr::storage); }

    /** Conversion operator shortcut to getUnderlyingVector. */
    constexpr operator StdVectorType&() { return Vctr::storage; }

    /** Conversion operator shortcut to getUnderlyingVector. */
    constexpr operator const StdVectorType&() const { return Vctr::storage; }

    /** Conversion operator that allows us to assign this Vector to a std::vector with different allocator
        type.

        Note that values will be copied here and that the destination Vector might allocate memory.
     */
    template <class OtherAllocator>
    requires isDifferentAllocatorTypeWithSameValueType<OtherAllocator>
    operator std::vector<ElementType, OtherAllocator>() const
    {
        return std::vector<ElementType, OtherAllocator> (Vctr::begin(), Vctr::end());
    }

    //==============================================================================
    // Resizing, initialization, and clearing
    //==============================================================================
    /** Changes the size of this Vector, potentially allocating memory.

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

    /** Resizes this vector to newSize and fills it with the given value.

        @see resize, fill.
     */
    constexpr void init (size_t newSize, const value_type& value)
    {
        resize (newSize);
        Vctr::fill (value);
    }

    /** Resizes this vector to newSize and fills it via the given initializerFunction.

        @see resize, fill.
    */
    template <is::suitableInitializerFunctionForElementType<ElementType> Fn>
    constexpr void init (size_t newSize, Fn&& initializerFunction)
    {
        resize (newSize);
        for (size_t i = 0; i < newSize; ++i)
            Vctr::at (i) = initializerFunction (i);
    }

    /** Erase all elements from the Vector.

        This is a standard interface function forwarded to std::vector::clear().
     */
    constexpr void clear() noexcept { Vctr::storage.clear(); }

    //==============================================================================
    // Allocator
    //==============================================================================
    /** Returns the allocator associated with the Vector.

        This is a standard interface function forwarded to std::vector::get_allocator().
     */
    constexpr Allocator<ElementType> get_allocator() const noexcept { return Vctr::storage.get_allocator(); }

    //==============================================================================
    // Size and capacity
    //==============================================================================
    /** Returns the number of elements the Vector can currently hold without re-allocation. */
    constexpr size_t capacity() const noexcept { return Vctr::storage.capacity(); }

    /** Returns the maximum number of elements the container is able to hold. */
    constexpr size_t max_size() const noexcept { return Vctr::storage.max_size(); }

    //==============================================================================
    // Erasing elements from the Vector
    //==============================================================================
    /** Erases the element referenced by elementToErase and returns the iterator to the element behind it.

        This is a standard interface function forwarded to std::vector::erase().
     */
    constexpr auto erase (ConstIterator elementToErase)
    {
        // Undefined behaviour if elementToErase does not refer to an element in this Vector
        VCTR_ASSERT (Vctr::contains (elementToErase));
        return Vctr::storage.erase (elementToErase);
    }

    /** Erases the element at index idx and returns the iterator to the element behind it. */
    constexpr auto erase (size_t idx)
    {
        return erase (Vctr::storage.begin() + idx);
    }

    /** Erases the range of elements referenced by it and returns the iterator to the element behind it.

        First is expected to be contained in the range while last is expected to be the first element
        after that range.

        This is a standard interface function forwarded to std::vector::erase().
     */
    constexpr auto erase (ConstIterator first, ConstIterator last)
    {
        // Undefined behaviour if the range between first and last does not refer to a range in this Vector
        VCTR_ASSERT (Vctr::contains (first));
        VCTR_ASSERT (Vctr::contains (last) || last == Vctr::end());

        return Vctr::storage.erase (first, last);
    }

    /** Erases numElements elements starting from index startIdx and returns the iterator to the element behind it. */
    constexpr auto erase (size_t startIdx, size_t numElements)
    {
        auto it = Vctr::storage.begin() + startIdx;
        return erase (it, it + numElements);
    }

    /** Erases the first occurrence of value from this Vector and adjusts its size. */
    template <std::equality_comparable_with<ElementType> T>
    constexpr auto eraseFirstOccurrenceOf (const T& value)
    {
        auto it = Vctr::find (value);
        return it != Vctr::end() ? erase (it) : it;
    }

    /** Erases the first occurrence for which predicate is true from this Vector and adjusts its size. */
    template <is::functionWithSignatureOrImplicitlyConvertible<bool (const ElementType&)> Fn>
    constexpr auto eraseFirstOccurrenceIf (Fn&& predicate)
    {
        auto it = Vctr::findIf (std::forward<Fn> (predicate));
        return it != Vctr::end() ? erase (it) : it;
    }

    /** Removes all occurrences of value from this Vector and adjusts its size.

        Returns true if one or more occurrences have been found and erased, false otherwise.
     */
    template <std::equality_comparable_with<ElementType> T>
    constexpr bool eraseAllOccurrencesOf (const T& value)
    {
        auto beginOfElementsToRemove = std::remove (Vctr::begin(), Vctr::end(), value);
        if (beginOfElementsToRemove == Vctr::end())
            return false;

        erase (beginOfElementsToRemove, Vctr::end());
        return true;
    }

    /** Removes all elements inside this Vector for which predicate is true and adjusts its size.

        Returns true if one or more predicate matches have been found and corresponding elements
        have been erased, false otherwise.
     */
    template <is::functionWithSignatureOrImplicitlyConvertible<bool (const ElementType&)> Fn>
    constexpr bool eraseAllOccurrencesIf (Fn&& predicate)
    {
        auto beginOfElementsToRemove = std::remove_if (Vctr::begin(), Vctr::end(), std::forward<Fn> (predicate));
        if (beginOfElementsToRemove == Vctr::end())
            return false;

        erase (beginOfElementsToRemove, Vctr::end());
        return true;
    }

    //==============================================================================
    // Adding elements to the Vector
    //==============================================================================
    /** Adds an element to the end of the Vector.

        This is a standard interface function forwarded to std::vector::push_back()
        but it adds a return value.

        @returns a reference to the just added element.
     */
    constexpr ElementType& push_back (ElementType&& newElement)
    {
        Vctr::storage.push_back (std::move (newElement));
        return Vctr::storage.back();
    }

    /** Adds an element to the end of the Vector.

        This is a standard interface function forwarded to std::vector::push_back()
        but it adds a return value.

        @returns a reference to the just added element.
     */
    constexpr ElementType& push_back (const ElementType& newElement)
    {
        Vctr::storage.push_back (newElement);
        return Vctr::storage.back();
    }

    /** Removes the last element in the vector, effectively reducing the container size by one.

        In addition to std::vector::pop_back, this implementation also return the popped element.
        Calling pop_back on an empty vector causes undefined behaviour.
     */
    constexpr ElementType pop_back()
    {
        VCTR_ASSERT (! Vctr::empty());
        auto x = std::move (Vctr::storage.back());
        Vctr::storage.pop_back();
        return x;
    }

    /** Constructs an element in-place at the end of the Vector

        This is a standard interface function forwarded to std::vector::emplace_back().
     */
    template <class... Args>
    constexpr void emplace_back (Args&&... args) { Vctr::storage.emplace_back (std::forward<Args> (args)...); }

    /** Swaps the underlying memory with the other Vector.

        This is a standard interface function forwarded to std::vector::swap().
     */
    constexpr void swap (Vector& other) noexcept { Vctr::storage.swap (other.storage); }

    /** Appends a Span, Array or Vector to the end of this Vector, optionally by moving elements from the source */
    template <is::anyVctr VctrToAppend>
    void append (VctrToAppend&& vctrToAppend, bool moveValuesFromSrc = false)
    {
        insert (Vctr::end(), std::forward<VctrToAppend> (vctrToAppend), moveValuesFromSrc);
    }

    /** Inserts value before the element referenced by pos and returns an iterator to the inserted value.

        pos must refer to an element in this Vector.

        This is a standard interface function forwarded to std::vector::insert().
     */
    auto insert (ConstIterator pos, const ElementType& value)
    {
        // Undefined behaviour if pos does not refer to an element in this Vector
        VCTR_ASSERT (pos == Vctr::end() || Vctr::contains (pos));
        return Vctr::storage.insert (pos, value);
    }

    /** Inserts value at index idx and returns an iterator to the inserted value. */
    auto insert (size_t idx, const ElementType& value) { return insert (Vctr::begin() + idx, value); }

    /** Inserts value before the element referenced by pos and returns an iterator to the inserted value.

        pos must refer to an element in this Vector.

        This is a standard interface function forwarded to std::vector::insert().
     */
    auto insert (ConstIterator pos, ElementType&& value)
    {
        // Undefined behaviour if pos does not refer to an element in this Vector
        VCTR_ASSERT (pos == Vctr::end() || Vctr::contains (pos));
        return Vctr::storage.insert (pos, std::move (value));
    }

    /** Inserts value at index idx and returns an iterator to the inserted value. */
    auto insert (size_t idx, ElementType&& value) { return insert (Vctr::begin() + idx, std::move (value)); }

    /** Inserts numCopies copies of value before the element referenced by pos and returns an iterator
        to the inserted values.

        pos must refer to an element in this Vector.

        This is a standard interface function forwarded to std::vector::insert().
     */
    auto insert (ConstIterator pos, size_t numCopies, const ElementType& value)
    {
        // Undefined behaviour if pos does not refer to an element in this Vector
        VCTR_ASSERT (pos == Vctr::end() || Vctr::contains (pos));
        return Vctr::storage.insert (pos, numCopies, value);
    }

    /** Inserts numCopies copies of value at index idx and returns an iterator to the inserted values. */
    auto insert (size_t idx, size_t numCopies, const ElementType& value) { return insert (Vctr::begin() + idx, numCopies, value); }

    /** Inserts a range of values before the element referenced by pos and returns an iterator to the
        inserted values.

        pos must refer to an element in this Vector, first and last must denote the begin and end of
        the range to be inserted. The inserted range must not be part of this Vector.

        This is a standard interface function forwarded to std::vector::insert().
     */
    template <is::inputIteratorToConstructValuesOfType<ElementType> InputIterator>
    auto insert (ConstIterator pos, InputIterator first, InputIterator last)
    {
        // Undefined behaviour if pos does not refer to an element in this Vector
        VCTR_ASSERT (pos == Vctr::end() || Vctr::contains (pos));

        if constexpr (is::contiguousIteratorWithValueTypeSameAs<ElementType, InputIterator>)
        {
            // The range between first and last must not be elements of this Vector
            VCTR_ASSERT (! Vctr::contains (first));
            VCTR_ASSERT (! Vctr::contains (last));
        }

        return Vctr::storage.insert (pos, first, last);
    }

    /** Inserts a range of values at index idx and returns an iterator to the inserted values.

        First and last must denote the begin and end of the range to be inserted. The inserted range
        must not be part of this Vector.
     */
    template <is::inputIteratorToConstructValuesOfType<ElementType> InputIterator>
    auto insert (size_t idx, InputIterator first, InputIterator last)
    {
        return insert (Vctr::begin() + idx, first, last);
    }

    /** Inserts a list of values before the element referenced by pos and returns an iterator to the
        first inserted value.

        pos must refer to an element in this Vector.

        This is a standard interface function forwarded to std::vector::insert().
     */
    auto insert (ConstIterator pos, std::initializer_list<ElementType> initList)
    {
        // Undefined behaviour if pos does not refer to an element in this Vector
        VCTR_ASSERT (pos == Vctr::end() || Vctr::contains (pos));
        return Vctr::storage.insert (pos, initList);
    }

    /** Inserts a list of values at index idx and returns an iterator to the first inserted value. */
    auto insert (size_t idx, std::initializer_list<ElementType> initList) { return insert (Vctr::begin() + idx, std::move (initList)); }

    /** Inserts a VCTR container before the element referenced by pos and returns an iterator to the
        first inserted value.

        pos must refer to an element in this Vector. If moveValuesFromSrc is true, the function attempts
        that it's safe to move the values out of vtrToInsert even if we pass it by reference or as a
        vctr::Span. This will invalidate that source values.
     */
    template <is::anyVctr VctrToInsert>
    auto insert (ConstIterator pos, VctrToInsert&& vctrToInsert, bool moveValuesFromSrc = false)
    {
        // Undefined behaviour if pos does not refer to an element in this Vector
        VCTR_ASSERT (pos == Vctr::end() || Vctr::contains (pos));

        // If it is no view and no reference, the container passed in has no dependencies outside this scope,
        // so we can safely move its elements in every case
        if constexpr ((! is::view<VctrToInsert>) &&(! std::is_reference_v<VctrToInsert>) )
            moveValuesFromSrc = true;

        if (moveValuesFromSrc)
            return Vctr::storage.insert (pos, std::make_move_iterator (vctrToInsert.begin()), std::make_move_iterator (vctrToInsert.end()));

        return Vctr::storage.insert (pos, vctrToInsert.begin(), vctrToInsert.end());
    }

    /** Inserts a VCTR container at index idx and returns an iterator to the first inserted value.

        If moveValuesFromSrc is true, the function attempts that it's safe to move the values out of vtrToInsert
        even if we pass it by reference or as a vctr::Span. This will invalidate that source values.
     */
    template <is::anyVctr VctrToInsert>
    auto insert (size_t idx, VctrToInsert&& vctrToInsert, bool moveValuesFromSrc = false)
    {
        return insert (Vctr::begin() + idx, std::forward<VctrToInsert> (vctrToInsert), moveValuesFromSrc);
    }
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
