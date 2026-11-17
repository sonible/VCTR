# Review checklist

Condensed sonible C++ code style plus VCTR-specific checks. The repository's `AGENTS.md` files and `.clang-format`
override this list. Default severity is in brackets: **M** = Must fix, **S** = Should fix, **N** = Nitpick. Raise or
lower severity based on actual impact in context.

## Correctness and safety

| ID | Check | Sev |
| --- | --- | --- |
| `LIFE-1` | Spans, string views, and raw pointers are not retained or returned past the lifetime of their owner or an invalidating call (`resize`, `push_back`, `reserve` on a `Vector`). | M |
| `LIFE-2` | Expressions hold their sources by reference unless they were passed as rvalues. Expressions stored in a variable (`auto e = a + b;`) or returned from a function do not outlive a source; temporaries are moved into the expression, not referenced. | M |
| `OWN-1` | Ownership is explicit: value members, `std::unique_ptr`, or RAII wrappers. No owning raw `new`/`delete`. | M |
| `OWN-2` | Copy/move is intentional; deleted when copying is invalid. No top-level `const` on returned values. | S |
| `ALLOC-1` | No hidden heap allocation: expression evaluation, assignment into existing containers and `Span`/`Array` operations never allocate. Only explicitly allocating `Vector` members (constructors, `resize`, `push_back`, ...) may, and functions that allocate say so in their docs. | M |
| `SIMD-1` | SIMD paths handle sizes that are not a multiple of the register width, use unaligned loads unless the storage info guarantees alignment (`isDataSIMDAligned`), and don't read past the end unless `isStorageSIMDExtended`. | M |
| `SIMD-2` | In-place and aliasing cases (`a = vctr::abs << a`, sources overlapping the destination) give the same result on every evaluation path. | M |
| `NUM-1` | Boundaries defined and handled: zero, negative input to `log`/`sqrt`, non-finite values, empty sources, integer overflow and signed/unsigned wraparound. | M |
| `NUM-2` | All evaluation paths (scalar, AVX, SSE, Neon, Accelerate, IPP, constexpr/gcem) agree within a documented tolerance; reductions document that SIMD changes the evaluation order. | M |
| `NUM-3` | `size_t` to `int` conversions (IPP takes `int` sizes) go through `sizeToInt` or an equivalent checked conversion, not a plain cast. | M |
| `NUM-4` | `VCTR_ASSERT` and `operator[]` bounds checks are debug-only. Sizes that come from the caller are not assumed valid in release builds where a mismatch would corrupt memory; document the precondition at least. | S |
| `EXC-1` | `noexcept` only where the body and all callees uphold it. | M |

## Portability and compile time

| ID | Check | Sev |
| --- | --- | --- |
| `PORT-1` | Compiles on every CI configuration: Clang 15+, GCC 13+, MSVC/clang-cl, with and without IPP, x64 and ARM, Apple and non-Apple. No reliance on one compiler's leniency (e.g. floating point non-type template parameters, missing `typename`/`template` disambiguators). | M |
| `PORT-2` | Architecture- and library-specific code is guarded by `archX64`/`archARM`/`platformApple`/`hasIPP` in `requires` clauses or `#if` blocks, and only calls SIMD/platform functions that exist for that element type in `SIMD/` or `PlatformVectorOps/`. | M |
| `PORT-3` | x64 SIMD functions carry the matching `VCTR_TARGET` attribute (`"fma"` for float/double AVX, `"avx2"` for integer AVX, `"sse4.1"` for SSE); Neon functions carry none. | M |
| `CONST-1` | New functionality is `constexpr` where possible and actually usable in a constant expression; math in `operator[]` branches on `std::is_constant_evaluated()` and uses `gcem::` there. | S |

## Expression templates (`include/vctr/Expressions/`)

| ID | Check | Sev |
| --- | --- | --- |
| `EXPR-1` | Template parameter order is `<extent, SrcType(s), extra types...>`; `value_type` is declared before the `VCTR_COMMON_*_EXPRESSION_MEMBERS` macro; the source member is named `src` (or `srcA`/`srcB`). | M |
| `EXPR-2` | Type-dependent code uses the macro's `Expression` alias (`Expression::AVX`, `Expression::CommonElement::isRealFloat`, ...) instead of re-deriving types. | S |
| `EXPR-3` | Every `getAVX`/`getSSE`/`getNeon` has a matching `prepare<Arch>Evaluation()`, either via the forward macro or hand-written, never both. | M |
| `EXPR-4` | Overloads of the same evaluation hook have mutually exclusive `requires` clauses; platform vector ops use the right `is::suitableFor...VectorOp` concept with the `dontPreferIfIppAndAccelerateAreAvailable` / `preferIfIppAndAccelerateAreAvailable` tags. | M |
| `EXPR-5` | Compile-time constant variants come in both forms, `name<v>` (`Constant<v>`) and `nameRef<v>` (`ConstantRef<v>`). Runtime arguments are cheap to copy (they are copied per expression instance). | S |
| `EXPR-6` | The scalar `operator[]` (or `reduceElementWise`) is always available and is the reference for all accelerated paths. | M |
| `EXPR-7` | Naming and semantics match existing expressions and their `std` counterparts (`abs`, `max`, `clamp`, ...). | S |

