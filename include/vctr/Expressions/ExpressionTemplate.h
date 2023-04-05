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

/** The base class to every expression template.

    You should derive any expression template from this class. It supplies some handy
    typedefs and traits which are defined in the ExpressionTypes struct template. To
    use it, add a private typedef to your expression template class like

    @code
    using Expression = ExpressionTypes<ReturnElementType, SrcAType, SrcBType>;
    @endcode
 */
struct ExpressionTemplateBase : Config
{
    /** Supplies some handy typedefs and traits needed when implementing expression templates. */
    template <class ReturnElementType, class... SrcTypes>
    struct ExpressionTypes
    {
        /** Some common traits you want to check when constraining expression template functions */
        template <class T>
        struct ElementTraits
        {
            using Type = T;

            static constexpr auto isFloatingPoint = is::floatNumber<Type>;

            static constexpr auto isSigned = std::is_signed_v<Type>;

            static constexpr auto isUnsigned = std::is_unsigned_v<Type>;

            static constexpr auto isInt = std::is_integral_v<Type>;

            static constexpr auto isSignedInt = isSigned & isInt;

            static constexpr auto isComplex = is::complexNumber<Type>;

            static constexpr auto isComplexFloat = is::complexNumber<Type> && is::floatNumber<Type>;

            static constexpr auto isComplexInt = is::complexNumber<Type> && ! is::floatNumber<Type>;

            static constexpr auto isInt32 = std::same_as<int32_t, Type>;

            static constexpr auto isUint32 = std::same_as<uint32_t, Type>;

            static constexpr auto isInt64 = std::same_as<int64_t, Type>;

            static constexpr auto isUint64 = std::same_as<uint64_t, Type>;

            static constexpr auto isFloat = std::same_as<float, Type>;

            static constexpr auto isDouble = std::same_as<double, Type>;
        };

        /** The common type for all source element types along with its traits */
        using CommonSrcElement = ElementTraits<std::common_type_t<std::remove_const_t<ValueType<SrcTypes>>...>>;

        /** The common type for all source element types and the return type along with its traits */
        using CommonElement = ElementTraits<std::common_type_t<ReturnElementType, typename CommonSrcElement::Type>>;

        /** Indicates if all source element types and the return types are same */
        static constexpr auto allElementTypesSame = are::same<ReturnElementType, std::remove_const_t<ValueType<SrcTypes>>...>;

        //==============================================================================
        /** The NeonRegister type for the common element type */
        using Neon = NeonRegister<typename CommonElement::Type>;

        /** The NeonRegister type for the common source element type */
        using NeonSrc = NeonRegister<typename CommonSrcElement::Type>;

        /** The NeonRegister type for the common return element type */
        using NeonRet = NeonRegister<ReturnElementType>;

        //==============================================================================
        /** The AVXRegister type for the common element type */
        using AVX = AVXRegister<typename CommonElement::Type>;

        /** The AVXRegister type for the common source element type */
        using AVXSrc = AVXRegister<typename CommonSrcElement::Type>;

        /** The AVXRegister type for the common return element type */
        using AVXRet = AVXRegister<ReturnElementType>;

        //==============================================================================
        /** The SSERegister type for the common element type */
        using SSE = SSERegister<typename CommonElement::Type>;

        /** The SSERegister type for the common source element type */
        using SSESrc = SSERegister<typename CommonSrcElement::Type>;

        /** The SSERegister type for the common return element type */
        using SSERet = SSERegister<ReturnElementType>;

        //==============================================================================
        /** The PlatformVectorOps::AppleAccelerate type for the common element type */
        using Accelerate = PlatformVectorOps::AppleAccelerate<typename CommonElement::Type>;

        /** The PlatformVectorOps::IntelIPP type for the common element type */
        using IPP = PlatformVectorOps::IntelIPP<typename CommonElement::Type>;
    };
};

