# VCTR

Header-only C++20 library (sonible) wrapping `std::vector` / `std::array` / `std::span` as `vctr::Vector` / `vctr::Array` /
`vctr::Span`, plus lazy, SIMD- and platform-lib-accelerated **expression templates** (`vctr::exp << (a + b)`).
Used in real-time audio: no hidden heap allocations, no intermediate temporaries.

## Build & test

```bash
git submodule update --init --recursive
cmake . -B build -D CMAKE_PROJECT_TOP_LEVEL_INCLUDES=ext/cmake-conan/conan_provider.cmake -D VCTR_BUILD_TEST=1
cmake --build build --target vctr_test
./build/test/TestMain/vctr_test                # all tests
./build/test/TestMain/vctr_test "[abs]"        # Catch2 tag filter
```

Tests need conan (fetches Catch2 + gcem). Other options: `VCTR_BUILD_DEMO`, `VCTR_BUILD_BENCHMARK`, `VCTR_AUTOLINK_IPP`.
CI (`.github/workflows/UnitTests.yml`) builds with Clang 15+, GCC 13+, MSVC/clang-cl, with and without IPP — code must
compile on all of them, so don't rely on one compiler's leniency.

### Verify script (preferred for agents)

`scripts/verify.sh` wraps the build, test and header steps with bounded console output. Full logs go to
`<build dir>/verify/<step>.log`; on failure you get the error lines and the tail of the log. Prefer it over running
cmake and the test binary directly, so the context stays small.

```bash
scripts/verify.sh                          # build + test + headers (default)
scripts/verify.sh -s test -f "[abs]"       # only tests matching a Catch2 spec
scripts/verify.sh -s all                   # configure, build, test, docs, headers
scripts/verify.sh -s docs                  # Doxygen; prints the warning count
scripts/verify.sh -b build-release -t 80   # other build dir, 80 tail lines on failure
```

- `-s` steps (comma-separated): `configure`, `build`, `test`, `docs`, `headers`, or `all`. Steps run in order and stop at
  the first failure (exit 1; exit 2 for usage errors).
- `configure` runs automatically when the build directory has no `CMakeCache.txt`.
- Environment overrides: `VCTR_VERIFY_STEPS`, `VCTR_BUILD_DIR`, `VCTR_TEST_FILTER`, `VCTR_TAIL_LINES`, `CMAKE_ARGS`
  (extra configure arguments, e.g. `-G Ninja`), `DOXYGEN` (path to the doxygen binary). Command-line flags win.
- `docs` needs `doxygen` on the PATH (CI uses 1.9.6). It is not in the default steps.
- `scripts/verify.sh -h` prints the same usage.

## Layout

| Path | Contents |
|---|---|
| `include/vctr/vctr.h` | Umbrella header. **Every new header must be added here** (order matters, it is the only include point). |
| `include/vctr/Containers/` | `VctrBase` (shared member functions, `.tpp`), `Vector`, `Array`, `Span` |
| `include/vctr/Expressions/` | Expression templates by category. See [Expressions/AGENTS.md](include/vctr/Expressions/AGENTS.md) before adding/changing one. |
| `include/vctr/SIMD/` | `AVXRegister`, `SSERegister`, `NeonRegister` wrappers (per element type specialisations) |
| `include/vctr/PlatformVectorOps/` | Thin `AppleAccelerate<T>` / `IntelIPP<T>` wrappers, see its `Readme.md` |
| `include/vctr/TypeTraitsAndConcepts/` | `vctr::is::*`, `vctr::has::*`, `vctr::are::*` concepts, incl. `is::suitableFor...VectorOp` |
| `include/vctr/Miscellaneous/Config.h` | Platform flags (`archX64`, `archARM`, `platformApple`, `hasIPP`), `VCTR_FORCEDINLINE`, `VCTR_TARGET`, `VCTR_ASSERT` |
| `include/vctr_utils/` | Optional helpers for tests/demo/benchmark, not core |
| `test/` | Catch2 tests, see [test/AGENTS.md](test/AGENTS.md) |

## Conventions

- `snake_case` headers are user-includable; `CamelCase` headers are implementation details that only work via `vctr.h`
  (they have no includes of their own).
- Member functions: `camelCase`, except forwarded std members which keep `snake_case` (`size()`, `push_back()`).
- Style: `.clang-format` (Allman braces, space before call parentheses: `foo (x)`, `std::abs (v)`, but `size()` with no args).
- Every file starts with the LGPL copyright header (at byte 0, then one blank line). After creating or editing C++
  files run `python3 .claude/skills/fix-file-headers/fix_headers.py <files>` (adds/repairs it; `--check` to only verify).
- Public API gets a Doxygen `/** */` comment; expressions also need `@ingroup Expressions`.
- Prefer `constexpr`; operations must be usable in compile-time contexts where possible (gcem for math when
  `std::is_constant_evaluated()`).
- `extent` = compile-time size or `std::dynamic_extent` (same meaning as in `std::span`).

## Testing

New functionality and bug fixes must come with tests (see [test/AGENTS.md](test/AGENTS.md)). Where possible, prove the
tests actually catch bugs with a mutation check: temporarily break the implementation, confirm the relevant test fails,
then revert. A test that still passes against broken code is not done.
