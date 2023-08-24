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

namespace vctr::detail
{
template <size_t value>
requires (is::powerOfTwoInt<value>)
constexpr size_t previousMultipleOf (size_t numElements)
{
    constexpr auto numMaskBits = std::bit_width (value) - 1;
    constexpr auto allBitsOne = size_t (-1);
    constexpr auto mask = allBitsOne << numMaskBits;

    return numElements & mask;
}

template <size_t value>
requires (is::powerOfTwoInt<value>)
constexpr size_t nextMultipleOf (size_t numElements)
{
    return size_t ((int64_t (numElements) + int64_t (value) - 1) & -int64_t (value));
}

template <uintptr_t requiredAlignment = Config::maxSIMDRegisterSize>
inline constexpr bool isPtrAligned (const void* ptr)
{
    // SIMD alignment is not meaningful in case of constant evaluation
    if (std::is_constant_evaluated())
        return false;

    return reinterpret_cast<std::uintptr_t> (ptr) % requiredAlignment == 0;
}
} // namespace vctr::detail

namespace vctr
{

/** A helper class to describe some properties regarding the storage class wrapped in a VctrBase instance.

    VctrBase expects a suitable storage info to supply three member variables
    - memberAlignment:        Specifies the alignment requirement of the storage member of the VctrBase instance. The
                              default case is simply alignof (StorageType). Can be used to make memory aligned when the
                              StorageType wraps a stack memory buffer, such as std::array. Must be static constexpr.
    - dataIsSIMDAligned:      Specifies if the data is aligned for AVX access (x64) or Neon access (ARM).
    - hasSIMDExtendedStorage: Specifies if the actually allocated memory is extended with unused dummy values at the end
                              in a way that it is a multiple of the size of an AVX register (x64) or Neon register (ARM).

    There are template specialisations for std::vector with an AlignedAllocator and std::array, which define all
    variable as static constexpr in order to make this struct as lightweight as possible in most cases.

 */

template <class StorageType>
struct StorageInfo
{
    // The default implementation is empty and should never be used
};

template <has::sizeAndData StorageType>
struct StorageInfo<StorageType>
{
    constexpr StorageInfo()
        : dataIsSIMDAligned (false),
          hasSIMDExtendedStorage (false)
    {}

    template <is::storageInfo OtherStorageInfoType>
    constexpr StorageInfo (const OtherStorageInfoType& other)
        : dataIsSIMDAligned (other.dataIsSIMDAligned),
          hasSIMDExtendedStorage (other.hasSIMDExtendedStorage)
    {}

    template <class T>
    constexpr StorageInfo init (const T* ptr, size_t s)
    {
        dataIsSIMDAligned = detail::isPtrAligned (ptr);
        hasSIMDExtendedStorage = (s * sizeof (T)) % Config::maxSIMDRegisterSize == 0;
        return *this;
    }

    static constexpr size_t memberAlignment = alignof (StorageType);

    bool dataIsSIMDAligned;

    bool hasSIMDExtendedStorage;
};

template <class ElementType, size_t alignmentInBytes>
struct StorageInfo<std::vector<ElementType, AlignedAllocator<ElementType, alignmentInBytes>>>
{
    constexpr StorageInfo init (const void*, size_t) { return *this; }

    static constexpr size_t memberAlignment = alignof (std::vector<ElementType, AlignedAllocator<ElementType, alignmentInBytes>>);

    static constexpr bool dataIsSIMDAligned = alignmentInBytes == Config::maxSIMDRegisterSize;

    /** This is ensured by the AlignedAllocator */
    static constexpr bool hasSIMDExtendedStorage = true;
};

template <class ElementType, size_t size>
struct StorageInfo<std::array<ElementType, size>>
{
    constexpr StorageInfo init (const void*, size_t) { return *this; }

    static constexpr size_t memberAlignment = Config::alignedArray ? Config::maxSIMDRegisterSize : alignof (std::array<ElementType, size>);

    /** This might be ensured at build time by an enforced member memberAlignment depending on the VCTR_ALIGNED_ARRAY config */
    static constexpr bool dataIsSIMDAligned = Config::alignedArray;

    static constexpr bool hasSIMDExtendedStorage = (size * sizeof (ElementType)) % Config::maxSIMDRegisterSize == 0;
};

template <class Allocator>
struct StorageInfo<detail::VectorBoolWorkaround<Allocator>>
{
    constexpr StorageInfo init (const void*, size_t) { return *this; }

    static constexpr size_t memberAlignment = alignof (detail::VectorBoolWorkaround<Allocator>);

    static constexpr bool dataIsSIMDAligned = false;

    static constexpr bool hasSIMDExtendedStorage = false;
};

/** A storage info type especially used to pass compile time constant traits when viewing externally owned memory via a
    Span in case we know that the viewed memory will fulfil certain traits.
 */
template <bool isDataSIMDAligned, bool isStorageSIMDExtended, size_t customMemberAlignment>
struct StaticStorageInfo
{
    static constexpr bool dataIsSIMDAligned = isDataSIMDAligned;
    static constexpr bool hasSIMDExtendedStorage = isStorageSIMDExtended;
    static constexpr size_t memberAlignment = customMemberAlignment;
};

/** A storage info type especially used for vctr::Span.

    A vctr::Span is a vctr::VctrBase wrapping a std::span. Therefore we should not take the class member alignment
    of the viewed container as the alignment for the std::span but should reflect all other storage information
    of the original source.
 */
template <size_t alignment, class WrappedInfo>
struct StorageInfoWithMemberAlignment : WrappedInfo
{
    constexpr StorageInfoWithMemberAlignment() = default;

    constexpr StorageInfoWithMemberAlignment (const WrappedInfo& i) : WrappedInfo (i) {}

    static constexpr size_t memberAlignment = alignment;
};

namespace detail
{

template <bool, bool>
struct ConstexprStorageInfoChecker
{
};
} // namespace detail

namespace is
{
template <class T>
concept constexprStorageInfo = requires (const T&) { detail::ConstexprStorageInfoChecker<T::dataIsSIMDAligned, T::hasSIMDExtendedStorage>(); };
}

template <class InfoA, class InfoB>
struct CombinedStorageInfo
{
    CombinedStorageInfo (const InfoA& a, const InfoB& b)
        : dataIsSIMDAligned (a.dataIsSIMDAligned && b.dataIsSIMDAligned),
          hasSIMDExtendedStorage (a.hasSIMDExtendedStorage && b.hasSIMDExtendedStorage)
    {}

    bool dataIsSIMDAligned;

    bool hasSIMDExtendedStorage;
};

template <is::constexprStorageInfo InfoA, is::constexprStorageInfo InfoB>
struct CombinedStorageInfo<InfoA, InfoB>
{
    constexpr CombinedStorageInfo (const InfoA&, const InfoB&) {}

    static constexpr bool dataIsSIMDAligned = InfoA::dataIsSIMDAligned && InfoB::dataIsSIMDAligned;

    static constexpr bool hasSIMDExtendedStorage = InfoA::hasSIMDExtendedStorage && InfoB::hasSIMDExtendedStorage;
};

} // namespace vctr
