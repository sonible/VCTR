# How Expression templates work

An expression template contains one or more possibilities to compute a certain (mathematical) expression.
The most basic expression template implementation must look like that:

```C++
template <size_t extent, class SrcType>
class MyExpression : ExpressionTemplateBase
{
public:
    using value_type = typename SrcType::value_type;
    
    using Expression = ExpressionTypes<value_type, SrcType>;

    constexpr MyExpression (SrcType s) : src (std::move (s)) {}

    constexpr size_t size() const { return src.size (); }

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const
    {
        return someComputation (src[i]);
    }
private:
    SrcType src;
};
```

The only exception to the minimum requirements outlined above are reduction expressions which are discussed separately
below.

Every kind of expression has to take at least two template arguments:

- The extent, which is denoted by the source of the expression
- The source type(s). Unary expressions, this means expressions that have a single source, need to specify a single
  source type. Expressions with multiple sources have to specify multiple source types accordingly.

When instantiated in the user code, the source type(s) will either be the type of the previous expression, if
expressions are chained or the type(s) of the source vector(s). It needs to store the source as a private member and the
constructor should take it by value and move it into that member. It has to implement the `size()` function, which will
simply forward the source `size()` function, and it has to implement a `operator[]` which contains the most basic
scalar implementation to evaluate the expression for a single source element. At best, this implementation is fully
constexpr.

## Creating an `ExpressionChainBuilder` instance

In order to use the `operator<<` chaining syntax, a unary expression has to be wrapped into an `ExpressionChainBuilder`
instance. This helper class supplies overloads for `operator<<` that return the actual expression. An
`ExpressionChainBuilder` instance has no members and should be `constexpr`. Simply declare it like this at the
end of your implementation:

```C++
constexpr ExpressionChainBuilder<MyExpression> myExpression;
```

## Binary expressions

The types of expressions described above are unary expressions, this means that they transform a single source vector or
expression. Operations like e.g. additions and multiplications need multiple operands. They should take two source
types as template argument and two constructor arguments accordingly. They are exposed via free functions or
operator overloads. This is how e.g. the vector addition implementation looks like:

```C++
template <class SrcAType, class SrcBType>
requires (is::anyVctrOrExpression<std::remove_cvref_t<SrcAType>> &&
          is::anyVctrOrExpression<std::remove_cvref_t<SrcBType>>)
constexpr auto operator+ (SrcAType&& a, SrcBType&& b)
{
    assertCommonSize (a, b);
    constexpr auto extent = getCommonExtent<SrcAType, SrcBType>();

    return AddVectors<extent, SrcAType, SrcBType> (std::forward<SrcAType> (a), std::forward<SrcBType> (b));
}
```

Note that we have to take care that sizes match when combining multiple sources through an expression. This can be
done by the `assertCommonSize` helper function, which performs a compile-time check in case both expressions have a
runtime defined extent or a run-time check in case one of them or both have a dynamic extent.

## Using alternative approaches to evaluate the expression

Besides the per-element evaluation through `operator[]`, the expression can implement various other ways to evaluate
the expression:

- AXV: Implement `AVXRegister<value_type> getAVX (size_t i) const` for suitable types. Make sure to require at
  least `archX64 && has::getAVX<SrcType>`
- SSE: Implement `SSERegister<value_type> getSSE (size_t i) const` for suitable types. Make sure to require at
  least `archX64 && has::getSSE<SrcType>`
- Neon: Implement `NeonRegister<value_type> getNeon (size_t i) const` for suitable types. Make sure to require at
  least `archARM && has::getAVX<SrcType>`
- Platform specific vector operations (Intel IPP, Apple Accelerate, etc). See dedicated section below.

Intel architecture specific implementations have to be prefixed with the `VCTR_TARGET (<arch>)` macro to instruct
the compiler to deliberately generate instructions for that instruction set, no matter what compiler flags are set. The
calling side will do a runtime check if the corresponding functions are available at runtime. Valid values for `<arch>`
are `"avx"`, `"avx2`" and `"sse4.1"`.

In case implementations are only available or make sense for specific constraints, you can constrain them and possibly
add multiple implementations for different types, e.g. like this

```C++
VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
requires (archX64 && has::getSSE<SrcType> && is::floatNumber<SrcElementType>)
{
    static const auto sseSignBit = SSESrcType::broadcast (SrcElementType (-0.0));

    return SSERetType::andNot (sseSignBit, src.getSSE (i));
}

VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
requires (archX64 && has::getSSE<SrcType> && std::same_as<int32_t, value_type>)
{
    return SSERetType::abs (src.getSSE (i));
}

VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
requires (archX64 && has::getSSE<SrcType> && is::unsignedIntNumber<value_type>)
{
    return src.getSSE (i); // unsigned integers are always positive
}
```

Here we see some explicit overloads for floating point values, int32 values and unsigned integers. For int64, there
is no straightforward abs function in SSE 4.1 so we don't implement it. Calling abs on an int64 vector might fall back
to the default `operator[]` implementation then. When assigning the expression, the implementation tries its best to
choose the most promising strategy for the given architecture it runs on, so it's a good idea to implement multiple
possibilities.

## Platform Specific Vector Operations

