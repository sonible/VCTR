/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2023 by sonible GmbH.

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

template <size_t extent, class SrcType, std::constructible_from<ValueType<SrcType>> DstValueType>
class TransformedByStaticCast : public ExpressionTemplateBase
{
public:
    using value_type = DstValueType;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (TransformedByStaticCast, src)

    VCTR_FORCEDINLINE constexpr DstValueType operator[] (size_t i) const
    {
        return static_cast<DstValueType> (src[i]);
    }

    VCTR_FORCEDINLINE const DstValueType* evalNextVectorOpInExpressionChain (DstValueType* dst) const
    requires (sizeof (DstValueType) == sizeof (ValueType<SrcType>) || is::anyVctr<SrcType>) &&
             has::evalNextVectorOpInExpressionChain<SrcType, ValueType<SrcType>> &&
             std::is_trivially_destructible_v<ValueType<SrcType>> &&
             std::is_trivially_constructible_v<DstValueType>
    {
        // Since the constraints verified that we can use a different typed destination memory to
        // evaluate an intermediate chain result, we cast the destination memory pointer to a pointer
        // of type SrcType::value_type*
        const auto* s = src.evalNextVectorOpInExpressionChain (static_cast<ValueType<SrcType>*> (static_cast<void*> (dst)));

        const auto n = src.size();
        for (size_t i = 0; i < n; ++i)
            dst[i] = static_cast<DstValueType> (s[i]);

        return dst;
    }
};

} // namespace vctr::expressions

namespace vctr
{
/** Transforms all source elements to DstValueType by applying a static_cast<DstValueType> to them.

    @ingroup Expressions
 */
template <class DstType>
constexpr inline ExpressionChainBuilder<expressions::TransformedByStaticCast, DstType> transformedByStaticCastTo;

} // namespace vctr

