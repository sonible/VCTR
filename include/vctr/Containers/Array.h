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

namespace detail
{

template <class T, size_t e>
struct StorageExtent
{
    static constexpr size_t extent = e;
};

template <is::number T, size_t e>
struct StorageExtent<T, e>
{
    static constexpr size_t extent = nextMultipleOf<Config::maxSIMDRegisterSize / sizeof (T)> (e);
};
} // namespace detail

/** The stack-based container type.

    Wraps a std::array of at least the length extent. The size of the underlying std::array might be greater for
    certain types in order to facilitate SIMD loops, however, the size reported by size() and the value of
    extent always refer to the expected size.

    @ingroup Core
 */
template <class ElementType, size_t extent, size_t storageExtent = detail::StorageExtent<ElementType, extent>::extent>
requires (extent != std::dynamic_extent && extent <= storageExtent)
class Array : public VctrBase<ElementType, std::array<ElementType, storageExtent>, extent>
{
protected:
    //==============================================================================
    using Vctr = VctrBase<ElementType, std::array<ElementType, storageExtent>, extent>;
    using StdArrayType = std::array<ElementType, storageExtent>;

public:
    //==============================================================================
    using value_type = typename Vctr::value_type;

    //==============================================================================
    // Constructors
    //==============================================================================

    /** Creates an uninitialised Array.

        In case of constant evaluation the values are actually default initialized since
        uninitialized data structures are not allowed in that case.
     */
    constexpr Array()
    {
        if (std::is_constant_evaluated())
            Vctr::storage.fill (value_type());
    }

    /** Creates an Array with all elements initialised to initialValue */
    constexpr Array (ElementType initialValue)
    requires (extent > 1)
    {
        Vctr::storage.fill (initialValue);
    }

    /** Creates an Array from a list of at least two initial values. */
    template <is::suitableInitializerForElementType<ElementType>... T>
    requires (sizeof...(T) > 1)
    constexpr Array (T&&... initialValues) : Vctr (StdArrayType { std::forward<T> (initialValues)... })
    {}

    /** Creates an Array with extent 1 from an initial value. */
    template <is::suitableSingleElementInitializer T>
    constexpr Array (T&& initialValues) : Vctr (StdArrayType { initialValues }) {}

    /** Creates an Array by moving a std::array into it */
    template <class OtherElementType, size_t otherSize>
    constexpr Array (std::array<OtherElementType, otherSize>&& other) : Vctr (std::move (other)) {}

    /** This constructor will create an Array instance of the same size as OtherContainer
        and will copy its values into this Array.

        OtherContainer has to satisfy triviallyCopyableWithDataAndSize, that is
        - Its elements are trivially copyable
        - It supplies a size() member function returning the number of elements
        - It supplies a data() member function returning a raw pointer to the storage
        - It supplies an operator[] returning the nth element
     */
    template <is::triviallyCopyableWithDataAndSize OtherContainer>
    requires (std::same_as<value_type, ValueType<OtherContainer>> && ! std::same_as<Array, OtherContainer>)
    constexpr Array (const OtherContainer& other)
    {
        if (std::is_constant_evaluated())
        {
            for (size_t i = 0; i < extent; ++i)
                Vctr::storage[i] = other[i];
        }
        else
        {
            Vctr::copyFrom (other.data(), other.size());
        }
    }

    /** This constructor will create an Array instance of the same size as OtherContainer
        and will copy its values into this Array.

        OtherContainer has to satisfy iteratorCopyable, that is
        - It supplies a begin() and end() member function returning iterators
        - It must not satisfy triviallyCopyableWithDataAndSize
     */
    template <is::iteratorCopyable OtherContainer>
    constexpr Array (const OtherContainer& other) : Array (other.begin(), other.end()) {}

    /** Copies size elements from the memory location pointed to by data. */
    constexpr Array (const ElementType* data, size_t size)
    {
        Vctr::copyFrom (data, size);
    }

    /** Creates an Array from an iterator and a sentinel by initialising the Array with the content read from the iterator. */
    template <is::inputIteratorToConstructValuesOfType<ElementType> Iterator, std::sentinel_for<Iterator> Sentinel>
    constexpr Array (Iterator first, [[maybe_unused]] Sentinel last)
    {
        for (size_t i = 0; i < extent; ++i, ++first)
            Vctr::storage[i] = *first;

        if (! std::is_constant_evaluated())
        {
            // If you hit this assertion, the extent of the Array did not match the distance between first and last
            VCTR_ASSERT (first == last);
        }
    }

    /** Creates a vector of the given size and initialises all elements by calling initializerFunction with the
        element's index.

        @tparam Fn must be a function that takes a size_t argument and returns a suitable element type.
     */
    template <is::suitableInitializerFunctionForElementType<ElementType> Fn>
    constexpr Array (Fn&& initializerFunction) : Array (std::forward<Fn> (initializerFunction), std::make_index_sequence<extent>()) {}

