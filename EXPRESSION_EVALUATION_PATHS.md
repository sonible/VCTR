# VCTR expression evaluation paths: current state and gap plan

Status: analysis of `develop` @ f086e10 (2026-11-26). Based on reading the headers (`include/vctr/Expressions`,
`SIMD/*Register.h`, `PlatformVectorOps/*`), not on benchmarks or compile experiments, except where noted.

## 1. How a path gets selected (and why gaps hurt more than they look)

`VctrBase::assignExpressionTemplate` (`Containers/VctrBase.h:1027`) tries, at runtime and in this order:

1. **Platform vector op** (`evalNextVectorOpInExpressionChain`, IPP or Accelerate), if the destination isn't aliased.
2. **Neon** (`getNeon`), no runtime check.
3. **AVX**: `getAVX` with the `fma` target for float/double, `avx2` for integers, gated by a CPUID check.
4. **SSE 4.1** (`getSSE`).
5. Scalar `operator[]` loop (always available, also the only constexpr path).

Reductions (`ReductionExpression.h:60`) use the same order with `reduceVectorOp` / `reduce{Neon,AVX,SSE}RegisterWise`.
A platform reduction only works on a container source, since it reads `src.data()`.

**A strategy is all-or-nothing for the whole chain.** `has::getAVX<Expression>` is only true if every node in the
tree has `getAVX`. Platform ops and SIMD can't be mixed. So one node without SIMD makes the whole expression scalar.
For example, `vctr::sqrt << (a * a + b * b)` runs scalar on every SIMD backend today, because `Sqrt` has no SIMD path.
On Linux with GCC/Clang and no IPP, `magToDb` is scalar because `Log10` has no SIMD path there. That's why the cheap
gaps below are worth closing even where each one looks minor.

Weighting used in this report:

- **Neon** (Apple Silicon, ARM Linux) and **AVX2/FMA** (every current x64 CPU) matter most.
- **IPP** is the main platform library on x64; **Accelerate** on Apple.
- **SSE 4.1** is only a fallback now: it covers CPUs without FMA/AVX2 (pre-2013) and the Intel-Mac vForce paths.
  SSE-only gaps are therefore low priority. The exception is a node that only has SSE, which drags a chain down to SSE.

## 2. Coverage matrix

Legend: ✅ = implemented for the natural types · ◐ = partial (types listed) · ❌ = missing, could be implemented ·
— = sensible restriction (see §3) · `svml` = MSVC only (`VCTR_HAS_SVML`) · `vf` = Apple vForce only (float).

### Element-wise arithmetic

