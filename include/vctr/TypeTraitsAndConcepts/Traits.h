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

template <class T>
struct VctrBaseInspector
{
};

template <class E, class S, size_t e, class I>
struct VctrBaseInspector<VctrBase<E, S, e, I>>
{
    using ElementType = E;
    using StorageType = S;
};

template <class T>
struct ExpressionInspector
{
};

template <template <size_t, class...> class Expression, size_t e, class S>
struct ExpressionInspector<Expression<e, S>>
{
    using ReturnType = typename Expression<e, S>::value_type;
    using SourceType = S;

    static constexpr size_t numSources = 1;
    static constexpr size_t extent = e;
};

template <template <size_t, class...> class Expression, size_t e, class SA, class SB>
struct ExpressionInspector<Expression<e, SA, SB>>
{
    using ReturnType = typename Expression<e, SA, SB>::value_type;
    using SourceTypeA = SA;
    using SourceTypeB = SB;

    static constexpr size_t numSources = 2;
    static constexpr size_t extent = e;
};

template <template <size_t, class...> class Expression, size_t e, class... S>
requires (sizeof...(S) > 2)
struct ExpressionInspector<Expression<e, S...>>
{
    using ReturnType = typename Expression<e, S...>::value_type;

    static constexpr size_t numSources = sizeof...(S);
    static constexpr size_t extent = e;
};

template <class T>
struct ValueType
{
    using Type = typename T::value_type;
};

template <has::size StorageType>
struct Extent
{
    static constexpr size_t value = std::dynamic_extent;
};

template <class T, size_t extent>
struct Extent<std::array<T, extent>>
{
    static constexpr size_t value = extent;
};

template <class T, size_t extent>
struct Extent<std::span<T, extent>>
{
    static constexpr size_t value = extent;
};

template <class E, class S, size_t extent, class I>
struct Extent<VctrBase<E, S, extent, I>>
{
    static constexpr size_t value = extent;
};

template <class T, size_t extent, size_t s>
struct Extent<Array<T, extent, s>>
{
    static constexpr size_t value = extent;
};

template <class T, size_t extent, class S>
struct Extent<Span<T, extent, S>>
{
    static constexpr size_t value = extent;
};

template <is::expression T>
struct Extent<T>
{
    static constexpr size_t value = ExpressionInspector<T>::extent;
};

template <class T>
struct RealType
{
    using Type = T;
};

template <class T>
struct RealType<std::complex<T>>
{
    using Type = T;
};

template <class T>
struct FloatType
{};

template <std::integral T>
struct FloatType<T>
{
    using Type = std::conditional_t<sizeof (T) < 4, float, double>;
};

template <std::floating_point T>
struct FloatType<T>
{
    using Type = T;
};

template <class T>
struct StorageInfoType
{
    using Type = StorageInfo<T>;
};

template <is::anyVctr T>
struct StorageInfoType<T>
{
    using Type = std::remove_cvref_t<std::invoke_result_t<decltype (&T::getStorageInfo), const T>>;
};

template <class T>
struct RemovePointer
{
};

template <class T>
struct RemovePointer<T*>
{
    using Type = T;
};

template <class T>
struct RemovePointer<T* const>
{
    using Type = const T;
};

template <has::data T>
class DataType
{
private:
    // This function is only used to deduce a return type, so we can safely ignore warnings about returning a stack address
    VCTR_START_IGNORE_WARNING_CLANG_GCC (return-stack-address)
    static auto* invokeData (T t) { return t.data(); }
    VCTR_END_IGNORE_WARNING_CLANG_GCC

public:
    using Type = typename RemovePointer<std::invoke_result_t<decltype (&DataType::invokeData), T>>::Type;
};

} // namespace vctr::detail

namespace vctr
{

/** If T is an expression template, it equals its return type, if it's a type that defines
    value_type as a public typedef, it equals value_type.
 */
template <class T>
using ValueType = typename detail::ValueType<std::remove_cvref_t<T>>::Type;

/** The const correct element type derived from a call to T::data */
template <class T>
using DataType = typename detail::DataType<std::remove_reference_t<T>>::Type;

/** If T is any instance of std::complex, this will be the real value_type,
    otherwise this will be T.
 */
template <is::number T>
using RealType = typename detail::RealType<std::remove_cvref_t<T>>::Type;

/** The best matching float type for the real number type T.

    If T is an integer, the type will be float if T is < 32 Bit and double otherwise.
    If T is a floating point type, the type will be T.
 */
template <is::realNumber T>
using FloatType = typename detail::FloatType<std::remove_cvref_t<T>>::Type;



/** If t is a type derived from VctrBase, this will equal the return value of T::getStorageInfo,
    otherwise this will be StorageInfo<T>.
 */
template <has::sizeAndData T>
using StorageInfoType = typename detail::StorageInfoType<std::remove_cvref_t<T>>::Type;

/** Equals the extent of the container or expression type */
template <has::size T>
constexpr size_t extentOf = detail::Extent<std::remove_cvref_t<T>>::value;

/** Returns std::dynamic_extent in case both sources specify a dynamic extent.
    Throws in case both sources specify a non-matching non-dynamic extent.
    Returns the non-dynamic extent found otherwise.
 */
template <class A, class B>
consteval size_t getCommonExtent()
{
    if constexpr (extentOf<A> == std::dynamic_extent && extentOf<B> == std::dynamic_extent)
        return std::dynamic_extent;

    if constexpr (extentOf<A> != std::dynamic_extent && extentOf<B> != std::dynamic_extent && extentOf<A> != extentOf<B>)
        throw std::logic_error ("A and B both define different non-dynamic extents");

    return extentOf<A> != std::dynamic_extent ? extentOf<A> : extentOf<B>;
}

/** Ensures that both sources have the same size. In case they both specify a non-dynamic extent,
    this will be a compile-time static_assert, otherwise it will perform a runtime assert comparing the sizes.
 */
template <class A, class B>
constexpr void assertCommonSize ([[maybe_unused]] const A& a, [[maybe_unused]] const B& b)
{
    if constexpr (extentOf<A> == std::dynamic_extent || extentOf<B> == std::dynamic_extent)
    {
        VCTR_ASSERT (a.size() == b.size());
    }
    else
    {
        static_assert (extentOf<A> == extentOf<B>);
    }
}

/** A helper struct intended to check if a value is a constexpr.

    This is most useful to check if a member variable is compile time
    constant or the return value of a member function is compile time constant.
    Example:
    @code
    if constexpr (requires { typename RequireConstexpr<T::member>; })
    {
        constexpr auto m = T::member;
        // Treat it as a compile time constant
    }
    @endcode
 */
template <auto v>
struct RequireConstexpr {};

} // namespace vctr
