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
/** Returns aligned pointers when allocations are requested. */
template <typename ElementType, size_t alignmentInBytes>
requires (alignmentInBytes >= alignof (ElementType) && is::powerOfTwoInt<alignmentInBytes>)
class AlignedAllocator
{
public:
    using value_type = ElementType;
    using size_type = std::size_t;

    AlignedAllocator() noexcept = default;

    template <class OtherElementType>
    AlignedAllocator (const AlignedAllocator<OtherElementType, alignmentInBytes>&) noexcept {}

    [[nodiscard]] ElementType* allocate (size_t nElementsToAllocate)
    {
        if (nElementsToAllocate > std::numeric_limits<size_t>::max() / sizeof (ElementType))
        {
            throw std::bad_array_new_length();
        }

        const auto nBytesToAllocate = detail::nextMultipleOf<Config::maxSIMDRegisterSize> (nElementsToAllocate * sizeof (ElementType));

// Aligned operator new is only available on macOS 10.14 or higher. Therefore, we use posix_memalign on non-Windows
// platforms which is widely available on unix platforms to perform aligned allocation if not on Windows.
#if VCTR_WINDOWS
        return reinterpret_cast<ElementType*> (::operator new[] (nBytesToAllocate, std::align_val_t (alignmentInBytes)));
#else
        void* ptr;
        auto status = posix_memalign (&ptr, alignmentInBytes, nBytesToAllocate);
        if (status != 0)
            throw std::bad_alloc();
        return reinterpret_cast<ElementType*> (ptr);
#endif
    }

    void deallocate (ElementType* allocatedPointer, [[maybe_unused]] size_t nBytesAllocated)
    {
#if VCTR_WINDOWS
        ::operator delete[] (allocatedPointer, std::align_val_t (alignmentInBytes));
#else
        free (allocatedPointer);
#endif
    }

    template <class T>
    struct rebind
    {
        using other = AlignedAllocator<T, alignmentInBytes>;
    };
};

template <class ElementType, size_t alignmentInBytesLHS, size_t alignmentInBytesRHS>
constexpr bool operator== (const AlignedAllocator<ElementType, alignmentInBytesLHS>&, const AlignedAllocator<ElementType, alignmentInBytesRHS>&)
{
    return alignmentInBytesLHS == alignmentInBytesRHS;
}

} // namespace vctr