| Expression | IPP | Accelerate | AVX | SSE | Neon | Notes |
|---|---|---|---|---|---|---|
| `AddVectors` (`a + b`) | ✅ r+c | ✅ r+c | ◐ float, **unsigned only** | ✅ | ✅ | AVX excludes signed ints although `add_epi32/64` exist (**bug-like gap**) |
| `AddSingleToVec` | ✅ r+c | ✅ r+c | ✅ float+int | ✅ | ❌ | AddConstantToVec has Neon, this one doesn't |
| `AddConstantToVec` | ✅ r+c | ◐ real only | ◐ float | ◐ float | ◐ float | Accelerate complex wrapper exists (used by AddSingleToVec); int SIMD missing |
| `SubtractVectors` | ✅ r+c | ◐ real only | ◐ float, **unsigned only** | ✅ | ✅ | Accelerate `zvsub` wrapper exists but isn't used; same signed-int AVX gap |
| `SubtractVecFromSingle` | ✅ r+c | ◐ real | ✅ float+int | ✅ | ✅ | |
| `SubtractSingleFromVec` | ✅ r+c | ✅ r+c | ✅ float+int | ✅ | ✅ | |
| `MultiplyVectors` | ✅ r+c | ✅ r+c | ◐ float | ❌ | ◐ no int64 | **No SSE at all**; AVX int needs `mullo` wrapper; Neon int64 — |
| `MultiplyVecBySingle` | ✅ r+c | ◐ real | ◐ float | ◐ float | ❌ | Neon missing; int SIMD missing |
| `MultiplyVecByConstant` | ✅ r+c | ◐ real | ◐ float | ◐ float | ◐ float | int SIMD missing |
| `DivideVectors` | ✅ r+c | ✅ r+c | ✅ float | ✅ float | ❌ | Neon `div` wrapper exists for float/double |
| `DivideSingleByVec` | ◐ float only (—) | ❌ | ✅ float | ✅ float | ❌ | `vDSP_svdiv[D]` exists; IPP has no 64f `DivCRev` |
| `DivideVecBySingle` | ✅ r+c | ◐ real | ✅ float | ✅ float | ❌ | |
| `MultiplyAccumulateVectors` | ❌ | ❌ | ◐ float | ❌ | ◐ | `vDSP_vma` / `ippsAddProduct`; see bugs in §4 |
| `MultiplySubtractVectors` | ❌ | ❌ | ◐ float | ❌ | ◐ | `vDSP_vmsb`; IPP via `AddProduct` with a negated operand is awkward (—) |
| `Abs` | ✅ float, int32, c→r | ✅ float, c→r | ✅ float, int32, unsigned | ✅ same | ✅ | int64 AVX/SSE — (no `abs_epi64` before AVX-512); complex SIMD — |
| `Square` | ✅ r+c | ✅ r+c | ◐ float | ◐ float | ◐ float | int SIMD missing (Neon int32 `mul` already exists) |
| `Cube` | — | — | ◐ float | ◐ float | ✅ float+int32 | no library primitive (§3); AVX/SSE int needs `mullo` |
| `Sqrt` | ✅ r+c | ✅ real | ❌ | ❌ | ❌ | **No SIMD at all.** No `sqrt` in any register wrapper |
| `SignedSqrt` | ❌ | ✅ | ❌ | ❌ | ❌ | Falls out of adding `sqrt` to the registers |
| `Clamp` (runtime bounds) | ✅ | ✅ | ❌ | ❌ | ❌ | `ClampByConstant` has all three; runtime variant has no SIMD |
| `ClampByConstant` | ✅ | ✅ | ◐ float | ◐ float | ◐ float | int clamp via `max/min` int32 wrappers possible |
| `ElementwiseMax/Min` | ❌ | ❌ | ✅ float, (u)int32 | ✅ | ✅ | `vDSP_vmax/vmin[D]`; IPP `ippsMaxEvery/MinEvery` (check signature per IPP version) |
| `NormalizeSum` | ✅ | ✅ | ❌ | ❌ | ❌ | sum is computed up front, so SIMD is just a broadcast divide |
| `Map`, `MapFrom0To1`, `MapTo0To1` | ✅ | ✅ | ❌ | ❌ | ❌ | affine maps, one or two `fma`/`mul`/`add` per register |

### Exponential, logarithm, power

| Expression | IPP | Accelerate | AVX | SSE | Neon | Notes |
|---|---|---|---|---|---|---|
| `Exp` | ✅ r+c | ✅ float, int32→float | `svml` | `svml`/`vf` | `vf` | reference implementation for the transcendental pattern |
| `Ln`, `Log10`, `Log2` | ✅ | ✅ (+int32) | ❌ | ❌ | ❌ | register wrappers (`log`, `log10`, `log2`) **already exist** for svml/vf but aren't used |
| `PowVectors` | ✅ r+c | ✅ real | ❌ | `vf` | `vf` | AVX svml `pow` wrapper exists, unused; Neon concept typo (§4) |
| `PowSingleExponent` (`pow (v, s)`) | ✅ | — | ❌ | ❌ | ❌ | SIMD: `pow (x, broadcast (e))` via svml/vf |
| `PowConstantExponent` | ✅ | — | ❌ | ❌ | ❌ | same; special cases 2 → square, 0.5 → sqrt are cheap |
| `PowSingleBase` (`pow (s, v)`) | ❌ | ❌ | ❌ | ❌ | ❌ | **No acceleration at all.** `exp (x · ln b)` maps to `mulC` + `exp` on both libs |
| `PowConstantBase` | **fake** | **fake** | ❌ | ❌ | ❌ | the "platform" path is a scalar `std::pow` loop (TODO in code). Used by `dbToMag` |
| `FastExp` | — | — | ✅ float | ✅ | ✅ | real only; complex — |
| `FastExp2` | — | — | ◐ float | ◐ float | ✅ | AVX/SSE double — (double → int64 conversion needs AVX-512) |
| `FastLog2` | — | — | ✅ | ✅ | ✅ | float only by design |
| `magToDb` / `dbToMag` (aliases) | inherit | inherit | inherit | inherit | inherit | `magToDb` = Clamp∘Mul∘**Log10**; `dbToMag` = **PowConstantBase**∘Mul |