In many cases, highly optimized vector operation libraries like Intel IPP or Apple Accelerate outperform our
handwritten SIMD code. We can also use them to execute our expressions. To do so, we need to implement two functions,
`isNotAliased` and `evalNextVectorOpInExpressionChain`. Let's have a look of the `Abs` template again to see how they
are used:

```C++
constexpr bool isNotAliased (const void* dst) const
{
    return src.isNotAliased (dst);
}

VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
requires (platformApple && has::evalNextVectorOpInExpressionChain<SrcType, value_type> && is::floatNumber<value_type>)
{
    AccelerateRetType::abs (src.evalNextVectorOpInExpressionChain (dst), dst, int (size()));
    return dst;
}
```

`evalNextVectorOpInExpressionChain` takes a destination memory location as argument and returns a source memory
location. The returned pointer is the memory that the next expression should read from. The argument is the memory that
we write our expression result to. When we assign an expression to a Vector, it will pass the vectors' storage as
argument to `evalNextVectorOpInExpressionChain`. This way we are able to write the expression result directly into the
destination memory. A usual expression template should return the destination memory, making chained expression work in
place on the destination memory. `VctrBase` is the only class that returns its `data` pointer from
`evalNextVectorOpInExpressionChain`, so if the source is a vector, the first expression will perform an out-of-place
operation from the source memory into the destination memory.

As we write to the destination memory directly, there can be cases where we need the destination vector as a source
vector while evaluating the expression. Take this one for an example:
`
a = a + vctr::abs (b);
`
With the implementation strategy described above, the template would first call `evalNextVectorOpInExpressionChain` on
the `abs (b)` expression, take the memory of `b` as source and write the result into the memory of `a`. Then it would
perform the addition between the result of that computation and `a`. But wait, at that point, we already replaced the
value of `a` with the result of `abs (b)`. We have an aliasing problem here. To overcome this problem, we need
`isNotAliased`. It takes the destination memory as argument and returns false in case it detects a case of aliasing. As
aliasing is only a problem of binary expressions, unary expressions should simply forward the `isNotAliased` call as a
`constexpr` function. Binary expressions should do a check like this:

```C++
constexpr bool isNotAliased (const void* dst) const
{
    if constexpr (is::expression<SrcAType> && is::anyVctr<SrcBType>)
    {
        return dst != srcB.data();
    }

    if constexpr (is::anyVctr<SrcAType> && is::expression<SrcBType>)
    {
        return dst != srcA.data();
    }

    return true;
}
```

As evaluating a binary expression with two expressions as sources can never work with our
`evalNextVectorOpInExpressionChain` implementation strategy as it would need an intermediate buffer that we want to
avoid at all costs, binary expressions should always be constrained by the `is::suitableForBinaryEvalVectorOp` concept.

## Reduction expressions

While the expressions discussed above are used to transform a source vector into a destination vector, reduction
expressions reduce a source vector into a single reduction result value. Examples are e.g. a vector sum or finding
the maximum value in a vector. They use the same class template signature and also define an `ExpressionChainBuilder`
instance but their member functions look different. Futhermore, an expression chain terminated by a reduction expression
is not evaluated lazily when assigned to a destination but are evaluated right away and returns the computed single 
value.

The most basic reduction expression template implementation must look like that:

```C++
template <size_t extent, class SrcType>
class MyExpression : ExpressionTemplateBase
{
public:
    using value_type = typename SrcType::value_type;
    
    using Expression = ExpressionTypes<value_type, SrcType>;

    constexpr MyExpression (SrcType s) : src (std::move (s)) {}

    constexpr size_t size() const { return src.size (); }

    static constexpr value_type reductionResultInitValue = std::numeric_limits<value_type>::min();

    VCTR_FORCEDINLINE void reduceElementWise (value_type& result, size_t i) const
    {
        result = someComputation (result, src[i]);
    }
private:
    SrcType src;
};
```

The calling code will create a variable which is initialised to `reductionResultInitValue` and then passed to 
`reduceElementWise` which is called in a loop over all source elements.

When platform specific vector operations should be used, the required signature is
```c++
value_type reduceVectorOp() const
```

This usually requires, that the source type supplies direct access to the `data()` pointer and is 
likely not suitable when chained expressions are used, as there is no scratch buffer to write the
previous expression results to. Better constrain your implementation according to that, using e.g.
the `vctr::has::data` concept.

When SIMD operations should be used, the required signature is
```c++
    VCTR_FORCEDINLINE void reduceNeonRegisterWise (NeonRegister<value_type>& result, size_t i) const;

    VCTR_FORCEDINLINE VCTR_TARGET ("avx") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const;

    VCTR_FORCEDINLINE VCTR_TARGET ("avx2") void reduceAVXRegisterWise (AVXRegister<value_type>& result, size_t i) const;

    VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") void reduceSSERegisterWise (SSERegister<value_type>& result, size_t i) const;

    template <size_t n>
    VCTR_FORCEDINLINE static value_type finalizeSIMDReduction (const std::array<value_type, n>& subResults);
```

They basically work the same as the element-wise implementations but they evaluate a whole SIMD register
at a time. Possible residual elements are then evaluated using a scalar loop in the calling code. This leads to a SIMD
register and a single scalar value as sub-results, which are passed to `finalizeSIMDReduction` for a last final 
reduction step.
