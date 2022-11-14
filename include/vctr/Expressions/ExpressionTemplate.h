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

} // namespace vctr

/** A helper macro to avoid repetitive boilerplate code when implementing an expression template class.

    It expects that the expression class has a suitable value_type defined before this, that the second
    class template type name is SrcType and that there is a private member SrcType src.

    It defines an Expression type, a constructor, a getStorageInfo(), a size() and an isNotAliased (const void*)
    member function according to the expression template conventions.
 */
// clang-format off
#define VCTR_COMMON_UNARY_EXPRESSION_MEMBERS(ExpressionName)                               \
using Expression = ExpressionTypes<value_type, SrcType>;                                   \
                                                                                           \
template <class Src>                                                                       \
constexpr ExpressionName (Src&& s) : src (std::forward<Src> (s)) {}                        \
                                                                                           \
constexpr const auto& getStorageInfo() const { return src.getStorageInfo(); }              \
                                                                                           \
constexpr size_t size () const { return src.size (); }                                     \
                                                                                           \
constexpr bool isNotAliased (const void* other) const { return src.isNotAliased (other); }
// clang-format on