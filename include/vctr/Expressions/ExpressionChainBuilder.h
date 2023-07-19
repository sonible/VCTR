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
/** Internal helper structure to pass runtime arguments to expressions in a chain
    built from a concatenation of expression chain builder instances.

    The expression chain builder primarily holds a template template for the
    expression chain which will then be instantiated  into a final expression once
    an actual source is appended to the chain. Until that point, we need a temporarily
    data structure that holds these values in order to apply it to the expression
    instance.
 */
template <is::stdTuple... ArgTuples>
class RuntimeArgChain
{
public:
    template <class... Args>
    requires (! (is::stdTuple<Args> || ...))
    constexpr RuntimeArgChain (Args... firstArgs)
        : chain {{ std::tuple (std::move (firstArgs)...) }}
    {}

    constexpr RuntimeArgChain()
    requires (sizeof... (ArgTuples) == 1)
        : chain {{ std::tuple<>() }}
    {}

    // TODO: This is a workaround for https://github.com/sonible/VCTR/issues/111 and should be replaced by a generic solution
    constexpr RuntimeArgChain()
    requires (sizeof... (ArgTuples) == 3)
        : chain {{ std::tuple<>() }, { std::tuple<>() }, { std::tuple<>() }}
    {}

    static constexpr size_t size() { return sizeof... (ArgTuples); }

    template <is::stdTuple... ArgTuplesToPrepend>
    constexpr RuntimeArgChain<ArgTuplesToPrepend..., ArgTuples...> prepend (RuntimeArgChain<ArgTuplesToPrepend...> chainToPrepend) const
    {
        return { std::tuple_cat (chainToPrepend.chain, chain) };
    }

    template <size_t idx>
    static constexpr bool hasValue()
    {
        return ! std::is_same_v<std::tuple<>, std::tuple_element_t<idx, decltype (chain)>>;
    }

    template <size_t idx>
    constexpr auto& get() const
    {
        return std::get<idx> (chain);
    }

private:
    template <is::stdTuple...> friend class RuntimeArgChain;

    constexpr RuntimeArgChain (std::tuple<ArgTuples...>&& newChain)
        : chain (std::move (newChain))
    {}

    std::tuple<ArgTuples...> chain;
};

template <class SrcChainer, template <size_t, class> class ExpressionToAdd>
struct ExpressionChainingHelper
{
    template <size_t extent, class SrcType>
    using NewExpressionChain = ExpressionToAdd<extent, typename SrcChainer::template Expression<extent, SrcType>>;

    template <class RuntimeArgs>
    using NewExpressionChainBuilder = ExpressionChainBuilderWithRuntimeArgs<NewExpressionChain, RuntimeArgs>;
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

/** A helper struct to indicate that a constant template should be considered disabled */
struct DisabledConstant
{
    /** A stupid type that converts itself to std::numeric_limits<T>::max when assigned to any numeric type T */
    struct Value
    {
        template <is::realNumber T>
        constexpr operator T() const { return std::numeric_limits<T>::max(); }

        template <is::realNumber T>
        constexpr operator std::complex<T>() const { return { std::numeric_limits<T>::max(), std::numeric_limits<T>::max() }; }
    };

    static constexpr Value value {};
};

/** An expression chain builder is an object which supplies various operator<< overloads which build chains of
    Expression Templates by prepending the templated ExpressionType to the source.

    Note that it is not the expression itself but rather contains all the information to instantiate the
    expression when a data source is prepended to it. This base class allows to store argument values needed
    by some expressions. Most expressions don't need that and can just instantiate a constexpr
    ExpressionChainBuilder instance. Others should create an instance using makeExpressionChainBuilderWithRuntimeArgs.

    @see ExpressionChainBuilder, makeExpressionChainBuilderWithRuntimeArgs
 */
template <template <size_t, class...> class ExpressionType, class RuntimeArgs, class... AdditionalCompileTimeParameters>
struct ExpressionChainBuilderWithRuntimeArgs
{
    template <size_t extent, class SrcType>
    using Expression = ExpressionType<extent, SrcType, AdditionalCompileTimeParameters...>;

