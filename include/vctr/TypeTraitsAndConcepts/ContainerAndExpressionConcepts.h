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

// clang-format off
template <class T>
struct AnyVctr : std::false_type {};

template <class T, template <class> class A>
struct AnyVctr<Vector<T, A>> : std::true_type {};

template <class T, size_t e, size_t s>
struct AnyVctr<Array<T, e, s>> : std::true_type {};

template <class T, size_t n, class S>
struct AnyVctr<Span<T, n, S>> : std::true_type {};

template <class T>
struct IsVctr : std::false_type {};

template <class E, class S, size_t e, class I>
struct IsVctr<VctrBase<E, S, e, I>> : std::true_type {};

template <class T>
struct IsStdArray : std::false_type {};

template <class T, size_t e>
struct IsStdArray<std::array<T, e>> : std::true_type {};

template <class T>
struct IsStdSpan : std::false_type {};

template <class T, size_t e>
struct IsStdSpan<std::span<T, e>> : std::true_type {};

template <class T>
struct IsExpressionChainBuilder : std::false_type {};

template <template <size_t, class...> class ExpressionType, class... AdditionalParameters>
struct IsExpressionChainBuilder<ExpressionChainBuilder<ExpressionType, AdditionalParameters...>> : std::true_type {};
// clang-format on

} // namespace vctr::detail

namespace vctr::has
{

/** Constrains a type to have a member function getNeon (size_t) const */
template <class T>
concept getNeon = requires (const T& t, size_t i) { t.getNeon (i); };

/** Constrains a type to have a member function getAVX (size_t) const */
template <class T>
concept getAVX = requires (const T& t, size_t i) { t.getAVX (i); };

/** Constrains a type to have a member function getSSE (size_t) const */
template <class T>
concept getSSE = requires (const T& t, size_t i) { t.getSSE (i); };

/** Constrains a type to have a non const operator[] overload taking a size_t argument */
template <class T>
concept indexOperator = requires (T& t) { t[size_t()]; };

/** Constrains a type to have a const operator[] overload taking a size_t argument */
template <class T>
concept constIndexOperator = requires (const T& t) { t[size_t()]; };

/** Constrains a type to have a member function evalNextVectorOpInExpressionChain (value_type*) const */
template <class T, class DstType>
concept evalNextVectorOpInExpressionChain = requires (const T& t, DstType* d) { t.evalNextVectorOpInExpressionChain (d); } && std::same_as<DstType, typename std::remove_cvref_t<T>::value_type>;

/** Constrains a type to have a member function data() const */
template <class T>
concept data = requires (const T& t) { t.data(); };

/** Constrains a type to have a member function data() const returning a pointer convertible to const ElementType* */
template <class T, class ElementType>
concept dataWithElementType = requires (const T& t, const ElementType* s) { s = t.data(); };

/** Constrains a type to have a member function size() const */
template <class T>
concept size = requires (const T& t, size_t i) { i = t.size(); };

/** Constrains a type to both, a member function size() const and data() const */
template <class T>
concept sizeAndData = size<T> && data<T>;

/** Constrains a type to both, a member function size() const and data() const returning a pointer convertible to ElementType* */
template <class T, class ElementType>
concept sizeAndDataWithElementType = size<T> && dataWithElementType<T, ElementType>;

/** Constrains a type to have a member function begin() or begin() const */
template <class T>
concept begin = requires (T& t) { *t.begin(); } || requires (const T& t) { *t.begin(); };

/** Constrains a type to have a member function end() or end() const */
template <class T>
concept end = requires (T& t) { *t.end(); } || requires (const T& t) { *t.begin(); };

/** Constrains a type to have a function resize (size_t) */
template <class T>
concept resize = requires (T& t, size_t n) { t.resize (n); };

template <class T>
concept getStorageInfo = requires (const T& t) { t.getStorageInfo(); };

template <class T>
concept isNotAliased = requires (const T& t, bool r, const void* o) { r = t.isNotAliased (o); };

} // namespace vctr::has

namespace vctr::detail
{
/** Special flags for platform vector op related concepts below.

        If IPP is available on macOS, we might have situations where both, IPP and Apple Accelerate
        can offer similar operations for a certain operation. In that case the flag marks if we want
        to prefer the one or the other
     */
enum PlatformVectorOpPreference
{
    dontPreferIfIppAndAccelerateAreAvailable,
    preferIfIppAndAccelerateAreAvailable
};

template <PlatformVectorOpPreference pref>
concept isPreferredVectorOp = (Config::hasIPP && Config::platformApple && (pref == preferIfIppAndAccelerateAreAvailable)) ||
                              ((! (Config::hasIPP && Config::platformApple)) && (pref == dontPreferIfIppAndAccelerateAreAvailable));
} // namespace vctr::detail

