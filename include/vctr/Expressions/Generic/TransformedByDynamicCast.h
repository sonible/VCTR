/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2025 by sonible GmbH.

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

namespace vctr::expressions
{

template <class T>
concept isRefOrPtr = is::lvalueReference<T> || is::pointer<T>;

template <size_t extent, class SrcType, isRefOrPtr DstValueType, is::constant ExpectCastToSucceed>
class TransformedByDynamicCast : public ExpressionTemplateBase
{
public:
    using value_type = DstValueType;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (TransformedByDynamicCast, src)

    VCTR_FORCEDINLINE constexpr DstValueType operator[] (size_t i) const
    {
        DstValueType casted = dynamic_cast<DstValueType> (src[i]);

        if constexpr (is::pointer<ValueType<SrcType>> && ExpectCastToSucceed::value)
            VCTR_ASSERT (casted != nullptr);

        return casted;
    }
};

} // namespace vctr::expressions

namespace vctr
{
/** Transforms all source elements to DstValueType by applying a dynamic_cast<DstValueType> to them.

    If expectCastToSucceed is true, then a VCTR_ASSERT will be inserted to ensure the dynamic cast
    did not result in nullptr when casting pointers.

    @ingroup Expressions
 */
template <class DstType, bool expectCastToSucceed = true>
constexpr inline ExpressionChainBuilder<expressions::TransformedByDynamicCast, DstType, Constant<expectCastToSucceed>> transformedByDynamicCastTo;

} // namespace vctr