namespace detail
{

template <size_t i, class RuntimeArgs, is::expression Expression>
void tryApplyingRuntimeArgsToThisExpression (const RuntimeArgs& args, Expression& e)
{
    if constexpr (RuntimeArgs::template hasValue<i>())
    {
        std::apply ([&] (const auto&... args)
                    {
                        static_assert (has::applyRuntimeArgs<Expression, decltype (args)...>,
                                       "The expression does not implement a applyRuntimeArgs overload that matches the argument types passed.");
                        e.applyRuntimeArgs (args...);
                    }, args.template get<i>());
    }
}

template <size_t i, class RuntimeArgs, is::anyVctrOrExpression Src>
void tryApplyingRuntimeArgsToSrc (const RuntimeArgs& args, Src& src)
{
    if constexpr (has::iterateOverRuntimeArgChain<Src, i, RuntimeArgs>)
        src.template iterateOverRuntimeArgChain<i> (args);
}

} // namespace detail

} // namespace vctr

/** A helper macro to avoid repetitive boilerplate code when implementing a unary expression template class.

    It expects that the expression class has a suitable value_type defined before this, that the second
    class template type name is SrcType and that there is a private member SrcType src.

    It defines the member srcName of type SrcType, an Expression type, a constructor, a getStorageInfo(),
    a size(), an isNotAliased (const void*) and a iterateOverRuntimeArgChain (const RuntimeArgs&) member
    function according to the expression template conventions.

    Note that this macro contains a private and public sections and ends with the public access specifier.
    Therefore all code following this macro will be in the public section if not explicitly specified
    differently.
 */
// clang-format off
#define VCTR_COMMON_UNARY_EXPRESSION_MEMBERS(ExpressionName, srcName)                              \
private:                                                                                           \
    using Expression = ExpressionTypes<value_type, SrcType>;                                       \
    SrcType srcName;                                                                               \
public:                                                                                            \
                                                                                                   \
    template <class Src>                                                                           \
    constexpr ExpressionName (Src&& s) : srcName (std::forward<Src> (s)) {}                        \
                                                                                                   \
    constexpr const auto& getStorageInfo() const { return srcName.getStorageInfo(); }              \
                                                                                                   \
    constexpr size_t size () const { return srcName.size (); }                                     \
                                                                                                   \
    constexpr bool isNotAliased (const void* other) const { return srcName.isNotAliased (other); } \
                                                                                                   \
    template <size_t i, class RuntimeArgs>                                                         \
    void iterateOverRuntimeArgChain (const RuntimeArgs& rtArgs)                                    \
    {                                                                                              \
        tryApplyingRuntimeArgsToThisExpression<i> (rtArgs, *this);                                 \
        tryApplyingRuntimeArgsToSrc<i + 1> (rtArgs, srcName);                                      \
    }

/** A helper macro to avoid repetitive boilerplate code when implementing a binary expression template class
    that defines an expression between two vector-like sources.

    It expects that the expression class has a suitable value_type defined before this, that the second
    class template type name is SrcAType and the third class template type name is SrcBType.

    It defines the member srcAName of type SrcAType, srcBName of type SrcB and storageInfo of type
    CombinedStorageInfo<SrcAStorageInfoType, SrcBStorageInfoType>, an Expression type, a constructor,
    a getStorageInfo(), a size() and an isNotAliased (const void*)
    member function according to the expression template conventions.

    Note that this macro contains a private and public sections and ends with the public access specifier.
    Therefore all code following this macro will be in the public section if not explicitly specified
    differently.
 */