namespace vctr::is
{

// clang-format off

/** Constrains a type to be any derived instance of VctrBase */
template <class T>
concept anyVctr = detail::AnyVctr<std::remove_cvref_t<T>>::value;

/** Constrains a type to be an expression template */
template <class T>
concept expression = has::getStorageInfo<T> && has::size<T> && has::constIndexOperator<T> && has::isNotAliased<T> && ! anyVctr<T>;

/** Constrains a type to be an expression chain builder */
template <class T>
concept expressionChainBuilder = detail::IsExpressionChainBuilder<std::remove_cvref_t<T>>::value;

/** Constrains a type to either be an expression template or any derived instance of VctrBase */
template <class T>
concept anyVctrOrExpression = expression<std::remove_cvref_t<T>> || anyVctr<std::remove_cvref_t<T>>;

/** Constrains a type to be any instance of std::array */
template <class T>
concept stdArray = detail::IsStdArray<T>::value;

/** Constrains a type to be any instance of std::span */
template <class T>
concept stdSpan = detail::IsStdSpan<T>::value;

//==============================================================================
/** Constrains a type to be an expression template that defines evalNextVectorOpInExpressionChain for DstType */
template <class T, class DstType>
concept expressionWithEvalVectorOp = expression<T> && has::evalNextVectorOpInExpressionChain<T, DstType>;

//==============================================================================
/** A combined concept to check if Apple Accelerate is a suitable option for a real valued floating point vector operation */
template <class Src, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForAccelerateRealFloatVectorOp = detail::isPreferredVectorOp<pref> && Config::platformApple && has::evalNextVectorOpInExpressionChain<Src, DstType> && floatNumber<DstType>;

/** A combined concept to check if Apple Accelerate is a suitable option for a complex valued floating point vector operation */
template <class Src, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForAccelerateComplexFloatVectorOp = detail::isPreferredVectorOp<pref> && Config::platformApple && has::evalNextVectorOpInExpressionChain<Src, DstType> && complexFloatNumber<DstType>;

/** A combined concept to check if Apple Accelerate is a suitable option for a real or complex valued floating point vector operation */
template <class Src, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForAccelerateRealOrComplexFloatVectorOp = detail::isPreferredVectorOp<pref> && Config::platformApple && has::evalNextVectorOpInExpressionChain<Src, DstType> && (floatNumber<DstType> || complexFloatNumber<DstType>);

/** A combined concept to check if Apple Accelerate is a suitable option for a floating point vector operation that transforms a complex vector into a real one */
template <class Src, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForAccelerateComplexToRealFloatVectorOp = detail::isPreferredVectorOp<pref> && Config::platformApple && anyVctr<Src> && complexFloatNumber<typename Src::value_type> && floatNumber<DstType>;

/** A combined concept to check if Apple Accelerate is a suitable option for a vector operation that transforms an integer vector into a floating point one */
template <class Src, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForAccelerateRealIntToFloatVectorOp = detail::isPreferredVectorOp<pref> && Config::platformApple && has::evalNextVectorOpInExpressionChain<Src, typename std::remove_cvref_t<Src>::value_type> && floatNumber<DstType> && intNumber<typename std::remove_cvref_t<Src>::value_type>;

//==============================================================================
/** A combined concept to check if Intel IPP is a suitable option for a real valued floating point vector operation */
template <class Src, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForIppRealFloatVectorOp = detail::isPreferredVectorOp<pref> && Config::hasIPP && has::evalNextVectorOpInExpressionChain<Src, DstType> && floatNumber<DstType>;

/** A combined concept to check if Intel IPP is a suitable option for a real valued singed int32 vector operation */
template <class Src, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForIppRealSingedInt32VectorOp = detail::isPreferredVectorOp<pref> && Config::hasIPP && has::evalNextVectorOpInExpressionChain<Src, DstType> && std::same_as<int32_t, DstType>;

/** A combined concept to check if Intel IPP is a suitable option for a complex valued floating point vector operation */
template <class Src, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForIppComplexFloatVectorOp = detail::isPreferredVectorOp<pref> && Config::hasIPP && has::evalNextVectorOpInExpressionChain<Src, DstType> && complexFloatNumber<DstType>;

/** A combined concept to check if Intel IPP is a suitable option for a real or complex valued floating point vector operation */
template <class Src, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForIppRealOrComplexFloatVectorOp = detail::isPreferredVectorOp<pref> && Config::hasIPP && has::evalNextVectorOpInExpressionChain<Src, DstType> && (floatNumber<DstType> || complexFloatNumber<DstType>);

/** A combined concept to check if Intel IPP is a suitable option for a floating point vector operation that transforms a complex vector into a real one */
template <class Src, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForIppComplexToRealFloatVectorOp = detail::isPreferredVectorOp<pref> && Config::hasIPP && anyVctr<Src> && complexFloatNumber<typename std::remove_cvref_t<Src>::value_type> && floatNumber<DstType>;

//==============================================================================
/** Constrains two source types to be suitable for a an aliasing-free binary vector operation using platform vector ops */
template <class SrcA, class SrcB, class DstType>
concept suitableForBinaryVectorOp = (expressionWithEvalVectorOp < SrcA, DstType > && anyVctr < SrcB >) ||
                                    (anyVctr<SrcA> && expressionWithEvalVectorOp < SrcB, DstType >) ||
                                    (anyVctr<SrcA> && anyVctr<SrcB>);

//==============================================================================
/** A combined concept to check if Apple Accelerate is a suitable option for a real valued floating point binary vector operation */
template <class SrcA, class SrcB, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForAccelerateRealFloatBinaryVectorOp = detail::isPreferredVectorOp<pref> && Config::platformApple && suitableForBinaryVectorOp<SrcA, SrcB, DstType> && floatNumber<DstType>;

/** A combined concept to check if Apple Accelerate is a suitable option for a complex valued floating point binary vector operation */
template <class SrcA, class SrcB, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForAccelerateComplexFloatBinaryVectorOp = detail::isPreferredVectorOp<pref> && Config::platformApple && suitableForBinaryVectorOp<SrcA, SrcB, DstType> && complexFloatNumber<DstType>;

/** A combined concept to check if Apple Accelerate is a suitable option for a real or complex valued floating point binary vector operation */
template <class SrcA, class SrcB, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForAccelerateRealOrComplexComplexFloatBinaryVectorOp = detail::isPreferredVectorOp<pref> && Config::platformApple && suitableForBinaryVectorOp<SrcA, SrcB, DstType> && (floatNumber<DstType> || complexFloatNumber<DstType>);

//==============================================================================
/** A combined concept to check if Intel IPP is a suitable option for a real valued floating point binary vector operation */
template <class SrcA, class SrcB, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForIppRealFloatBinaryVectorOp = detail::isPreferredVectorOp<pref> && Config::hasIPP && suitableForBinaryVectorOp<SrcA, SrcB, DstType> && floatNumber<DstType>;

/** A combined concept to check if Intel IPP is a suitable option for a complex valued floating point binary vector operation */
template <class SrcA, class SrcB, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForIppComplexFloatBinaryVectorOp = detail::isPreferredVectorOp<pref> && Config::hasIPP && suitableForBinaryVectorOp<SrcA, SrcB, DstType> && complexFloatNumber<DstType>;

/** A combined concept to check if Intel IPP is a suitable option for a real or complex valued floating point binary vector operation */
template <class SrcA, class SrcB, class DstType, detail::PlatformVectorOpPreference pref = detail::preferIfIppAndAccelerateAreAvailable>
concept suitableForIppRealOrComplexComplexFloatBinaryVectorOp = detail::isPreferredVectorOp<pref> && Config::hasIPP && suitableForBinaryVectorOp<SrcA, SrcB, DstType> && (floatNumber<DstType> || complexFloatNumber<DstType>);

//==============================================================================
/** Constrains a type to supply a data and size function, an index operator and define a trivially copyable value_type */
template <class T>
concept triviallyCopyableWithDataAndSize = has::sizeAndDataWithElementType<T, typename T::value_type> && has::constIndexOperator<T> && triviallyCopyable<typename T::value_type>;

/** Constrains a type to supply begin and end functions and to not satisfy triviallyCopyableWithDataAndSize */
template <class T>
concept iteratorCopyable = has::begin<T> && has::end<T> && ! triviallyCopyableWithDataAndSize<T>;

/** Constrains the type to be suitable for initializing a single element Vctr with a given ElementType, that is, it is
    the same type as the ElementType with const and reference version of the ElementType being allowed.
 */
template <class T, class ElementType>
concept suitableInitializerForElementType = std::same_as<ElementType, std::remove_cvref_t<T>> && ! expression<T>;

/** Constrains the type to be a function suitable for initializing the nth element of a Vctr, that is,
    it returns an element for a given index passed as size_t argument.
 */
template <class T>
concept suitableInitializerFunction = std::invocable<T, size_t> && ! std::same_as<void, std::invoke_result_t<T, size_t>>;

/** Constrains the type to be a function suitable for initializing the nth element of a Vctr with a given ElementType,
    that is, it returns an element for a given index passed as size_t argument.
 */
template <class T, class ElementType>
concept suitableInitializerFunctionForElementType = suitableInitializerFunction<T> && std::constructible_from<ElementType, std::invoke_result_t<T, size_t>>;

/** Constrains the type to be an input iterator with a value type suitable to construct an ElementType. */
template <class T, class ElementType>
concept suitableInputIteratorForElementType = std::input_iterator<T> && std::constructible_from<ElementType, std::iter_value_t<T>>;

/** Constrains the type to be suitable for initializing a single element Vctr, that is, it is not of any other type
    for which special single argument constructors exist, so no ambiguity can arise.
 */
template <class T>
concept suitableSingleElementInitializer = ! anyVctrOrExpression<T> && ! triviallyCopyableWithDataAndSize<T> && ! iteratorCopyable<T> && ! suitableInitializerFunction<T>;

/** Constrains the type to be a suitable storage info type.

    @see StorageInfo
 */
template <class T>
concept storageInfo = requires (const T& info, size_t a, bool b) { a = T::memberAlignment; b = info.dataIsSIMDAligned; b = info.hasSIMDExtendedStorage; };

// clang-format on

} // namespace vctr::is

namespace vctr::are
{

/** Constrains a pack of types to be no expression templates */
template <class... T>
concept noExpressionTemplates = (! is::expression<T> && ...);

} // namespace vctr::are