### Trigonometric (`Sin Cos Tan Asin Acos Atan Sinh Cosh Tanh Asinh Acosh Atanh`)

| | IPP | Accelerate | AVX | SSE | Neon |
|---|---|---|---|---|---|
| All 12 | ✅ r+c | ✅ real | ❌ | `vf` only | `vf` only |

The SSE/Neon paths call `vsinf` & co. directly instead of the register wrappers. So the MSVC/SVML paths that the
registers already provide (`SSE::sin`, `AVX::sin`, …) are never reached. `asin`, `acos`, `atan` have no register wrapper
at all.

### Reductions

| Expression | IPP | Accelerate | AVX | SSE | Neon | Notes |
|---|---|---|---|---|---|---|
| `Sum` | ✅ r+c | ◐ real | ◐ float, int32 | ◐ float, int32 | ◐ float, int32 | uint32/int64/uint64 `add` wrappers exist; Accelerate complex via strided `vDSP_sve` |
| `Mean` | ✅ r+c | ◐ real | ◐ float, int32 | ◐ | ◐ | |
| `MeanSquare`, `RootMeanSquare` | ✅ (via L2 norm) | ✅ | ◐ float | ◐ float | ◐ float, int32 | AVX/SSE int32 needs `mullo` |
| `Product` | — | — | ◐ float | ◐ float | ◐ float, int32 | no library primitive; AVX/SSE int32 needs `mullo` |
| `Max`, `Min` | ◐ real | ◐ real | ◐ float, int32 | ◐ | ◐ | uint32 `max/min` wrappers exist, unused |
| `MaxAbs`, `MinAbs` | ◐ real | ❌ | ✅ float, (u)int32 | ✅ | ✅ | `vDSP_maxmgv/minmgv[D]` |

### Complex and type conversion

| Expression | IPP | Accelerate | SIMD | Notes |
|---|---|---|---|---|
| `Real`, `Imag` | ❌ | ✅ | — | `ippsReal_32fc/64fc`, `ippsImag_…` (or `ippsCplxToReal`) |
| `RealToComplex` | ❌ | ✅ | — | `ippsRealToCplx` accepts a null imaginary pointer |
| `RealImagToComplex` | ✅ | ✅ | — | |
| `Conjugate`, `Angle`, `PowerSpectrum` | ✅ | ✅ | — | |
| `BinaryCaseDistinction` | — | — | AVX ✅, Neon ✅, **SSE ❌** | SSE register lacks `compare`/`bitwiseBlend` (`_mm_blendv_ps` is SSE4.1) |
| `TransformedByStaticCast` | pass-through | pass-through | ❌ | int32 ↔ float `convertToFp`/`convertToInt` wrappers exist |
| `TransformedBy`, `TransformedByDynamicCast` | — | — | — | arbitrary callables |
| `Forward`, filter expressions | ✅ | ✅ | ✅ | pass-through |

## 3. Restrictions that are sensible

These aren't worth implementing, or only with a disproportionate design change:

