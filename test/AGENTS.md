# Tests

Catch2 v3. New `.cpp` files must be listed in `test/CMakeLists.txt` (`vctr_test_cases` sources), otherwise they are
silently not built. Expression tests live in `TestCases/Expressions/<Name>.cpp`.

## Expression test pattern

```c++
#include <vctr_test_utils/vctr_test_common.h>

TEMPLATE_PRODUCT_TEST_CASE ("MyOp", "[VCTR][Expressions][myOp]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, std::complex<float>))
{
    VCTR_TEST_DEFINES (10)   // ElementType, filter, srcA/srcB (constexpr Array), srcC/srcD (Vector), srcUnaligned (Span)

    const vctr::Vector r = filter << vctr::myOp << srcA;
    REQUIRE_THAT (r, vctr::EqualsMappedBy ([] (ElementType x) { return reference (x); }, srcA).withEpsilon (0.00001));

    // constexpr check, if the expression supports it
    [[maybe_unused]] static constexpr vctr::Array c = filter << vctr::myOp << srcA;
}
```

- The first type list selects the evaluation strategy via `filter` (`DontUseAcceleration`, `PlatformVectorOps`,
  `VCTR_NATIVE_SIMD`, `VCTR_ALL_EVALUATION_STRATEGIES`). Always prepend `filter <<` so every path is exercised.
- Pick value ranges with `VCTR_TEST_DEFINES_IN_RANGE (start, end, size)`, `..._NO_ZEROS`, `..._WITH_TRAILING_ZERO`
  (see `include/vctr_test_utils/vctr_test_common.h`).
- Prefer `vctr::EqualsMappedBy (fn, srcs...)` (any callable, n sources) over the older `EqualsTransformedBy<fn>`;
  other matchers: `vctr::Equals`, `vctr::AllElementsEqualTo` (all in `include/vctr_test_utils/Matchers/`).
- Test vec-vec, vec-scalar, scalar-vec and constant variants in separate `SECTION`s (see `Expressions/Pow.cpp`).
- Test results, not implementation details. Don't assert in tests which evaluation path an expression takes, e.g. no
  `static_assert (vctr::has::evalNextVectorOpInExpressionChain<...>)` or `if constexpr (vctr::Config::platformApple)`
  blocks. The `filter` selects the path and the mutation check below proves it is covered. If you find that a path
  can't be reached, fix the implementation and mention it when reporting instead of encoding it in the test.

## Mutation check

Every new or changed piece of functionality needs a test, and where possible verify the test with manual mutations.
For each code path you added, one at a time:

1. Introduce a small bug, e.g. `+` → `-`, `<` → `<=`, off-by-one on an index or size, swap two arguments, return the
   source unchanged, drop a `clampLow`/`if constexpr` branch.
2. Rebuild and run only the relevant tag: `./build/test/TestMain/vctr_test "[myOp]"`. It must **fail**.
3. Revert the mutation (`git diff` must show only your intended change afterwards).

For expressions, mutate each evaluation path separately (`operator[]`, `getAVX`, `getSSE`, `getNeon`,
`evalNextVectorOpInExpressionChain`, `reduce*`): the test must fail for the filter that selects that path. A path that
can't be made to fail on this machine (e.g. AVX on ARM, IPP without IPP) can't be verified here — say so when reporting.
If a mutation survives, strengthen the test (tighter epsilon, more value ranges, the unaligned `srcUnaligned` source,
sizes that aren't multiples of the SIMD width) rather than accepting it.
