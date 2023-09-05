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

/** A StaticStorageInfo type alias suitable for vctr::Span.

    Useful if you want to define a Span type to point to some externally managed
    raw data pointer that you know to be SIMD aligned.
    @code
    float a alignas (32) [12]

    using SIMDAlignedFloatSpan = vctr::Span<float, sizeof (a), vctr::SpanStorageInfo<float, true>>;

    SIMDAlignedFloatSpan aSpan (a, sizeof (a));
    @endcode
 */
template <class ElementType, bool isDataSIMDAligned, bool isStorageSIMDExtended = false>
using SpanStorageInfo = StaticStorageInfo<isDataSIMDAligned, isStorageSIMDExtended, alignof (std::span<ElementType>)>;

/** Creates a StaticStorageInfo instance suitable to be passed to a Span constructor.

    Declares the viewed data to be SIMD aligned. Required alignment is 32 byte on x64
    and 16 byte on ARM.
 */
template <class ElementType>
consteval auto simdAlignedSpanStorageInfo() { return SpanStorageInfo<ElementType, true>(); }

// clang-format off

/** The view type.

    Wraps a std::span and acts as a view to contiguous data owned and stored somewhere else.
    All containers that offer a size and data interface are implicitly convertible into a Span.
    Note that unlike Vector and Array, Spans have a const ElementType in case they view non-mutable data. This means,
    e.g. a const Vector<Foo> will be converted into a Span<const Foo>.

    A Span should always act as a view to a valid memory location, therefore it has no default constructor. The
    assignment operators of a Span never reassign the viewed memory location but copy elements from the source to
    the memory viewed by the Span instance.

    @ingroup Core
 */
template <class ElementType, size_t extent = std::dynamic_extent, class StorageInfoType = StorageInfoWithMemberAlignment<alignof (std::span<ElementType, extent>), StorageInfo<std::span<ElementType, extent> > > > // Extra whitespaces are needed for proper doxygen rendering
class Span : public VctrBase<ElementType, std::span<ElementType, extent>, extent, StorageInfoType>
{
// clang-format on
private:
    //==============================================================================
    using Vctr = VctrBase<ElementType, std::span<ElementType, extent>, extent, StorageInfoType>;
    using StdSpanType = std::span<ElementType, extent>;

public:
    //==============================================================================
    using value_type = typename Vctr::value_type;

    using element_type = ElementType;

    //==============================================================================
    // Constructors
    //==============================================================================

    /** Creates a Span with a given size that views externally managed data, accessed by ptr. */
    constexpr Span (ElementType* ptr, size_t size)
        : Vctr (StdSpanType (ptr, size))
    {
        VCTR_ASSERT (extent == size || extent == std::dynamic_extent);

        if (Vctr::getStorageInfo().dataIsSIMDAligned)
        {
            // If you hit this assertion, you passed a StaticStorageInfo that specifies SIMD aligned
            // memory, but it is not aligned. Required alignment is 32 byte on x64 and 16 byte on ARM.
            VCTR_ASSERT (isPtrAligned (ptr));
        }
    }

    /** Creates a Span with a given size that views externally managed data, accessed by ptr.
     
        The StaticStorageInfo instance describes extra SIMD alignment specifications of the
        viewed memory.
     */
    template <bool isDataSIMDAligned, bool isStorageSIMDExtended>
    constexpr Span (ElementType* ptr, size_t size, const StaticStorageInfo<isDataSIMDAligned, isStorageSIMDExtended, alignof (StdSpanType)>& info)
        : Vctr (StdSpanType (ptr, size), info)
    {
        VCTR_ASSERT (extent == size || extent == std::dynamic_extent);

        if constexpr (isDataSIMDAligned)
        {
            // If you hit this assertion, you passed a StaticStorageInfo that specifies SIMD aligned
            // memory, but it is not aligned. Required alignment is 32 byte on x64 and 16 byte on ARM.
            VCTR_ASSERT (isPtrAligned (ptr));
        }
    }

    /** Creates a Span that acts as a view to the given container.

        The container can be any type that supplies a matching data() and view() member function.
     */
    template <has::sizeAndDataWithElementType<ElementType> Container>
    constexpr Span (Container&& containerToView)
        : Vctr (StdSpanType (containerToView.data(), containerToView.size()), makeStorageInfo (containerToView))
    {
        VCTR_ASSERT (extent == containerToView.size() || extent == std::dynamic_extent);
    }

