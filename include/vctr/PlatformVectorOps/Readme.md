# platform_vec_ops

platform_vec_ops is a generic name for third-party, platform optimized vector math operations. At the moment we
particularly use functions from the Apple Accelerate Framework and from the Intel IPP library.

Both, Accelerate and IPP are not available on all systems. For Apple, we assume that Accelerate is generally available,
the CMake target already links against it. If IPP should be used, the user has to specify `VCTR_HAS_IPP` and ensure
that IPP headers are available on the search path and that we link against it properly.

PlatformVectorOp functions are wrapped behind a struct template, which is empty by default, but have specializations
for the datatype for which they are available. These specializations are wrapped in `#if` blocks so that they are only
available if the platform we build for supports them. Using this approach of an empty default struct and
specializations, we can reference the struct in code without needing to wrap that code into `#if` sections, we only
need to ensure that the relevant functions are not available in case there is no matching operation by constraining them
via concepts. Let's take the `AppleAccelerate` implementation as an example:

```C++
namespace vctr::platform_vec_ops
{

template <class T>
struct AppleAccelerate
{};

#if VCTR_APPLE

template <>
struct AppleAccelerate<float>
{
    //==============================================================================
    // vForce functions
    //==============================================================================
    static void abs (const float* src, float* dst, int len)                        { vvfabsf (dst, src, &len); }
    //... some more
    
    //==============================================================================
    // vDSP functions
    //==============================================================================
    static void add (const float* srcA, const float* srcB, float* dst, size_t len) { vDSP_vadd (srcA, 1, srcB, 1, dst, 1, len); }
    static void add (const float* srcA, float srcB,        float* dst, size_t len) { vDSP_vsadd (srcA, 1, &srcB, dst, 1, len); }
    static void sub (const float* srcA, const float* srcB, float* dst, size_t len) { vDSP_vsub (srcA, 1, srcB, 1, dst, 1, len); }
    static void mul (const float* srcA, const float* srcB, float* dst, size_t len) { vDSP_vmul (srcA, 1, srcB, 1, dst, 1, len); }
    static void mul (const float* srcA, float srcB,        float* dst, size_t len) { vDSP_vsmul (srcA, 1, &srcB, dst, 1, len); }
    static void div (const float* srcA, const float* srcB, float* dst, size_t len) { vDSP_vdiv (srcA, 1, srcB, 1, dst, 1, len); }
    static void div (const float* srcA, float srcB,        float* dst, size_t len) { vDSP_vsdiv (srcA, 1, &srcB, dst, 1, len); }
    //... some more
};
    
//... some further specializations for other types
#endif
```

We see, that the template specialization acts as a very thin wrapper around the functions which are available by the
framework. E.g. functions from vForce take signed `int` length arguments while functions taken from vDSP take `size_t`
length arguments, so we forward them accordingly. vDSP offers functions for adding, multiplying and dividing a vector
to/by a scalar but non for subtracting a subtracting a scalar from a vector. Therefore, we only have a single `sub`
overload but multiple ones for the other three operations. Still we apply a few common adoptions, e.g. the stride
values for vDSP functions are always hard coded to 1, and the pointer to the size argument required by vForce is
converted to a by-value argument in the wrapper.

## Using them in expressions

Before continuing here, you should have understood the general ideas of how to write Expression templates. The relevant
documentation is found [here](../Expressions/Readme.md).

As described above, we have to take care that the platform specific operations are only invoked in case they are
supported on the platform we are building for. Let's have a look at how the `Abs` expression uses the Accelerate
abs operation:

```C++
const ReturnElementType* evalNextVectorOpInExpressionChain (ReturnElementType* dst) const
requires (platformApple && has::evalNextVectorOpInExpressionChain<SrcType, ReturnElementType> && is::realFloatNumber<ReturnElementType>)
{
    AccelerateRetType::abs (src.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
    return dst;
}
```

We see that the first constraint is `platformApple` – this way, we would disable that function overload
if we are building for a non-apple platform where the corresponding struct would be an empty dummy struct. Then, after
checking if the source allows to evaluate vector ops, we check if the data type matches – Accelerate defines functions
for `float` and `double` so we constrain it to `is::realFloatNumber`. This way, we can be sure that this code path is only
taken if we are evaluating abs for float or double on Apple.

Now if this should also use IPP on x64 CPUs in case of non-apple, we could add another overload with different constraints like this:

```C++
const ReturnElementType* evalNextVectorOpInExpressionChain (ReturnElementType* dst) const
requires (hasIPP && ! platformApple && has::evalNextVectorOpInExpressionChain<SrcType, ReturnElementType> && is::realFloatNumber<ReturnElementType>)
{
    IPPRetType::abs (src.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));
    return dst;
}
```

## Safe int casting

`vctr::Vector` uses `size_t` types for sizes, which is the de-facto standard library standard. Some platform specific
implementations however use `int` as the size types. In nearly every case, the range of a 32 bit integer should be all
you need. Still, we are able to generate vectors of sizes that exceed the range of an `int`. To catch such rare edge
cases, use the `sizeToInt` function rather than a direct cast. It will warn the user via assertion in case the cast will
likely produce a dangerous truncation.
