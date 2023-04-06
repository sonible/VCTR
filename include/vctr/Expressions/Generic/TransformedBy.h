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

namespace vctr::Expressions
{

template <size_t extent, class SrcType, std::invocable<ValueType<SrcType>> FunctionType>
requires (! std::is_void_v<std::invoke_result<FunctionType, ValueType<SrcType>>>)
class TransformedBy : public ExpressionTemplateBase
{
public:
    using value_type = std::invoke_result_t<FunctionType, ValueType<SrcType>>;

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (TransformedBy, src)

    void applyRuntimeArgs (const FunctionType& transformationFunction)
    {
        fn = transformationFunction;
    }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return fn (src[i]);
    }

private:
    using FnCopyType = std::conditional_t<std::copyable<FunctionType>, FunctionType, std::function<value_type (ValueType<SrcType>)>>;
    FnCopyType fn;
};

} // namespace vctr::Expressions

namespace vctr
{
/** Transforms all source elements by applying fn to them.

    Fn can either be a function, a std::function or an object with a call operator. It must return
    a non-void value which will then be assigned to the destination of the expression - so the return
    type of Fn will become the value type of the expression.

    Note: This expression will create a copy of the function object. Depending on the type of Fn, this expression
    might or might not allocate heap memory. For all trivially copyable types such as simple function pointers or
    lambdas without any capture, you can expect this expression to not allocate any heap memory. For other types of
    functions you should expect a copy involving heap memory allocation.

    @ingroup Expressions
 */
template <class Fn>
constexpr auto transformedBy (Fn&& fn)
{
    return makeTemplateExpressionChainBuilderWithRuntimeArgs<Expressions::TransformedBy, std::remove_cvref_t<Fn>> (std::forward<Fn> (fn));
}

} // namespace vctr