    /** Creates a Span that acts as a view to a compile time constant sized C style array. */
    template <std::same_as<ElementType> T, size_t n>
    requires (n != std::dynamic_extent)
    constexpr Span (T (&cStyleArray)[n])
    requires (n == extent)
        : Vctr (StdSpanType (cStyleArray))
    {
        if (Vctr::getStorageInfo().dataIsSIMDAligned)
        {
            // If you hit this assertion, you passed a StaticStorageInfo that specifies SIMD aligned
            // memory, but it is not aligned. Required alignment is 32 byte on x64 and 16 byte on ARM.
            VCTR_ASSERT (isPtrAligned (cStyleArray));
        }
    }

    //==============================================================================
    // Operators
    //==============================================================================
    // Deleting the default copy operator forces the usage of the intended operator below
    Span& operator= (const Span&) = delete;

    /** Copies or moves the data of the source container to this Span. You have to ensure that the source size matches. */
    template <has::sizeAndDataWithElementType<ElementType> Container>
    constexpr Span& operator= (Container&& containerToCopyDataFrom)
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

    /** Assigns elements from the initializer list to this Span. You have to ensure that the source size matches. */
    constexpr Span& operator= (std::initializer_list<ElementType> elementsToAssign)
    {
        Vctr::assign (std::move (elementsToAssign));
        return *this;
    }

    /** Assigns the result of an expression to this vector.

        The Span has to match the size of the expression.
     */
    template <is::expression E>
    constexpr void operator= (const E& expression)
    {
        VCTR_ASSERT (expression.size() == Vctr::size());

        Vctr::assignExpressionTemplate (expression);
    }

private:
    //==============================================================================
    template <has::sizeAndData Container>
    static constexpr auto makeStorageInfo (const Container& container)
    {
        constexpr size_t memberAlignment = alignof (StdSpanType);

        if constexpr (is::anyVctr<Container>)
        {
            const auto& info = container.getStorageInfo();
            return StorageInfoWithMemberAlignment<memberAlignment, std::remove_cvref_t<decltype (info)>> (info);
        }
        else
        {
            return StorageInfoWithMemberAlignment<memberAlignment, StorageInfo<Container>>().init (container.data(), container.size());
        }
    }
};

//==============================================================================
// Deduction guides
//==============================================================================

template <class ElementType, bool isDataSIMDAligned, bool isStorageSIMDExtended>
Span (ElementType*, size_t, const StaticStorageInfo<isDataSIMDAligned, isStorageSIMDExtended, alignof (std::span<ElementType>)>&) -> Span<ElementType, std::dynamic_extent, StaticStorageInfo<isDataSIMDAligned, isStorageSIMDExtended, alignof (std::span<ElementType>)>>;

template <class Container>
Span (Container&&) -> Span<DataType<Container>, extentOf<Container>, StorageInfoWithMemberAlignment<alignof (std::span<DataType<Container>, extentOf<Container>>), StorageInfoType<Container>>>;

template <class ElementType, size_t size>
Span (ElementType (&)[size]) -> Span<ElementType, size, StorageInfoWithMemberAlignment<alignof (std::span<ElementType, size>), StorageInfo<std::span<ElementType, size>>>>;

/** Creates a span with dynamic extent pointing to a memory location that is expected to be SIMD aligned. */
template <class ElementType>
auto makeSimdAlignedSpan (ElementType* data, size_t size)
{
    // If you hit this assertion, you passed a StaticStorageInfo that specifies SIMD aligned
    // memory, but it is not aligned. Required alignment is 32 byte on x64 and 16 byte on ARM.
    VCTR_ASSERT (isPtrAligned (data));

    return Span<ElementType, std::dynamic_extent, StaticStorageInfo<true, false, alignof (std::span<ElementType>)>> (data, size);
}

/** Creates a span with static extent pointing to a memory location that is expected to be SIMD aligned. */
template <size_t extent, class ElementType>
requires (extent != std::dynamic_extent)
auto makeSimdAlignedSpan (ElementType* data)
{
    // If you hit this assertion, you passed a StaticStorageInfo that specifies SIMD aligned
    // memory, but it is not aligned. Required alignment is 32 byte on x64 and 16 byte on ARM.
    VCTR_ASSERT (isPtrAligned (data));

    return  Span<ElementType, extent, StaticStorageInfo<true, false, alignof (std::span<ElementType>)>> (data, extent);
}

} // namespace vctr
