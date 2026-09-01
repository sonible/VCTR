# Conan profiles used by CI

Every entry of the CI build matrix in `.github/workflows/UnitTests.yml` selects a Conan profile
through its `conan_profile` key:

* `conan_profile: default` &ndash; let Conan auto-detect the profile (`conan profile detect`).
  This is what we use on Linux and macOS, where auto-detection is reliable.
* `conan_profile: <name>` &ndash; use `.github/conan/profiles/<name>` instead.

The profile is passed to [cmake-conan](https://github.com/conan-io/cmake-conan) via the
`CONAN_HOST_PROFILE` / `CONAN_BUILD_PROFILE` CMake cache variables. The host profile is always
combined with cmake-conan's `auto-cmake` profile, which is appended last and injects the compiler
executables detected by CMake.

## Why custom profiles are needed on Windows

Conan's auto-detection guesses the Visual Studio installation from the compiler it finds and gets
this wrong on the GitHub runner images (it assumed VS 17 while the image ships VS 18), which makes
the `CMakeToolchain`/`VCVars` generator fail with
`VS non-existing installation: Visual Studio 17`.

The Windows profiles therefore pin the toolchain explicitly. To avoid hardcoding values that change
with the runner image, they are Jinja templates that read the values detected by the
`(Windows) Detect toolchain versions` workflow step:

| Environment variable | Meaning                                                | Example |
|----------------------|--------------------------------------------------------|---------|
| `VS_VERSION`         | Visual Studio major version (`tools.microsoft.msbuild:vs_version`) | `18`  |
| `VS_RUNTIME_VERSION` | MSVC toolset, used as `compiler.runtime_version` for clang | `v145`  |
| `VS_MSVC_VERSION`    | Conan `compiler.version` for `compiler=msvc`             | `195`   |
| `CLANG_VERSION`      | Conan `compiler.version` for `compiler=clang`            | `20`    |

Each variable has a sane fallback, so the profiles can also be used locally without those variables
being set.

## Available profiles

| Profile          | Used for                                                        |
|------------------|-----------------------------------------------------------------|
| `windows-clang`  | `clang-cl` (MSVC driver) and `clang++` (GNU driver) on Windows   |
| `windows-msvc`   | `cl` (MSVC) on Windows                                          |