- **Arbitrary callables** (`TransformedBy`, `TransformedByDynamicCast`): no general way to vectorise user code.
- **Complex SIMD.** The register types only wrap real scalars. Complex SIMD (interleaved shuffles for mul/div, sign
  masks for conjugate) would need new register types (`AVXRegister<std::complex<float>>`) and touches every
  expression's requires clauses. That's an architectural project, not a gap fill. IPP and Accelerate already cover
  the common complex ops.
- **64-bit integer `mul`, `abs`, `min`, `max`, `compare` on AVX2/SSE.** There are no instructions for these before
  AVX-512, and emulating them costs more than the scalar loop. Neon also has no int64 `mul`.
- **Library ops for `Cube` and `Product`.** Neither IPP nor Accelerate has these primitives. Chaining `mul` twice would
  need a temporary buffer whenever the source is an expression (the source pointer can be `dst`), and the library
  can't allocate on the audio thread. SIMD already covers both.
- **Accelerate for `PowSingleExponent` / `PowConstantExponent`.** vForce has no scalar-exponent `pow`, so it would need
  an exponent buffer of length n. Use SIMD (`vpowf` with a broadcast) instead.
- **IPP `DivideSingleByVec` for double.** `ippsDivCRev` only exists for 32f.
- **`FastExp2` double on AVX/SSE.** The double → int64 conversion needs AVX-512. Neon handles it.
- **`MultiplySubtract` on IPP.** No matching primitive.
- **Transcendentals on GCC/Clang outside Apple.** There is no vendor vector math library there. Closing this needs
  in-house polynomial approximations (accuracy and test work) or a dependency like SLEEF. The explicit `fast*`
  expressions are the existing answer. See phase 5.
- **Platform reductions need a container source** (`src.data()`). This is by design: a chained reduction would need
  a scratch buffer.

## 4. Latent bugs found during the analysis

These aren't gaps but should be fixed first. Each one needs a test that triggers it.

1. **`MultiplyAccumulateVectors::getAVX` / `MultiplySubtractVectors::getAVX` don't require `has::getAVX<SrcCType>`**
   (`MultiplyAccumulate.h:98`, `MultiplySubtract.h:98`). If `srcC` is a node without AVX, `has::getAVX` is still
   true and the body fails to compile.
2. **`MultiplyAccumulateVectors::getNeon` / `MultiplySubtractVectors::getNeon` have no element-type constraint**, but
   `NeonRegister` only has `fma`/`fms` for float/double. `vctr::multiplyAccumulate` on int32 would fail to compile on
   ARM. The tests only cover float/double.
3. **`PowVectors::getNeon` checks `suitableForAccelerateNeonOp<value_type, SrcBType, SrcBType>`** (`Pow.h:73`), so
   `SrcAType` is never checked.
4. **`AddVectors` / `SubtractVectors` AVX integer overloads require `isUnsigned`**. Signed `int32_t`/`int64_t`
   `a + b` falls back to SSE even though `AVXRegister<int32_t/int64_t>::add/sub` exist. The scalar-vector variants
   correctly use `isInt`. This looks like a copy-paste slip.
5. **Preference tags are inverted in `MeanSquare`, `RootMeanSquare`, `Log2`** (Accelerate preferred over IPP). This is
   probably deliberate, since the IPP versions are emulated via L2 norm or `ln · c`. Add a comment, or confirm.

## 5. Implementation plan

Each step follows `Expressions/AGENTS.md`: copy the closest existing expression, keep `requires` clauses mutually
exclusive, add tests with `filter <<` over `VCTR_ALL_EVALUATION_STRATEGIES` (or at least `PlatformVectorOps,
VCTR_NATIVE_SIMD`), use sizes that aren't multiples of the SIMD width, and do a per-path mutation check. Each phase is
a reasonable PR on its own. Within a phase, the steps are independent unless noted.

### Phase 0: fix latent bugs (small, do first)