    RuntimeArgs runtimeArgs;

    constexpr ExpressionChainBuilderWithRuntimeArgs (RuntimeArgs&& rtArgs)
        : runtimeArgs (std::move (rtArgs))
    {}

    constexpr ExpressionChainBuilderWithRuntimeArgs() {}

    /** Returns an expression which holds a reference to the Vector passed in as source */
    template <is::anyVctr Src>
    constexpr auto operator<< (const Src& src) const
    {
        auto expression = Expression<extentOf<Src>, const Src&> { src };
        expression.template iterateOverRuntimeArgChain<0> (runtimeArgs);

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
        expression.template iterateOverRuntimeArgChain<0> (runtimeArgs);

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
        expression.template iterateOverRuntimeArgChain<0> (runtimeArgs);

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
        expression.template iterateOverRuntimeArgChain<0> (runtimeArgs);

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
    constexpr auto operator<< (SrcExpressionChainBuilder srcExpressionChainBuilder) const
    {
        auto newRuntimeArgChain = srcExpressionChainBuilder.runtimeArgs.prepend (runtimeArgs);

        using ChainingHelper = detail::ExpressionChainingHelper<SrcExpressionChainBuilder, Expression>;

        return typename ChainingHelper::template NewExpressionChainBuilder<decltype(newRuntimeArgChain)> (std::move (newRuntimeArgChain));
    }
};

/** Convenient typedef to create a simple expression chain builder instance for an expression template. */
template <template <size_t, class...> class ExpressionType, class... AdditionalCompileTimeParameters>
using ExpressionChainBuilder = ExpressionChainBuilderWithRuntimeArgs<ExpressionType, detail::RuntimeArgChain<std::tuple<>>, AdditionalCompileTimeParameters...>;

/** Helper function to build factory functions for expressions that rely on runtime argument values.

    These kinds of expressions don't expose a constexpr instance of the expression chain builder but
    supply a factory function, which returns an expression chain builder instance which holds the
    desired value to apply. Example:
    Example:
    @code

    // The factory function that takes the desired runtime argument s.
    template <class T>
    constexpr auto clamp (T lowerBound, T upperBound)
    {
        return makeExpressionChainBuilderWithRuntimeArgs<Clamp> (lowerBound, upperBound);
    }
    @endcode

    The expression chain created like that will later pass the two arguments to the
    void applyRuntimeArgs member function of the expression in the same order as they
    were passed to makeExpressionChainBuilderWithRuntimeArgs – so in case you want to
    build an expression that takes runtime arguments, make sure to implement that function.
 */
template <template <size_t, class...> class ExpressionType, class... RuntimeArgs>
requires (sizeof... (RuntimeArgs) > 0)
auto makeExpressionChainBuilderWithRuntimeArgs (RuntimeArgs... runtimeArgs)
{
    return ExpressionChainBuilderWithRuntimeArgs<ExpressionType, detail::RuntimeArgChain<std::tuple<RuntimeArgs...>>> (detail::RuntimeArgChain<std::tuple<RuntimeArgs...>> (std::move (runtimeArgs)...));
}

/** Helper function to build factory functions for expressions that rely on runtime argument values.

    This function is just like makeExpressionChainBuilderWithRuntimeArgs except for allowing to pass one
    further template type argument to the ExpressionType.
 */
template <template <size_t, class...> class ExpressionType, class ExpressionTemplateArg, class... RuntimeArgs>
requires (sizeof... (RuntimeArgs) > 0)
auto makeTemplateExpressionChainBuilderWithRuntimeArgs (RuntimeArgs... runtimeArgs)
{
    return ExpressionChainBuilderWithRuntimeArgs<ExpressionType, detail::RuntimeArgChain<std::tuple<RuntimeArgs...>>, ExpressionTemplateArg> (detail::RuntimeArgChain<std::tuple<RuntimeArgs...>> (std::move (runtimeArgs)...));
}

} // namespace vctr
