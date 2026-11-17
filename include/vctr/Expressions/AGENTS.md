# Writing expression templates

Background: [Readme.md](Readme.md) (concepts) and [../PlatformVectorOps/Readme.md](../PlatformVectorOps/Readme.md).
**Some snippets in those READMEs are outdated** (e.g. `platformApple && has::...` constraints, `AccelerateRetType`,
`SSERetType`). When the README and the code disagree, follow the code. Copy the closest existing expression instead of
writing from scratch:

| Need | Copy from |
|---|---|
| Plain unary (`vctr::square << v`) | `BasicMath/Square.h` |
| Unary that changes the value type (complex → real) | `BasicMath/Abs.h` |
| Runtime arguments (`vctr::clamp (lo, hi) << v`) | `Clamp` in `BasicMath/Clamp.h` |
| Compile-time constant (`vctr::addConstant<3> << v`) | `AddConstantToVec` in `BasicMath/Add.h`, `ClampByConstant` |
| Binary vec-vec / scalar-vec (`a + b`, `2.0f + a`) | `AddVectors`, `AddSingleToVec` in `BasicMath/Add.h` |
| Reduction (returns one value) | `BasicMath/Sum.h`, `BasicMath/Max.h` |
| Generic / callable based | `Generic/Map.h`, `Generic/TransformedBy.h` |

## Skeleton (unary)

```c++
namespace vctr::expressions
{

template <size_t extent, class SrcType>               // always <extent, SrcType(s), extra type params...>
class MyOp : ExpressionTemplateBase
{
public:
    using value_type = ValueType<SrcType>;            // must come BEFORE the macro

    VCTR_COMMON_UNARY_EXPRESSION_MEMBERS (MyOp, src)  // source member MUST be named `src`; macro ends in `public:`

    VCTR_FORCEDINLINE constexpr value_type operator[] (size_t i) const { return someComputation (src[i]); }

    // Optional accelerated paths, see below
};

} // namespace vctr::expressions

namespace vctr
{
/** Does something.

    @ingroup Expressions
 */
constexpr inline ExpressionChainBuilder<expressions::MyOp> myOp;
} // namespace vctr
```

Then: include the header in `include/vctr/vctr.h` (in its category block) and add a test (see `test/AGENTS.md`).

## Rules that cause most compile errors

- The macro privately defines `using Expression = ExpressionTypes<value_type, SrcType...>`. Use it for everything
  type-dependent: `Expression::AVX`, `Expression::SSE`, `Expression::Neon`, `Expression::Accelerate`, `Expression::IPP`,
  `Expression::CommonElement::isRealFloat / isInt / isInt32 / isUnsigned / isSigned / isComplexFloat`,
  `Expression::allElementTypesSame`.
- `VCTR_FORWARD_PREPARE_SIMD_EVALUATION_UNARY_EXPRESSION_MEMBER_FUNCTIONS` hardcodes the name `src`. Binary uses
  `VCTR_FORWARD_PREPARE_SIMD_EVALUATION_BINARY_EXPRESSION_MEMBER_FUNCTIONS (srcA, srcB)`.
- Any `getAVX` / `getSSE` / `getNeon` needs a matching `prepare<Arch>Evaluation()`: either the forward macro, or —
  if you keep a `mutable SIMDRegisterUnion<Expression> x {};` (the `{}` is required for constexpr) — write them by hand
  and call `src.prepare<Arch>Evaluation()` first. Never both.
- Overloads with the same signature need **mutually exclusive** `requires` clauses, otherwise calls are ambiguous.
- Only call SIMD / platform functions that exist for that element type in `SIMD/*/…Register.h` or
  `PlatformVectorOps/{AppleAccelerate,IntelIPP}.h`. If missing, add the wrapper there first (inside the `#if` block of
  the matching type specialisation).
- In a class template, extra compile-time params are **types** (`Constant<v>`, `ConstantRef<v>`, `std::true_type`,
  `DisabledConstant`), passed as `ExpressionChainBuilder<expressions::X, Constant<v>>`. Read them as `T::value`.

## Accelerated paths (all optional, `operator[]` is the always-available fallback)