    /** Creates an Array from an expression template.

        In case the expressions extent is non-dynamic, e.g. its sources are Arrays or
        Spans with a non-dynamic extent, class template argument deduction will work with this constructor.
        If you are trying to assign an expression with dynamic extent and you know the expected size, you have to
        specify the ElementType and extent manually. In case you don't you will hit the require constraint below.
     */
    template <is::expression Expression>
    constexpr Array (Expression&& e)
    requires (extentOf<Expression> != std::dynamic_extent)
    {
        if (! std::is_constant_evaluated())
            VCTR_ASSERT (e.size() == extent);

        Vctr::assignExpressionTemplate (std::forward<Expression> (e));
    }

    //==============================================================================
    // Operators
    //==============================================================================
    /** Copies or moves the data of the source container to this Array. You have to ensure that the source size matches. */
    template <has::sizeAndDataWithElementType<ElementType> Container>
    constexpr Array& operator= (Container&& containerToCopyDataFrom)
    {
        if constexpr (Vctr::template shouldMoveFromOtherContainer<Container>)
        {
            VCTR_ASSERT (containerToCopyDataFrom.size() == Vctr::size());
            std::copy (std::make_move_iterator (containerToCopyDataFrom.begin()), std::make_move_iterator (containerToCopyDataFrom.end()), Vctr::storage.begin());
        }
        else
        {
            Vctr::copyFrom (containerToCopyDataFrom.data(), containerToCopyDataFrom.size());
        }

        return *this;
    }

    /** Assigns elements from the initializer list to this Array. You have to ensure that the source size matches. */
    constexpr Array& operator= (std::initializer_list<ElementType> elementsToAssign)
    {
        Vctr::assign (std::move (elementsToAssign));
        return *this;
    }

    /** Assigns the result of an expression to this vector.

        The Array has to match the size of the expression.
     */
    template <is::expression E>
    constexpr void operator= (const E& expression)
    {
        if (! std::is_constant_evaluated())
            VCTR_ASSERT (expression.size() == extent);

        Vctr::assignExpressionTemplate (expression);
    }

private:
    //==============================================================================
    template <is::suitableInitializerFunction Fn, size_t... i>
    constexpr Array (Fn&& initializerFunction, std::index_sequence<i...>) : Vctr (StdArrayType { initializerFunction (i)... }) {}
};

/** A handy shortcut for Array<std::unique_ptr<OwnedElementType>, n>. */
template <class OwnedElementType, size_t extent>
class OwnedArray : public Array<std::unique_ptr<OwnedElementType>, extent>
{
private:
    //==============================================================================
    using ArrayType = Array<std::unique_ptr<OwnedElementType>, extent>;

public:
    //==============================================================================
    OwnedArray() = default;

    /** Creates an OwnedArray from a list of raw pointers that will be owned by the Array. */
    template <is::pointer... Pointer>
    OwnedArray (Pointer... elementsToOwn) : OwnedArray (std::unique_ptr<OwnedElementType> (elementsToOwn)...) {}

    /** Forwards every other constructor call to the base class constructor. */
    template <is::noPointer... Args>
    OwnedArray (Args&&... args) : ArrayType (std::forward<Args> (args)...) {}
};

//==============================================================================
// Deduction guides
//==============================================================================

template <class OtherElementType, size_t otherSize>
Array (std::array<OtherElementType, otherSize>&&) -> Array<OtherElementType, otherSize, otherSize>;

template <class First, is::suitableInitializerForElementType<std::remove_cvref_t<First>>... Other>
Array (First&&, Other&&...) -> Array<std::remove_cvref_t<First>, sizeof...(Other) + 1>;

namespace detail
{

// Clang failed to respect constrained single template types in deduction guides for some reasons.
// Using this as workaround for the single argument deduction guide below
template <class T>
struct SingleArgDeductionHelper
{
    using Type = std::remove_cvref_t<T>;
    static constexpr size_t extent = 1;
};

template <has::size T>
struct SingleArgDeductionHelper<T>
{
    using Type = vctr::ValueType<T>;
    static constexpr size_t extent = extentOf<T>;
};

} // namespace detail

template <class SingleArg>
Array (SingleArg&&) -> Array<typename detail::SingleArgDeductionHelper<SingleArg>::Type, detail::SingleArgDeductionHelper<SingleArg>::extent>;

template <class Pointer, std::same_as<Pointer>... Pointers>
OwnedArray (Pointer*, Pointers*...) -> OwnedArray<Pointer, sizeof...(Pointers) + 1>;

} // namespace vctr
