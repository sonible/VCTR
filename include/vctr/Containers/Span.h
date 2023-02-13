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

/** Creates a StaticStorageInfo instance suitable to be passed to a Span constructor.

    Declares the viewed data to be SIMD aligned. Required alignment is 32 byte on x64
    and 16 byte on ARM.
 */
template <class ElementType>
consteval auto simdAlignedSpanStorageInfo() { return StaticStorageInfo<true, false, alignof (std::span<ElementType>)>(); }

// clang-format off

/** The view type.

    Wraps a std::span and acts as a view to contiguous data owned and stored somewhere else.
    All containers that offer a size and data interface are implicitly convertible into a Span.
    Note that unlike Vector and Array, Spans have a const ElementType in case they view non-mutable data. This means,
    e.g. a const Vector<Foo> will be converted into a Span<const Foo>.

    @ingroup Core
 */
template <class ElementType, size_t extent = std::dynamic_extent, class StorageInfoType = StorageInfo<std::span<ElementType, extent> > > // Extra whitespaces are needed for proper doxygen rendering
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

    /** Creates an empty Span with size = 0 and data = nullptr. */
    constexpr Span()
    requires (extent == 0 || extent == std::dynamic_extent)
    = default;

    /** Creates a Span with a given size that views externally managed data, accessed by ptr. */
    constexpr Span (ElementType* ptr, size_t size)
        : Vctr (StdSpanType (ptr, size))
    {
        VCTR_ASSERT (extent == size || extent == std::dynamic_extent);
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
            VCTR_ASSERT (detail::isPtrAligned (ptr));
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

    /** Copies the data of the source container to the memory this span refers to. */
    template <has::sizeAndDataWithElementType<ElementType> Container>
    constexpr Span& operator= (const Container& containerToCopyDataFrom)
    {
        Vctr::copyFrom (containerToCopyDataFrom.data(), containerToCopyDataFrom.size());
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
        if constexpr (is::anyVctr<Container>)
        {
            return container.getStorageInfo();
        }
        else
        {
            return StorageInfo<Container>().init (container.data(), container.size());
        }
    }
};

//==============================================================================
// Deduction guides
//==============================================================================

template <class ElementType, bool isDataSIMDAligned, bool isStorageSIMDExtended>
Span (ElementType*, size_t, const StaticStorageInfo<isDataSIMDAligned, isStorageSIMDExtended, alignof (std::span<ElementType>)>&) -> Span<ElementType, std::dynamic_extent, StaticStorageInfo<isDataSIMDAligned, isStorageSIMDExtended, alignof (std::span<ElementType>)>>;

template <class Container>
Span (Container&&) -> Span<DataType<Container>, extentOf<Container>, StorageInfoType<Container>>;
} // namespace vctr