#define VCTR_COMMON_BINARY_VEC_VEC_EXPRESSION_MEMBERS(ExpressionName, srcAName, srcBName)                                      \
private:                                                                                                                       \
    using Expression = ExpressionTypes<value_type, SrcAType, SrcBType>;                                                        \
    using SrcAStorageInfoType = std::invoke_result_t<decltype (&std::remove_cvref_t<SrcAType>::getStorageInfo), SrcAType>;     \
    using SrcBStorageInfoType = std::invoke_result_t<decltype (&std::remove_cvref_t<SrcBType>::getStorageInfo), SrcBType>;     \
                                                                                                                               \
    SrcAType srcAName;                                                                                                         \
    SrcBType srcBName;                                                                                                         \
    const CombinedStorageInfo<std::remove_cvref_t<SrcAStorageInfoType>, std::remove_cvref_t<SrcBStorageInfoType>> storageInfo; \
                                                                                                                               \
public:                                                                                                                        \
    template <class SrcA, class SrcB>                                                                                          \
    constexpr ExpressionName (SrcA&& a, SrcB&& b)                                                                              \
      : srcAName (std::forward<SrcA> (a)),                                                                                     \
        srcBName (std::forward<SrcB> (b)),                                                                                     \
        storageInfo (srcAName.getStorageInfo(), srcBName.getStorageInfo())                                                     \
    {}                                                                                                                         \
                                                                                                                               \
    constexpr const auto& getStorageInfo() const { return storageInfo; }                                                       \
                                                                                                                               \
    constexpr size_t size() const { return srcAName.size(); }                                                                  \
                                                                                                                               \
    constexpr bool isNotAliased (const void* dst) const                                                                        \
    {                                                                                                                          \
        if constexpr (is::expression<SrcAType> && is::anyVctr<SrcBType>)                                                       \
        {                                                                                                                      \
            return dst != srcBName.data();                                                                                     \
        }                                                                                                                      \
                                                                                                                               \
        if constexpr (is::anyVctr<SrcAType> && is::expression<SrcBType>)                                                       \
        {                                                                                                                      \
            return dst != srcAName.data();                                                                                     \
        }                                                                                                                      \
                                                                                                                               \
        return true;                                                                                                           \
    }

/** A helper macro to avoid repetitive boilerplate code when implementing a binary expression template class
    that defines an expression between a single value and a vector

    It expects that the expression class has a suitable value_type defined before this and that the second
    class template type name is SrcType.

    It defines the member srcName of type SrcType, singleName of type Expression::CommonSrcElement::Type
    an Expression type, a constructor, a getStorageInfo(), a size() and an isNotAliased (const void*)
    member function according to the expression template conventions.

    Note that this macro contains a private and public sections and ends with the public access specifier.
    Therefore all code following this macro will be in the public section if not explicitly specified
    differently.
 */
#define VCTR_COMMON_BINARY_SINGLE_VEC_EXPRESSION_MEMBERS(ExpressionName, srcVecName, srcSingleName)   \
private:                                                                                              \
    using Expression = ExpressionTypes<value_type, SrcType>;                                          \
                                                                                                      \
    SrcType srcVecName;                                                                               \
    typename Expression::CommonSrcElement::Type srcSingleName;                                        \
    const typename Expression::SSESrc srcSingleName##AsSSE;                                           \
    const typename Expression::NeonSrc srcSingleName##AsNeon;                                         \
public:                                                                                               \
                                                                                                      \
    template <class Src>                                                                              \
    constexpr ExpressionName (typename Expression::CommonSrcElement::Type a, Src&& b)                 \
        : srcVecName (std::forward<Src> (b)),                                                         \
          srcSingleName (a),                                                                          \
          srcSingleName##AsSSE (Expression::SSESrc::broadcast (a)),                                   \
          srcSingleName##AsNeon (Expression::NeonSrc::broadcast (a))                                  \
    {}                                                                                                \
                                                                                                      \
    constexpr const auto& getStorageInfo() const { return srcVecName.getStorageInfo(); }              \
                                                                                                      \
    constexpr size_t size() const { return srcVecName.size(); }                                       \
    constexpr bool isNotAliased (const void* other) const { return srcVecName.isNotAliased (other); }
// clang-format on