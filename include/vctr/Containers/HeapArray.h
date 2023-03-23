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
template <is::nonConst ElementType, size_t extent, template <class> class Allocator = DefaultVectorAllocator>
class HeapArray : public VctrBase<ElementType, detail::StdVectorType<ElementType, Allocator>, extent>
{
private:
    using StdVectorType = detail::StdVectorType<ElementType, Allocator>;
    using Vctr = VctrBase<ElementType, StdVectorType, extent>;

    template <class OtherAllocator>
    static constexpr bool isDifferentAllocatorTypeWithSameValueType = (! std::is_same_v<Allocator<ElementType>, OtherAllocator>) &&std::is_same_v<ElementType, typename OtherAllocator::value_type>;

    using ConstIterator = typename StdVectorType::const_iterator;

public:
    //==============================================================================
    using value_type = typename Vctr::value_type;

    //==============================================================================
    // Constructors
    //==============================================================================
    /** Creates an uninitialised HeapArray */
    constexpr HeapArray() : Vctr (StdVectorType (extent)) {}

    /** Creates an HeapArray with all elements initialised to initialValue */
    constexpr HeapArray (ElementType initialValue) : Vctr (StdVectorType (extent, initialValue)) {}

    /** Creates a HeapArray from an initializer list. */
    constexpr HeapArray (std::initializer_list<ElementType> il) : Vctr (StdVectorType (il)) {}

    /** Creates a Vector by moving a pack of elements into it. */
    template <is::suitableInitializerForElementType<ElementType> First, std::same_as<First>... Other>
    requires (sizeof...(Other) > 0) && (! std::convertible_to<First, size_t>)
    constexpr HeapArray (First&& first, Other&&... other)
    {
        Vctr::storage.reserve (1 + sizeof...(other));
        push_back (std::forward<First> (first));
        (push_back (std::forward<Other> (other)), ...);
    }

    /** Creates a copy of the other HeapArray. */
    constexpr HeapArray (const HeapArray& other) : Vctr (StdVectorType (other.storage)) {}

    /** Moves the other HeapArray into this one. */
    constexpr HeapArray (HeapArray&& other) : Vctr (std::move (other.storage)) {}

    /** This constructor will create a HeapArray instance of the same size as OtherContainer
        and will copy its values into this HeapArray.

        OtherContainer has to satisfy triviallyCopyableWithDataAndSize, that is
        - Its elements are trivially copyable
        - It supplies a size() member function returning the number of elements
        - It supplies a data() member function returning a raw pointer to the storage
     */
    template <is::triviallyCopyableWithDataAndSize OtherContainer>
    requires (! std::same_as<HeapArray, OtherContainer>)
    constexpr HeapArray (const OtherContainer& other)
        : Vctr (StdVectorType (other.size()))
    {
        VCTR_ASSERT (other.size() == extent);
        Vctr::copyFrom (other.data(), other.size());
    }

    /** This constructor will create a HeapArray instance of the same size as OtherContainer
        and will copy its values into this HeapArray.

        OtherContainer has to satisfy iteratorCopyable, that is
        - It supplies a begin() and end() member function returning iterators
        - It does not satisfy triviallyCopyableWithDataAndSize
     */
    template <is::iteratorCopyable OtherContainer>
    constexpr HeapArray (const OtherContainer& other)
        : HeapArray (other.begin(), other.end())
    {
        VCTR_ASSERT (std::distance (other.begin(), other.end()) == extent);
    }

    /** Copies size elements from the memory location pointed to by data. */
    constexpr HeapArray (const ElementType* data, size_t size)
        : Vctr (StdVectorType (size))
    {
        VCTR_ASSERT (size == extent);
        Vctr::copyFrom (data, size);
    }

    /** Creates an HeapArray from an iterator and a sentinel by initialising the Vector with the content read from the iterator. */
    template <is::inputIteratorToConstructValuesOfType<ElementType> Iterator, std::sentinel_for<Iterator> Sentinel>
    constexpr HeapArray (Iterator first, Sentinel last)
        : Vctr (StdVectorType (first, last))
    {
        VCTR_ASSERT (std::distance (first, last) == extent);
    }