```c++
// Platform vector ops: one Accelerate + one IPP overload, preference tags exactly like this
VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
requires is::suitableForAccelerateRealFloatVectorOp<SrcType, value_type, detail::dontPreferIfIppAndAccelerateAreAvailable>
{
    Expression::Accelerate::myOp (src.evalNextVectorOpInExpressionChain (dst), dst, size());          // size_t
    return dst;
}

VCTR_FORCEDINLINE const value_type* evalNextVectorOpInExpressionChain (value_type* dst) const
requires is::suitableForIppRealFloatVectorOp<SrcType, value_type, detail::preferIfIppAndAccelerateAreAvailable>
{
    Expression::IPP::myOp (src.evalNextVectorOpInExpressionChain (dst), dst, sizeToInt (size()));     // int
    return dst;
}

// SIMD: target attributes as used throughout the codebase
VCTR_FORCEDINLINE VCTR_TARGET ("fma") AVXRegister<value_type> getAVX (size_t i) const          // float/double
requires (archX64 && has::getAVX<SrcType> && Expression::CommonElement::isRealFloat) { ... }

VCTR_FORCEDINLINE VCTR_TARGET ("avx2") AVXRegister<value_type> getAVX (size_t i) const         // integers
requires (archX64 && has::getAVX<SrcType> && Expression::CommonElement::isInt) { ... }

VCTR_FORCEDINLINE VCTR_TARGET ("sse4.1") SSERegister<value_type> getSSE (size_t i) const
requires (archX64 && has::getSSE<SrcType> && Expression::CommonElement::isRealFloat) { ... }

NeonRegister<value_type> getNeon (size_t i) const                                               // no VCTR_TARGET
requires (archARM && has::getNeon<SrcType> && Expression::CommonElement::isRealFloat) { ... }
```

Pick the right `is::suitableFor...` concept from `TypeTraitsAndConcepts/ContainerAndExpressionConcepts.h`
(`RealFloat`, `ComplexFloat`, `RealOrComplexFloat`, `ComplexToRealFloat`, `...BinaryVectorOp`, `...VectorReductionOp`).
Chaining multiple platform calls: first reads `src.evalNextVectorOpInExpressionChain (dst)`, later ones work in place on `dst`.

## Variants

- **Runtime args**: add `constexpr void applyRuntimeArgs (A a, B b)` storing into members; expose a free function
  `template <class T> constexpr auto myOp (T a, T b) { return makeExpressionChainBuilderWithRuntimeArgs<expressions::MyOp> (a, b); }`.
  Args are copied per expression instance, keep them cheap.
- **Constants**: provide both `myOp<auto v>` (`Constant<v>`) and `myOpRef<const auto& v>` (`ConstantRef<v>`) — many
  compilers reject floating point NTTPs.
- **Binary vec-vec**: `VCTR_COMMON_BINARY_VEC_VEC_EXPRESSION_MEMBERS (X, srcA, srcB)`, value type via
  `CommonVecExpressionType<ValueType<SrcAType>, ValueType<SrcBType>, op>`. Expose as a free function/operator:
  `assertCommonSize (a, b); constexpr auto extent = getCommonExtent<SrcAType, SrcBType>(); return expressions::X<extent, SrcAType, SrcBType> (std::forward<SrcAType> (a), std::forward<SrcBType> (b));`
- **Binary scalar-vec**: `VCTR_COMMON_BINARY_SINGLE_VEC_EXPRESSION_MEMBERS (X, src, single)`, construct with
  `expressions::X<extentOf<Src>, Src> (single, std::forward<Src> (vec))`.
- **Reduction**: same signature + macro, but instead of `operator[]` implement
  `static constexpr value_type reductionResultInitValue`, `constexpr void reduceElementWise (value_type& result, size_t i) const`,
  `template <size_t n> static constexpr value_type finalizeReduction (const std::array<value_type, n>&)`.
  Optional: `reduceVectorOp()` (requires `...VectorReductionOp` concepts, uses `src.data()`) and
  `reduce{AVX,SSE,Neon}RegisterWise (Register& result, size_t i)`.
- **constexpr math**: in `operator[]`, branch on `std::is_constant_evaluated()` and use `gcem::` there, `std::` otherwise.