## Headers and layout

| ID | Check | Sev |
| --- | --- | --- |
| `HDR-1` | Every new header is included in `include/vctr/vctr.h` in the matching category block and at a position where its dependencies are already included. | M |
| `HDR-2` | `CamelCase` headers have no includes of their own; only `snake_case` headers are user-includable. | S |
| `HDR-3` | Every C++ file starts with the LGPL copyright header (check with `fix_headers.py --check`). Existing copyright lines are not changed. | S |

## Design

| ID | Check | Sev |
| --- | --- | --- |
| `DES-1` | Composition preferred over inheritance when it expresses the relationship; inheritance access is explicit. | S |
| `DES-2` | Mutable shared state is scoped to an owner, not global. | S |
| `DES-3` | Parameters pass by value (cheap types), const reference (large read-only), forwarding reference (expression sources), or `Span<const T>` (contiguous read-only ranges) deliberately. | N |
| `DES-4` | Third-party code under `ext/` is not edited in place. | M |
| `DES-5` | Obsolete code is removed, not commented out. | N |

## Language and style

Formatting is owned by `.clang-format`; do not hand-review whitespace when the formatter check can run. Report only
what it cannot catch.

| ID | Check | Sev |
| --- | --- | --- |
| `LANG-1` | Members initialized in-class or in the initializer list, not assigned in the constructor body. Typed constants/`constexpr`, not macros. | S |
| `LANG-2` | `const` on observational methods and locals where it communicates intent; `override` without `virtual`; `explicit` on single-argument constructors unless implicit conversion is intended (as for `Span`). | S |
| `LANG-3` | Type constraints use named concepts rather than equivalent traits, without widening the accepted set; VCTR concepts live in `vctr::is`, `vctr::has`, `vctr::are`. | N |
| `LANG-4` | `nullptr` with explicit comparisons; range-based loops; pre-increment; early returns; named C++ casts; `std::` math overloads; fixed-width integers where exact width matters; typed literals like `0.0f`. | N |
| `LANG-5` | `auto` only when the initializer shows the type or for expression objects; explicit type when signedness or precision matters. | N |
| `NAME-1` | `UpperCamelCase` types and type template parameters, `lowerCamelCase` functions/variables/non-type template parameters, forwarded `std` members keep `snake_case` (`size()`, `push_back()`), `VCTR_` prefix for macros. | N |
| `LAY-1` | Public section first, then protected/private; constructors/destructor first; members before private helpers; `struct` only for intentionally public simple types. | N |
| `FMT-1` | Run the formatter in check mode on the reviewed files. Report differences once as a single nitpick, not per line. | N |

## Documentation

| ID | Check | Sev |
| --- | --- | --- |
| `DOC-1` | Public API has a `/** */` comment that explains what the signature does not: borrowed lifetimes, allocation, preconditions, supported element types, numerical caveats. | S |
| `DOC-2` | Public expressions carry `@ingroup Expressions`, core containers `@ingroup Core`. | S |
| `DOC-3` | Source comments explain non-obvious intent; no commented-out code. | N |

For detailed Doxygen style checking, defer to `vctr-cpp-check-docs`.

## Tests

| ID | Check | Sev |
| --- | --- | --- |
| `TEST-1` | Behavior changes and bug fixes come with a focused regression test including the boundary case. | S |
| `TEST-2` | New test files are listed in `test/CMakeLists.txt`; otherwise they are silently not built. | M |
| `TEST-3` | Expression tests use `TEMPLATE_PRODUCT_TEST_CASE` over evaluation strategies and element types, and prepend `filter <<` so every evaluation path is exercised; they include the unaligned `srcUnaligned` source and sizes that are not a multiple of the SIMD width. | S |
| `TEST-4` | Tolerances are explicit and justified (`withEpsilon`); value ranges match the function's domain (`VCTR_TEST_DEFINES_IN_RANGE`, `..._NO_ZEROS`). | S |
| `TEST-5` | `constexpr`-capable functionality has a `static constexpr` check in the test. | S |
| `TEST-6` | The tests demonstrably fail when the implementation is broken (mutation check per evaluation path, see `test/AGENTS.md`); paths that cannot be compiled on the author's machine are named. | S |
| `TEST-7` | Benchmarks live in `benchmark/`, not in correctness tests. | N |