- Fix bugs 1–4 from §4. Add int32 cases to the `MultiplyAccumulate` tests (Neon compile check). Add a signed-int case
  to the `Add`/`Subtract` tests if it's missing. Add a chained case where `srcC` is a non-SIMD expression.
- Decide on bug 5 and document it inline.

### Phase 1: register infrastructure (prerequisite for phases 2 and 3)

All in `SIMD/{AVX,SSE,Neon}/*Register.h`, inside the matching type specialisations:

| Wrapper | AVX | SSE | Neon |
|---|---|---|---|
| `sqrt` float/double | `_mm256_sqrt_ps/pd` (avx) | `_mm_sqrt_ps/pd` | `vsqrtq_f32/f64` (AArch64) |
| `mul` int32/uint32 | `_mm256_mullo_epi32` (avx2) | `_mm_mullo_epi32` (sse4.1) | exists |
| `compare`, `bitwiseBlend` float/double/int32 | float/double exist; int32 `cmpgt/cmpeq` + `blendv_epi8` | add all (`_mm_cmp*_ps`, `_mm_blendv_ps`) | exists |
| `asin`, `acos`, `atan` | svml `_mm256_asin_ps` … | svml + vf `vasinf` … | vf `vasinf` … |
| `copysign` (for SignedSqrt) | `andnot`/`and`/`or` with sign mask | same | `vbslq` |

Add a register-level test or exercise each wrapper through an expression test in phase 2.

### Phase 2: cheap SIMD gaps (biggest user-visible win, pure header work)

Priority order: Neon and AVX first, SSE in the same change when it's free.

1. **`Sqrt`** AVX/SSE/Neon (float/double); then **`SignedSqrt`** SIMD (`copysign (sqrt (abs x), x)`).
2. **`Clamp` (runtime)**: copy `ClampByConstant`'s SIMD block, broadcasting `lowerBound`/`upperBound` in
   `prepare*Evaluation`. Add int32 via `max/min`. Do the same int extension for `ClampByConstant`.
