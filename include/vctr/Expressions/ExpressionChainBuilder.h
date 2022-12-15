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

namespace detail
{
template <class SrcChainer, template <size_t, class> class ExpressionToAdd>
struct ExpressionChainingHelper
{
    template <size_t extent, class SrcType>
    using NewExpressionChain = ExpressionToAdd<extent, typename SrcChainer::template Expression<extent, SrcType>>;

    using NewExpressionChainBuilder = ExpressionChainBuilder<NewExpressionChain>;
};
} // namespace detail

/** A simple helper struct to pass a constant as argument wrapped in a struct with a single public
    static constexpr member variable named value.
 */
template <auto constantValue>
struct Constant
{
    static constexpr auto value = constantValue;
};

/** An ExpressionChainBuilder is an object which supplies various operator<< overloads which build chains of
    Expression Templates by prepending the templated ExpressionType to the source.
 */
template <template <size_t, class...> class ExpressionType, class... ExtraParameters>
struct ExpressionChainBuilder
{
    template <size_t extent, class SrcType>
    using Expression = ExpressionType<extent, SrcType, ExtraParameters...>;

    /** Returns an expression which holds a reference to the Vector passed in as source */
    template <is::anyVctr Src>
    constexpr auto operator<< (const Src& src) const
    {
        auto expression = Expression<extentOf<Src>, const Src&> { src };

        if constexpr (is::reductionExpression<decltype (expression)>)
        {
            return ReductionExpression::reduce (expression);
        }
        else
        {
            return expression;
        }
    }

    /** Returns an expression which holds a reference to the Vector passed in as source */
    template <is::anyVctr Src>
    constexpr auto operator<< (Src& src) const
    {
        auto expression = Expression<extentOf<Src>, const Src&> { src };

        if constexpr (is::reductionExpression<decltype (expression)>)
        {
            return ReductionExpression::reduce (expression);
        }
        else
        {
            return expression;
        }
    }

    /** Returns an expression which owns a the Vector passed in as source */
    template <is::anyVctr Src>
    constexpr auto operator<< (Src&& src) const
    {
        auto expression = Expression<extentOf<Src>, Src> { std::move (src) };

        if constexpr (is::reductionExpression<decltype (expression)>)
        {
            return ReductionExpression::reduce (expression);
        }
        else
        {
            return expression;
        }
    }

    /** Returns an expression which owns another Expression passed in as source */
    template <is::expression SrcExpression>
    constexpr auto operator<< (SrcExpression&& e) const
    {
        auto expression = Expression<extentOf<SrcExpression>, SrcExpression> (std::forward<SrcExpression> (e));

        if constexpr (is::reductionExpression<decltype (expression)>)
        {
            return ReductionExpression::reduce (expression);
        }
        else
        {
            return expression;
        }
    }

    /** Returns an ExpressionChainBuilder instance which is the combination of the builder instance passed in,
        prepended by the expression represented by this builder.
     */
    template <is::expressionChainBuilder SrcExpressionChainBuilder>
    constexpr auto operator<< (SrcExpressionChainBuilder) const
    {
        using ChainingHelper = detail::ExpressionChainingHelper<SrcExpressionChainBuilder, Expression>;

        return typename ChainingHelper::NewExpressionChainBuilder();
    }
};
} // namespace vctr