    /** Creates a HeapArray of the given size and initialises all elements by calling initializerFunction with the
        element's index.

        @tparam Fn must be a function that takes a size_t argument and returns a suitable element type.
     */
    template <is::suitableInitializerFunctionForElementType<ElementType> Fn>
    constexpr HeapArray (Fn&& initializerFunction)
    {
        Vctr::storage.reserve (extent);
        for (size_t i = 0; i < extent; ++i)
            push_back (initializerFunction (i));
    }

    /** Creates a Vector from an expression. */
    template <is::expression Expression>
    constexpr HeapArray (Expression&& e)
        : Vctr (StdVectorType { extent })
    {
        VCTR_ASSERT (e.size() == extent);
        Vctr::assignExpressionTemplate (std::forward<Expression> (e));
    }

    //==============================================================================
    // Operators
    //==============================================================================
    /** Copies or moves the data of the source container to this Vector. This Vector will be resized to fit if needed. */
    template <has::sizeAndDataWithElementType<ElementType> Container>
    constexpr HeapArray& operator= (Container&& containerToCopyDataFrom)
    {
        if constexpr (has::begin<Container> &&
                      has::end<Container> &&
                      (! is::view<Container>) &&
                      (! std::is_reference_v<Container>))
        {
            if constexpr (std::same_as<Container, HeapArray>)
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
    constexpr HeapArray& operator= (const HeapArray& other)
    {
        // Forwards to the generic operator above. This is needed since the compiler does not recognize
        // the generic implementation as a copy assignment operator.
        return *this = other.storage;
    }

    /** Assigns elements from the initializer list to this HeapArray. This HeapArray will be resized to fit if needed. */
    constexpr HeapArray& operator= (std::initializer_list<ElementType> elementsToAssign)
    {
        VCTR_ASSERT (elementsToAssign.size() == extent);
        Vctr::storage.assign (std::move (elementsToAssign));
        return *this;
    }

    /** Assigns the result of an expression to this HeapArray.

        It may resize the HeapArray if it does not match the expression size.
     */
    template <is::expression E>
    constexpr void operator= (const E& expression)
    {
        VCTR_ASSERT (expression.size() == extent);
        Vctr::assignExpressionTemplate (expression);
    }

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

    constexpr void swap (HeapArray& other) noexcept
    {
        Vctr::storage.swap (other.storage);
    }
};

/** A handy shortcut for Vector<std::unique_ptr<OwnedElementType>>. */
//template <class OwnedElementType>
//class OwnedVector : public Vector<std::unique_ptr<OwnedElementType>>
//{
//private:
//    using VectorType = Vector<std::unique_ptr<OwnedElementType>>;
//
//public:
//    OwnedVector() = default;
//
//    /** Creates an OwnedVector from an initializer list of raw pointers that will be owned by the Vector. */
//    OwnedVector (std::initializer_list<OwnedElementType*> elementsToOwn)
//    {
//        VectorType::reserve (elementsToOwn.size());
//        for (auto* ptr : elementsToOwn)
//            VectorType::emplace_back (ptr);
//    }
//
//    /** Forwards every other constructor call to the base class constructor. */
//    template <class... Args>
//    OwnedVector (Args&&... args) : VectorType (std::forward<Args> (args)...) {}
//};

//==============================================================================
// Deduction guides
//==============================================================================

//template <is::triviallyCopyableWithDataAndSize OtherContainer>
//HeapArray (const OtherContainer& other) -> HeapArray<ValueType<OtherContainer>>;
//
//template <is::iteratorCopyable OtherContainer>
//HeapArray (const OtherContainer& other) -> HeapArray<ValueType<OtherContainer>>;
//
//template <std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
//HeapArray (Iterator first, Sentinel last) -> HeapArray<std::iter_value_t<Iterator>>;
//
//template <is::suitableInitializerFunction Fn>
//Vector (size_t size, Fn&& initializerFunction) -> Vector<std::invoke_result_t<Fn, size_t>>;
//
//template <is::expression Expression>
//Vector (Expression&& e) -> Vector<ValueType<Expression>>;
//
//template <is::uniquePtr FirstArg, is::uniquePtr... Args>
//requires (are::same<FirstArg, Args...>)
//OwnedVector (FirstArg&&, Args&&...) -> OwnedVector<typename FirstArg::element_type>;

} // namespace vctr
