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

/** The base class to all one dimensional containers and views in the VCTR project */
template <class ElementType, class StorageType, size_t extent, class StorageInfoType = StorageInfo<StorageType>>
class VctrBase : private Config,
                 private StorageInfoType
{
public:
    //==============================================================================
    using value_type = std::remove_cv_t<ElementType>;

    //==============================================================================
    // Retrieving size related information.
    //==============================================================================
    /** Returns the extent of this instance, optionally shrank by a certain amount.

        In case the instance specifies a dynamic extent, the return value will always be
        std::dynamic_extent, no matter if we specified an amount to shrink.
        In case the instance specifies a static extent, the return value will be extent - amountToShrink.
     */
    static consteval size_t getExtent (size_t amountToShrink = 0)
    {
        return extent == std::dynamic_extent ? std::dynamic_extent : (extent - amountToShrink);
    }

    /** Returns the number of elements. This overload is a non-static function, used in case the extent is dynamic. */
    constexpr size_t size() const noexcept requires (extent == std::dynamic_extent) { return storage.size(); }

    /** Returns the number of elements. This overload is a static function, used in case the extent is static. */
    static constexpr size_t size() noexcept requires (extent != std::dynamic_extent) { return extent; }

    /** Checks whether the container is empty. */
    constexpr bool empty() const noexcept { return storage.empty(); }

    /** Returns the container size in bytes.

        This overload is a non-static function, used in case the extent is dynamic.
     */
    constexpr size_t sizeInBytes() const noexcept requires (extent == std::dynamic_extent) { return size() * sizeof (value_type); }

    /** Returns the container size in bytes.

        This overload is a static function, used in case the extent is static.
     */
    static constexpr size_t sizeInBytes() noexcept requires (extent != std::dynamic_extent) { return size() * sizeof (value_type); }

    /** Returns the index referring to the last element in the vector.

        This overload is a non-static function, used in case the extent is dynamic.
     */
    constexpr size_t backIdx() const noexcept requires (extent == std::dynamic_extent) { return size() - 1; }

    /** Returns the index referring to the last element in the vector.

        This overload is a static function, used in case the extent is static.
     */
    static constexpr size_t backIdx() noexcept requires (extent != std::dynamic_extent) { return size() - 1; }

    //==============================================================================
    // Accessing elements.
    //==============================================================================
    /** Returns a reference to element i. Asserts in debug builds if i >= size() */
    constexpr auto& operator[] (size_t i)       { VCTR_ASSERT (i < size()); return storage[i]; }

    /** Returns a reference to element i. Asserts in debug builds if i >= size() */
    constexpr auto& operator[] (size_t i) const { VCTR_ASSERT (i < size()); return storage[i]; }

    /** Returns a reference to element i. Throws std::out_of_range if i >= size() */
    constexpr auto& at (size_t i)       { throwIfOutOfRange (i); return storage[i]; }

    /** Returns a reference to element i. Throws std::out_of_range if i >= size() */
    constexpr auto& at (size_t i) const { throwIfOutOfRange (i); return storage[i]; }

    /** Returns a reference to the first element. */
    constexpr auto&& front() { return storage.front(); }

    /** Returns a reference to the first element. */
    constexpr auto&& front() const { return storage.front(); }

    /** Returns a reference to the last element. */
    constexpr auto&& back() { return storage[backIdx()]; }

    /** Returns a reference to the last element. */
    constexpr auto&& back() const { return storage[backIdx()]; }

    /** Returns a raw pointer to the underlying storage */
    auto* data()       { return storage.data(); }

    /** Returns a raw pointer to the underlying storage */
    auto* data() const { return storage.data(); }

    /** Returns an iterator to the begin of the storage */
    constexpr auto begin()       { return storage.begin(); }

    /** Returns a const iterator to the begin of the storage */
    constexpr auto begin() const { return storage.begin(); }

    /** Returns an iterator to the first element behind the storage */
    constexpr auto end()         { return is::stdArray<StorageType> ? storage.begin() + extent : storage.end(); }

    /** Returns a const iterator to the first element behind the storage */
    constexpr auto end()   const { return is::stdArray<StorageType> ? storage.begin() + extent : storage.end(); }

    /** Returns a reverse iterator to the last element in the storage */
    constexpr auto rbegin()       { return std::reverse_iterator (end()); }

    /** Returns a const reverse iterator to the last element in the storage */
    constexpr auto rbegin() const { return std::reverse_iterator (end()); }

    /** Returns a reverse iterator to the element before the first element in the storage */
    constexpr auto rend()         { return std::reverse_iterator (begin()); }

    /** Returns a const reverse iterator to the element before the first element in the storage */
    constexpr auto rend()   const { return std::reverse_iterator (begin()); }

    //==============================================================================
    // Accessing sub spans.
    //==============================================================================
    /** Returns a Span that views a portion of this instance, starting at startIdx with a length of size() - startIdx.

        This overload with a compile time static index will return a Span with a static extent in case this instance also
        has a static extent.
     */
    template <size_t startIdx>
    constexpr auto subSpan()
    {
        assertIsInRange<startIdx>();
        return constCorrectSpan<getExtent (startIdx)> (data() + startIdx, size() - startIdx);
    }

    /** Returns a Span that views a portion of this instance, starting at startIdx with a length of size() - startIdx.

        This overload with a compile time static index will return a Span with a static extent in case this instance also
        has a static extent.
     */
    template <size_t startIdx>
    constexpr auto subSpan() const
    {
        assertIsInRange<startIdx>();
        return constCorrectSpan<getExtent (startIdx)> (data() + startIdx, size() - startIdx);
    }

    /** Returns a Span that views a portion of this instance, starting at startIdx with a length of size() - startIdx.

        This overload with a runtime defined index will always return a Span with a dynamic extent.
     */
    constexpr auto subSpan (size_t startIdx)
    {
        VCTR_ASSERT (startIdx < size());
        return constCorrectSpan<std::dynamic_extent> (data() + startIdx, size() - startIdx);
    }

    /** Returns a Span that views a portion of this instance, starting at startIdx with a length of size() - startIdx.

        This overload with a runtime defined index will always return a Span with a dynamic extent.
     */
    constexpr auto subSpan (size_t startIdx) const
    {
        VCTR_ASSERT (startIdx < size());
        return constCorrectSpan<std::dynamic_extent> (data() + startIdx, size() - startIdx);
    }

    /** Returns a Span that views a portion of this instance, starting at startIdx with a length of numElements.
        
        This overload with a compile time static length will always return a Span with a static extent.
     */
    template <size_t startIdx, size_t numElements>
    constexpr auto subSpan()
    {
        VCTR_ASSERT (int64_t (size()) - int64_t (startIdx) >= numElements);
        return constCorrectSpan<numElements> (data() + startIdx, numElements);
    }

    /** Returns a Span that views a portion of this instance, starting at startIdx with a length of numElements.
        
        This overload with a compile time static length will always return a Span with a static extent.
     */
    template <size_t startIdx, size_t numElements>
    constexpr auto subSpan() const
    {
        VCTR_ASSERT (int64_t (size()) - int64_t (startIdx) >= numElements);
        return constCorrectSpan<numElements> (data() + startIdx, numElements);
    }

    /** Returns a Span that views a portion of this instance, starting at startIdx with a length of numElements.
        
        This overload with a runtime defined length will always return a Span with a dynamic extent.
     */
    constexpr auto subSpan (size_t startIdx, size_t numElements)
    {
        VCTR_ASSERT (int64_t (size()) - int64_t (startIdx) >= numElements);
        return constCorrectSpan<std::dynamic_extent> (data() + startIdx, numElements);
    }

    /** Returns a Span that views a portion of this instance, starting at startIdx with a length of numElements.

        This overload with a runtime defined length will always return a Span with a dynamic extent.
     */
    constexpr auto subSpan (size_t startIdx, size_t numElements) const
    {
        VCTR_ASSERT (int64_t (size()) - int64_t (startIdx) >= numElements);
        return constCorrectSpan<std::dynamic_extent> (data() + startIdx, numElements);
    }

    //==============================================================================
    // Assign data.
    //==============================================================================
    /** Copies the content from otherData to this instance.

        In case the storage of this instance is resizable, it will resize it if necessary. Otherwise, it will
        simply assert that the size matches.
     */
    void copyFrom (const ElementType* otherData, size_t otherSize)
    requires is::triviallyCopyable<ElementType> && is::nonConst<ElementType>
    {
        resizeOrAssertSizeMatches (otherSize);
        std::memcpy (data(), otherData, sizeof (ElementType) * otherSize);
    }

    void copyFrom (const ElementType* otherData, size_t otherSize)
    requires (! is::triviallyCopyable<ElementType>) && is::nonConst<ElementType>
    {
        resizeOrAssertSizeMatches (otherSize);
        std::copy_n (otherData, otherSize, begin());
    }

    //==============================================================================
    // For each functionality.
    //==============================================================================
    /** Calls a function on each element.

        The function returns void and might take the elements by reference to mutate them.
     */
    template <is::functionWithSignatureOrImplicitlyConvertible<void (value_type&)> Fn>
    void forEach (Fn&& fn)
    {
        for (auto& e : *this)
            fn (e);
    }

    /** Calls a function on each element.

        The function returns void and must not mutate the elements.
     */
    template <is::functionWithSignatureOrImplicitlyConvertible<void (const value_type&)> Fn>
    void forEach (Fn&& fn) const
    {
        for (const auto& e : *this)
            fn (e);
    }

    /** Calls a function on each element.

        The function returns a value_type which is assigned as new value to the element.
     */
    template <is::functionWithSignatureOrImplicitlyConvertible<value_type (const value_type&)> Fn>
    void forEach (Fn&& fn)
    {
        for (auto& e : *this)
            e = fn (e);
    }

    /** Calls a function on each element and passes the element index as second function argument.

        The function returns void and might take the elements by reference to mutate them.
     */
    template <is::functionWithSignatureOrImplicitlyConvertible<void (value_type&, size_t)> Fn>
    void forEach (Fn&& fn)
    {
        const auto s = size();
        for (size_t i = 0; i < s; ++i)
            fn (storage[i], i);
    }

    /** Calls a function on each element and passes the element index as second function argument.

        The function returns void and must not mutate the elements.
     */
    template <is::functionWithSignatureOrImplicitlyConvertible<void (const value_type&, size_t)> Fn>
    void forEach (Fn&& fn) const
    {
        const auto s = size();
        for (size_t i = 0; i < s; ++i)
            fn (storage[i], i);
    }

    /** Calls a function on each element and passes the element index as second function argument.

        The function returns a value_type which is assigned as new value to the element.
     */
    template <is::functionWithSignatureOrImplicitlyConvertible<value_type (const value_type&, size_t)> Fn>
    void forEach (Fn&& fn)
    {
        const auto s = size();
        for (size_t i = 0; i < s; ++i)
            storage[i] = fn (storage[i], i);
    }

    /** Calls a function on each element and forwards fnArgs to the function after the value.

        The function returns void and might take the elements by reference to mutate them.
     */
    template <class... Args, is::functionWithSignatureOrImplicitlyConvertible<void (value_type&, Args&&...)> Fn>
    void forEach (Fn&& fn, Args&&... fnArgs)
    {
        for (auto& e : *this)
            fn (e, std::forward<Args> (fnArgs)...);
    }

    /** Calls a function on each element and forwards fnArgs to the function after the value.

        The function returns void and must not mutate the elements.
     */
    template <class... Args, is::functionWithSignatureOrImplicitlyConvertible<void (const value_type&, Args&&...)> Fn>
    void forEach (Fn&& fn, Args&&... fnArgs) const
    {
        for (const auto& e : *this)
            fn (e, std::forward<Args> (fnArgs)...);
    }

    /** Calls a function on each element and forwards fnArgs to the function after the value.

        The function returns void and might take the elements by reference to mutate them.
     */
    template <class... Args, is::functionWithSignatureOrImplicitlyConvertible<value_type (const value_type&, Args&&...)> Fn>
    void forEach (Fn&& fn, Args&&... fnArgs)
    {
        for (auto& e : *this)
            e = fn (e, std::forward<Args> (fnArgs)...);
    }

    //==============================================================================
    // Finding elements and manipulating them.
    //==============================================================================
    /** Returns an iterator to the first element that equals valueToLookFor or end() if none was found */
    template <std::equality_comparable_with<ElementType> T>
    constexpr auto find (const T& valueToLookFor)
    {
        return std::find (begin(), end(), valueToLookFor);
    }

    /** Returns a const iterator to the first element that equals valueToLookFor or end() if none was found */
    template <std::equality_comparable_with<ElementType> T>
    constexpr auto find (const T& valueToLookFor) const
    {
        return std::find (begin(), end(), valueToLookFor);
    }

    /** Returns a reverse iterator to the last element in this vector that equals valueToLookFor or rend() if none was found */
    template <std::equality_comparable_with<ElementType> T>
    constexpr auto findReverse (const T& valueToLookFor)
    {
        return std::find (rbegin(), rend(), valueToLookFor);
    }

    /** Returns a const reverse iterator to the last element in this vector that equals valueToLookFor or rend() if none was found */
    template <std::equality_comparable_with<ElementType> T>
    constexpr auto findReverse (const T& valueToLookFor) const
    {
        return std::find (rbegin(), rend(), valueToLookFor);
    }

    /** Returns an iterator to the first element in this vector for which predicate returns true or end() if none was found */
    template <is::functionWithSignatureOrImplicitlyConvertible<bool (const ElementType&)> Fn>
    constexpr auto findIf (Fn&& predicate)
    {
        return std::find_if (begin(), end(), std::forward<Fn> (predicate));
    }

    /** Returns a const iterator to the first element in this vector for which predicate returns true or end() if none was found */
    template <is::functionWithSignatureOrImplicitlyConvertible<bool (const ElementType&)> Fn>
    constexpr auto findIf (Fn&& predicate) const
    {
        return std::find_if (begin(), end(), std::forward<Fn> (predicate));
    }

    /** Returns a reverse iterator to the last element in this vector for which predicate returns true or rend() if none was found */
    template <is::functionWithSignatureOrImplicitlyConvertible<bool (const ElementType&)> Fn>
    constexpr auto findIfReverse (Fn&& predicate)
    {
        return std::find_if (rbegin(), rend(), std::forward<Fn> (predicate));
    }

    /** Returns a const reverse iterator to the last element in this vector for which predicate returns true or rend() if none was found */
    template <is::functionWithSignatureOrImplicitlyConvertible<bool (const ElementType&)> Fn>
    constexpr auto findIfReverse (Fn&& predicate) const
    {
        return std::find_if (rbegin(), rend(), std::forward<Fn> (predicate));
    }

    /** Returns the number of elements that are equal to valueToLookFor. */
    constexpr size_t count (const ElementType& valueToLookFor) const
    {
        return std::count (begin(), end(), valueToLookFor);
    }

    /** Returns the number of elements that satisfy predicate. */
    template <is::functionWithSignatureOrImplicitlyConvertible<bool (const ElementType&)> Fn>
    constexpr size_t countIf (Fn&& predicate) const
    {
        return std::count_if (begin(), end(), std::forward<Fn> (predicate));
    }

    /** Returns true if all elements satisfy the predicate or if the container is empty */
    template <is::functionWithSignatureOrImplicitlyConvertible<bool (const ElementType&)> Fn>
    constexpr bool all (Fn&& predicate) const
    {
        return std::all_of (begin(), end(), std::forward<Fn> (predicate));
    }

    /** Returns true if one or more elements satisfy the predicate */
    template <is::functionWithSignatureOrImplicitlyConvertible<bool (const ElementType&)> Fn>
    constexpr bool any (Fn&& predicate) const
    {
        return std::any_of (begin(), end(), std::forward<Fn> (predicate));
    }

    /** Returns true if all elements are equal to value or if the container is empty*/
    template <std::equality_comparable_with<ElementType> T>
    constexpr bool allElementsEqual (const T& value) const
    {
        return all ([&] (const auto& e) { return e == value; });
    }

    /** Returns true if all elements are equal to themselves */
    constexpr bool allElementsEqual() const
    requires std::equality_comparable<ElementType>
    {
        if (size() < 2)
            return true;

        return std::all_of (begin() + 1, end(), [&v = storage[0]] (const auto& e) { return v == e; });
    }

    /** Returns true if at least one element is equal to value */
    template <std::equality_comparable_with<ElementType> T>
    constexpr bool contains (const T& value) const
    {
        return find (value) != end();
    }

    //==============================================================================
    // Shuffling and sorting elements
    //==============================================================================
    /** Reverses the order of all elements */
    constexpr void reverse()
    {
        std::reverse (begin(), end());
    }

    /** Rotates the elements so that the element with the index newFirstElementIdx becomes the first
        element and the element with the index newFirstElementIdx - 1 becomes the last element.
     */
    constexpr void rotate (size_t newFirstElementIdx)
    {
        VCTR_ASSERT (newFirstElementIdx < size());
        std::rotate (begin(), begin() + newFirstElementIdx, end());
    }

    /** Shifts all elements to the left by n.

        In other words: It shifts the elements so that the element with the index n becomes the first
        element. If clearFreeSpaceAfterShiftedRegion is true, it clears all elements behind the shifted
        region by setting the memory to 0, otherwise it leaves that region as it was. For maximum
        efficiency, this works on a raw memory basis, so the elements have to be trivially copyable.
     */
    void shiftLeft (size_t n, bool clearFreeSpaceAfterShiftedRegion)
    requires std::is_trivially_copyable_v<ElementType>
    {
        if (n == size())
        {
            if (clearFreeSpaceAfterShiftedRegion)
                clear (data(), size());

            return;
        }

        VCTR_ASSERT (n < size());

        auto numElementsToMove = size() - n;
        move (data() + n, data(), numElementsToMove);

        if (clearFreeSpaceAfterShiftedRegion)
            clear (data() + numElementsToMove, n);
    }

    /** Shifts all elements to the right by n.

        In other words: It shifts the elements so that the previous first element now is the element with the
        index n. If clearFreeSpaceBeforeShiftedRegion is true, it clears all elements before the shifted
        region by setting the memory to 0, otherwise it leaves that region as it was. For maximum
        efficiency, this works on a raw memory basis, so the elements have to be trivially copyable.
     */
    void shiftRight (size_t n, bool clearFreeSpaceBeforeShiftedRegion)
    requires std::is_trivially_copyable_v<ElementType>
    {
        if (n == size())
        {
            if (clearFreeSpaceBeforeShiftedRegion)
                clear (data(), size());

            return;
        }

        VCTR_ASSERT (n < size());

        const auto numElementsToMove = size() - n;
        move (data(), data() + n, numElementsToMove);

        if (clearFreeSpaceBeforeShiftedRegion)
            clear (data(), n);
    }

    //==============================================================================
    // SIMD Register Access
    //==============================================================================
    NeonRegister<std::remove_const_t<ElementType>> getNeon (size_t i) const
    requires archARM && is::realNumber<ElementType>
    {
        VCTR_ASSERT (i % NeonRegister<std::remove_const_t<ElementType>>::numElements == 0);
        return NeonRegister<std::remove_const_t<ElementType>>::load (data() + i);
    }

    VCTR_TARGET ("avx")
    AVXRegister<std::remove_const_t<ElementType>> getAVX (size_t i) const
    requires archX64 && is::realNumber<ElementType>
    {
        VCTR_ASSERT (i % AVXRegister<std::remove_const_t<ElementType>>::numElements == 0);
        if (StorageInfoType::dataIsSIMDAligned)
            return AVXRegister<std::remove_const_t<ElementType>>::loadAligned (data() + i);
        else
            return AVXRegister<std::remove_const_t<ElementType>>::loadUnaligned (data() + i);
    }

    VCTR_TARGET ("sse4.1")
    SSERegister<std::remove_const_t<ElementType>> getSSE (size_t i) const
    requires archX64 && is::realNumber<ElementType>
    {
        VCTR_ASSERT (i % SSERegister<std::remove_const_t<ElementType>>::numElements == 0);
        if (StorageInfoType::dataIsSIMDAligned)
            return SSERegister<std::remove_const_t<ElementType>>::loadAligned (data() + i);
        else
            return SSERegister<std::remove_const_t<ElementType>>::loadUnaligned (data() + i);
    }

    //==============================================================================
    // Expression chain related functions
    //==============================================================================
    /** Evaluates a certain expression in place on this vector, e.g. it assigns the
        expression result back to the vector.
     */
    template <is::expressionChainBuilder ExpressionChain>
    void evalInPlace (const ExpressionChain&)
    {
        using Expression = typename ExpressionChain::template Expression<std::remove_const_t<ElementType>, const VctrBase&>;

        assignExpressionTemplate (Expression (*this));
    }

    constexpr bool isNotAliased (const void*) const { return true; }

    VCTR_FORCEDINLINE const ElementType* evalNextVectorOpInExpressionChain (void*) const { return data(); }

    constexpr const StorageInfoType& getStorageInfo() const { return *this; }

    //==============================================================================
    // Math operators.
    //==============================================================================
    /** Multiplies this by a vector or expression in place. */
    template <is::anyVctrOrExpression V>
    void operator*= (const V& v);

    /** Multiplies this by a constant in place. */
    void operator*= (value_type c);

    /** Divides this by a vector or expression in place. */
    template <is::anyVctrOrExpression V>
    void operator/= (const V& v);

    /** Divides this by a constant in place. */
    void operator/= (value_type c);

    /** Adds a vector or expression to this in place. */
    template <is::anyVctrOrExpression V>
    void operator+= (const V& v);

    /** Adds a constant to this in place. */
    void operator+= (value_type c);

    /** Subtracts a vector or expression from this in place. */
    template <is::anyVctrOrExpression V>
    void operator-= (const V& v);

    /** Subtracts a constant from this in place. */
    void operator-= (value_type c);

    //==============================================================================
    // Math reduction operations.
    //==============================================================================
    /** Returns the minimal value of all elements. */
    ElementType min() const requires std::totally_ordered<ElementType>;

    /** Returns the minimal absolute value of all elements. */
    ElementType minAbs() const requires is::number<ElementType>;

    /** Returns the maximum value of all elements. */
    ElementType max() const requires std::totally_ordered<ElementType>;

    /** Returns the maximum absolute value of all elements. */
    ElementType maxAbs() const requires is::number<ElementType>;

    /** Returns the sum of all elements. */
    ElementType sum() const requires has::operatorPlusEquals<ElementType>;

protected:
    //==============================================================================
    /** Sets numElements elements pointed to by ptr to zero */
    static void clear (ElementType* ptr, size_t numElements)
    {
        std::memset (ptr, 0, numElements * sizeof (ElementType));
    }

    /** Moves numElements elements from src to dst */
    static void move (const ElementType* src, ElementType* dst, size_t numElements)
    requires std::is_trivially_copyable_v<ElementType>
    {
        std::memmove (dst, src, numElements * sizeof (ElementType));
    }

    //==============================================================================
    constexpr VctrBase()
        : StorageInfoType (storage)
    {}

    constexpr VctrBase (StorageType&& s)
        : StorageInfoType (s),
          storage (std::move (s))
    {}

    template <is::storageInfo OtherStorageInfoType>
    constexpr VctrBase (StorageType&& s, const OtherStorageInfoType& otherInfo)
        : StorageInfoType (otherInfo),
          storage (std::move (s))
    {}

    //==============================================================================
    /** In case of a resizable storage, this will resize it to the desired size. */
    void resizeOrAssertSizeMatches (size_t desiredSize)
    requires has::resize<StorageType>
    {
        storage.resize (desiredSize);
    }

    /** Asserts that the current size matches the desired size. */
    void resizeOrAssertSizeMatches (size_t desiredSize) const
    {
        VCTR_ASSERT (size() == desiredSize);
    }

    /** A runtime assert that the template size argument is not greater than the runtime defined size. */
    template <size_t i>
    constexpr void assertIsInRange() const
    requires (extent == std::dynamic_extent)
    {
        VCTR_ASSERT (i < size());
    }

    /** A static assert that the template size argument is not greater than the compile time defined extent. */
    template <size_t i>
    constexpr void assertIsInRange() const
    requires (extent != std::dynamic_extent)
    {
        static_assert (i < extent);
    }

    /** Throws std::out_of_range and triggers an assertion if i is greater than size(). */
    void throwIfOutOfRange (size_t i) const
    {
        if (i >= size())
        {
            VCTR_ASSERT (false);
            throw std::out_of_range ("Vctr of size " + std::to_string (size()) + ": Element " + std::to_string (i) + " is out of range.");
        }
    }

    //==============================================================================
    // clang-format off
    template <size_t spanExtent>
    constexpr auto constCorrectSpan (ElementType* data, size_t spanSize);

    template <size_t spanExtent>
    constexpr auto constCorrectSpan (const ElementType* data, size_t spanSize) const
    requires (! is::stdSpan<StorageType>);

    template <size_t spanExtent>
    constexpr auto constCorrectSpan (ElementType* data, size_t spanSize) const
    requires is::stdSpan<StorageType>;
    // clang-format on

    //==============================================================================
    template <is::expression Expression>
    VCTR_FORCEDINLINE constexpr void assignExpressionTemplate (const Expression& e)
    {
        if (! std::is_constant_evaluated())
        {
            if constexpr (has::evalNextVectorOpInExpressionChain<Expression, ElementType>)
            {
                if (e.isNotAliased (data()))
                {
                    e.evalNextVectorOpInExpressionChain (data());
                    return;
                }
            }

            if constexpr (has::getNeon<Expression>)
            {
                assignExpressionTemplateNeon (e);
                return;
            }

            if constexpr (has::getAVX<Expression>)
            {
                if constexpr (is::floatNumber<ElementType>)
                {
                    if (supportsAVX)
                    {
                        assignExpressionTemplateAVX (e);
                        return;
                    }
                }
                else
                {
                    if (supportsAVX2)
                    {
                        assignExpressionTemplateAVX2 (e);
                        return;
                    }
                }
            }

            if constexpr (has::getSSE<Expression>)
            {
                if (highestSupportedCPUInstructionSet != CPUInstructionSet::fallback)
                {
                    assignExpressionTemplateSSE4_1 (e);
                    return;
                }
            }
        }

        const auto n = size();

        for (size_t i = 0; i < n; ++i)
            storage[i] = e[i];
    }

    template <class Expression>
    void assignExpressionTemplateNeon (const Expression& e)
    requires archARM
    {
        constexpr auto inc = NeonRegister<ElementType>::numElements;
        const bool hasExtendedSIMDStorage = e.getStorageInfo().hasSIMDExtendedStorage && StorageInfoType::hasSIMDExtendedStorage;
        const auto n = storage.size();
        const auto nSIMD = hasExtendedSIMDStorage ? detail::nextMultipleOf<inc> (n) : detail::previousMultipleOf<inc> (n);

        auto* d = data();

        size_t i = 0;
        for (; i < nSIMD; i += inc, d += inc)
            e.getNeon (i).store (d);

        for (; i < n; ++i, ++d)
            storage[i] = e[i];
    }

    template <class Expression>
    VCTR_TARGET ("avx2")
    void assignExpressionTemplateAVX2 (const Expression& e)
    requires archX64
    {
        constexpr auto inc = AVXRegister<ElementType>::numElements;
        const bool hasExtendedSIMDStorage = e.getStorageInfo().hasSIMDExtendedStorage && StorageInfoType::hasSIMDExtendedStorage;
        const auto n = storage.size();
        const auto nSIMD = hasExtendedSIMDStorage ? detail::nextMultipleOf<inc> (n) : detail::previousMultipleOf<inc> (n);

        auto* d = data();

        if (StorageInfoType::dataIsSIMDAligned)
        {
            size_t i = 0;
            for (; i < nSIMD; i += inc, d += inc)
                e.getAVX (i).storeAligned (d);

            for (; i < n; ++i, ++d)
                storage[i] = e[i];
        }
        else
        {
            size_t i = 0;
            for (; i < nSIMD; i += inc, d += inc)
                e.getAVX (i).storeUnaligned (d);

            for (; i < n; ++i, ++d)
                storage[i] = e[i];
        }
    }

    template <class Expression>
    VCTR_TARGET ("avx")
    void assignExpressionTemplateAVX (const Expression& e)
    requires archX64
    {
        constexpr auto inc = AVXRegister<ElementType>::numElements;
        const bool hasExtendedSIMDStorage = e.getStorageInfo().hasSIMDExtendedStorage && StorageInfoType::hasSIMDExtendedStorage;
        const auto n = storage.size();
        const auto nSIMD = hasExtendedSIMDStorage ? detail::nextMultipleOf<inc> (n) : detail::previousMultipleOf<inc> (n);

        auto* d = data();

        if (StorageInfoType::dataIsSIMDAligned)
        {
            size_t i = 0;
            for (; i < nSIMD; i += inc, d += inc)
                e.getAVX (i).storeAligned (d);

            for (; i < n; ++i, ++d)
                storage[i] = e[i];
        }
        else
        {
            size_t i = 0;
            for (; i < nSIMD; i += inc, d += inc)
                e.getAVX (i).storeUnaligned (d);

            for (; i < n; ++i, ++d)
                storage[i] = e[i];
        }
    }

    template <class Expression>
    VCTR_TARGET ("sse4.1")
    void assignExpressionTemplateSSE4_1 (const Expression& e)
    requires archX64
    {
        constexpr auto inc = SSERegister<ElementType>::numElements;
        const bool hasExtendedSIMDStorage = e.getStorageInfo().hasSIMDExtendedStorage && StorageInfoType::hasSIMDExtendedStorage;
        const auto n = storage.size();
        const auto nSIMD = hasExtendedSIMDStorage ? detail::nextMultipleOf<inc> (n) : detail::previousMultipleOf<inc> (n);

        auto* d = data();

        if (StorageInfoType::dataIsSIMDAligned)
        {
            size_t i = 0;
            for (; i < nSIMD; i += inc, d += inc)
                e.getSSE (i).storeAligned (d);

            for (; i < n; ++i, ++d)
                storage[i] = e[i];
        }
        else
        {
            size_t i = 0;
            for (; i < nSIMD; i += inc, d += inc)
                e.getSSE (i).storeUnaligned (d);

            for (; i < n; ++i, ++d)
                storage[i] = e[i];
        }
    }

    //==============================================================================
    alignas (StorageInfoType::memberAlignment) StorageType storage;
};

} // namespace vctr