3. **`Map`, `MapFrom0To1`, `MapTo0To1`**: broadcast the precomputed constants, one `fma` (AVX/Neon) or `mul+add` (SSE).
4. **`NormalizeSum`**: broadcast `srcSum`, `div`.
5. **Neon for scalar/vector variants**: `AddSingleToVec`, `MultiplyVecBySingle`, `DivideVectors`,
   `DivideSingleByVec`, `DivideVecBySingle` (follow `SubtractVecFromSingle`'s Neon block).
6. **`MultiplyVectors` SSE** (float/double, plus int32 after phase 1).
7. **Integer SIMD using `mullo`**: `MultiplyVectors` AVX int32, `MultiplyVecBySingle/ByConstant` int, `Square` int
   (Neon int32 works today), `Cube` AVX/SSE int32, `Product`/`MeanSquare`/`RMS` int32 reductions.
8. **Integer completeness from existing wrappers**: `AddConstantToVec` int (all three archs), `Max/Min` uint32
   reductions, `Sum`/`Mean` uint32/int64/uint64. Check `ReductionExpression`'s horizontal-reduce helpers for these
   types first.
9. **`BinaryCaseDistinction` SSE** (needs the phase 1 SSE `compare`/`bitwiseBlend`).
10. **`MultiplyAccumulate`/`MultiplySubtract` SSE** as `mul` + `add`/`sub`. Results differ slightly from fused FMA, so
    test with an epsilon.
11. **`TransformedByStaticCast`** int32 ↔ float SIMD via `convertToFp`/`convertToInt` (optional, lowest priority).

### Phase 3: transcendental SIMD via existing wrappers (MSVC SVML, Apple vForce)

Pattern: `Exp.h` (`getAVX` with `hasSVML`, `getSSE` with `suitableForVfpOrSvmlSSERegisterFunction`, `getNeon` with
`platformApple && float`).

1. **`Ln`, `Log2`, `Log10`**: add all three paths. This also gives `magToDb` SIMD on Apple and MSVC.
2. **Trigonometric (12 expressions)**: replace the direct `vsinf (...)` calls with `Expression::SSE::sin` /
   `Expression::Neon::sin`, widen the SSE concept to `suitableForVfpOrSvmlSSERegisterFunction`, and add `getAVX` with
   `hasSVML`. `asin/acos/atan` need the phase 1 wrappers. This is mechanical across 12 files; do it in one PR.
3. **`PowVectors` AVX** (svml) and the SSE concept widening; **`PowSingleExponent`/`PowConstantExponent`** SIMD via a
   broadcast exponent; **`PowSingleBase`/`PowConstantBase`** SIMD via a broadcast base.

### Phase 4: platform-library gaps

| Step | IPP | Accelerate | Notes |
|---|---|---|---|
| `PowSingleBase`, `PowConstantBase` as `exp (x · ln b)` | `mulC` + `exp` | `vsmul` + `vvexp` | Replace the scalar placeholder loop. Document the precision difference vs `std::pow` and test with an epsilon. Fixes `dbToMag` |
| `PowConstantExponent` special cases | `exp == 2` → `mul (s, s)`, `0.5` → `sqrt` (compile-time branch) | same | optional |
| `Real`, `Imag`, `RealToComplex` | `ippsReal/Imag/RealToCplx` | ✅ | add wrappers to `IntelIPP.h` |
| `DivideSingleByVec` | — | `vDSP_svdiv[D]` | |
| `SubtractVectors`, `AddConstantToVec` complex | ✅ | widen concept, reuse existing complex wrappers | |
| `MultiplyVecBySingle/ByConstant`, `DivideVecBySingle` complex | ✅ | `vDSP_zvzsml[D]` (divide via reciprocal) | |
| `MaxAbs`, `MinAbs` | ✅ | `vDSP_maxmgv/minmgv[D]` | |
| `ElementwiseMax/Min` | `ippsMaxEvery/MinEvery` (verify non-in-place variant in the pinned IPP version) | `vDSP_vmax/vmin[D]` | needs a binary `...BinaryVectorOp` concept, like `AddVectors` |
| `MultiplyAccumulate` | `AddProduct` after evaluating `srcC` into `dst`, only if A and B are containers | `vDSP_vma[D]` | mirror `MultiplyVectors::evalVectorOpMultiplyAccumulate` |
| `Sum`/`Mean` complex | ✅ | strided `vDSP_sve`/`vDSP_meanv` on real and imag lanes | low priority |
| `Max`/`Min`/`Sum` int32 | `ippsMax_32s`, `ippsSum_32s_Sfs` | — | low priority; SIMD covers it |

Every new wrapper goes inside the matching `#if` type specialisation in `PlatformVectorOps/*`. Accelerate paths can't be
compiled or tested on this Linux machine. They need CI (macOS) or a Mac for the mutation check, so flag that in the PR.

### Phase 5 (optional, needs a design decision first)

- **Portable SIMD transcendentals for GCC/Clang on Linux/Windows without IPP**: exp/log/sin/cos approximations with
  documented accuracy (comparable to the `fast*` family, but accurate), or an optional SLEEF backend behind a CMake
  option like `VCTR_AUTOLINK_IPP`. This is the largest remaining performance gap on non-Apple, non-IPP setups, but it is
  a feature, not a gap fill.
- **Complex SIMD register types**: see §3. Only worth it if there's a concrete profile showing complex-heavy chains
  without IPP or Accelerate.

### Verification per phase

- `scripts/verify.sh` (build + test + headers), `-f "[<tag>]"` while iterating.
- Mutation check for every new path on this machine: AVX, SSE and IPP (the local IPP build exists). Neon and Accelerate
  paths need the macOS/ARM CI runners. Say so explicitly in each PR.
- After phase 2 and 3, add a few chained tests such as `sqrt << (a * a + b * b)` and `magToDb << abs << x` that only
  reach SIMD when every node has the path. They test the result, not the path, as `test/AGENTS.md` requires.
