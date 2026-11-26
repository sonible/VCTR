# Unit-test workflow restructuring plan

## Goal

Separate compilation from test execution while retaining the existing compiler and platform coverage. Reuse the compiled test binaries in a native execution job and, for nightly or manually requested runs, an Intel SDE job that validates runtime CPU-feature dispatch on emulated x86-64 systems.

Keep regular push and pull-request feedback fast. SDE coverage must not run for every regular build.

## Recommended workflow structure

Use one primary workflow with dependent jobs:

1. `build_matrix`
2. `native_tests`
3. `sde_tests`

Keep the jobs in `.github/workflows/UnitTests.yml` initially. A reusable `.github/workflows/SDETests.yml` can be extracted later if the SDE setup becomes large enough to justify it.

Avoid coordinating independent workflows through `workflow_run`. It makes pull-request status handling, fork security, commit association, and artifact selection unnecessarily complex.

## Phase 1: Split build and native execution

### Build matrix

Preserve the existing build combinations:

- Linux: Clang and GCC, with and without IPP.
- Windows: MSVC, clang-cl, and Clang, with and without IPP.
- macOS: ARM without IPP and Intel with and without IPP.

Continue to:

- configure CMake with `VCTR_BUILD_TEST=1`;
- build `vctr_test`;
- treat compiler warnings as errors;
- use the existing Conan profiles and IPP setup.

Replace the in-place `Run test` step with artifact publication.

### Artifact contents

Each build uploads:

- the `vctr_test` executable;
- test runtime data, if any is added;
- shared/runtime libraries required by the executable;
- architecture and build metadata needed to reproduce the invocation;
- a short metadata file recording OS, architecture, compiler, IPP mode, commit SHA, and executable path.

Artifact names must be deterministic and unique, for example:

```text
vctr-tests-${runner}-${compiler}-${ipp}
```

Use a short retention period for regular pull-request artifacts and only retain the selected binaries needed by scheduled SDE jobs.

Before relying on artifacts, verify on a clean runner that the downloaded executable runs without rebuilding or depending on absolute paths from the producer workspace.

### Native test matrix

`native_tests` depends on `build_matrix` and downloads each corresponding artifact.

On every push and pull request:

- execute the same `vctr_test` invocation currently used;
- preserve the original job's required status semantics;
- include the source build combination in the display name and job summary;
- upload logs or Catch2 reports on failure.

Prefer one native execution job per build artifact. This keeps failures attributable to a specific compiler/platform configuration.

## Phase 2: Add event controls for SDE

Add:

```yaml
on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [develop]
  schedule:
    - cron: '0 2 * * *'
  workflow_dispatch:
    inputs:
      run_sde:
        description: Run Intel SDE CPU-model tests
        required: false
        type: boolean
        default: false
```

The SDE job condition should be equivalent to:

```yaml
if: github.event_name == 'schedule' || inputs.run_sde == true
```

Push and pull-request runs therefore build and execute native tests as before, but skip SDE.

## Phase 3: Add the SDE execution matrix

### Initial artifact selection

Start with a small, representative set rather than every build configuration:

- Windows x86-64, MSVC, no IPP.
- Windows x86-64, clang-cl, no IPP.
- Linux x86-64, Clang or GCC, no IPP.

Exclude IPP initially. IPP performs its own platform dispatch and makes SDE failures harder to attribute to VCTR.

macOS remains native-only until Intel SDE installation and process authorization are proven reliable on GitHub-hosted Intel macOS runners. Do not make the nightly workflow depend on that availability.

### CPU models

Run the same downloaded binary with:

| SDE option | Purpose |
| --- | --- |
| `-p4p` / `-p4` | No AVX/FMA/AVX2 baseline |
| `-mrm` | Merom-class x86-64 baseline |
| `-nhm` | Nehalem-class baseline with newer SSE, still no AVX |
| `-snb` | AVX without FMA/AVX2 |
| `-hsw` | AVX, FMA, and AVX2 baseline |

Confirm supported option names against the pinned SDE version before implementation.

Expected command shape:

```text
<sde executable> <cpu option> -- <vctr_test executable>
```

Use PowerShell on Windows and a POSIX shell on Linux so paths and executable handling remain explicit.

### Setup and security

- Pin an SDE setup action or download helper to a full commit SHA.
- Pin and verify the expected SDE version and artifact checksums where available.
- Record Intel SDE license acceptance as a prerequisite.
- Avoid third-party mirroring unless its provenance and checksum verification are acceptable.
- Verify downloaded binaries before adding them to `PATH`.

### Execution strategy

Keep SDE as a matrix with dimensions:

```yaml
artifact: [windows-msvc-no-ipp, windows-clang-no-ipp, linux-clang-no-ipp]
cpu: [-p4p, -mrm, -nhm, -snb, -hsw]
```

Set `fail-fast: false` so one unsupported model or platform does not hide the other results.

Start with the focused test filter for CPU detection:

```text
vctr_test "[cpuDetection]"
```

Then run the complete VCTR suite under each model. If runtime becomes excessive:

- retain full nightly coverage for one Linux binary;
- run CPU-detection plus SIMD/reduction tags on the other compiler combinations;
- record this reduction explicitly rather than silently weakening coverage.

## Phase 4: Add diagnostics and assertions

Enhance SDE runs to print:

- SDE version;
- emulated CPU model;
- VCTR's detected `sse4_1`, `avx`, `fma`, and `avx2` flags;
- operating system and binary metadata.

For `-snb`, assert at least that AVX is enabled and AVX2/FMA are disabled. For pre-AVX models, assert that all AVX-family flags are disabled. Prefer a small test utility or an optional Catch2 verbose message over parsing library internals in workflow scripts.

Do not rely on SDE alone to test an OS that reports AVX CPU support while withholding XCR0 state. Keep deterministic unit tests for combinations of:

- CPU AVX supported / OS AVX supported;
- CPU AVX supported / OS AVX unsupported;
- CPU AVX unsupported / OS AVX supported;
- AVX2 and FMA leaking through while effective AVX is false.

## Phase 5: Validate the migration

1. Run the existing workflow unchanged and capture expected native results.
2. Introduce the build artifacts but continue native execution in the same workflow; compare pass counts and failure behavior.
3. Move native execution into `native_tests` and confirm each artifact is executable on a clean runner.
4. Add `workflow_dispatch` with SDE disabled by default.
5. Manually run one Linux SDE model, then the complete Linux SDE matrix.
6. Add MSVC and clang-cl Windows SDE coverage.
7. Enable scheduled SDE only after all models are stable.
8. Make the native test job required in branch protection; do not initially make every SDE matrix leg required for pull requests.

## Acceptance criteria

- Push and pull-request builds retain all current compiler, platform, and IPP combinations.
- Native test jobs execute downloaded artifacts rather than binaries left in the build workspace.
- Artifacts run on clean runners without source checkout dependencies other than the explicitly retained test data.
- Regular pull-request and push runs do not start SDE.
- A manual dispatch can opt into SDE.
- Nightly runs exercise at least Windows MSVC, Windows clang-cl, and Linux x86-64.
- Pre-AVX models do not enter AVX dispatch paths.
- Sandy Bridge reports AVX but not FMA or AVX2.
- Haswell reports all three AVX-family features and executes the accelerated paths without failure.
- Failures identify the originating artifact, compiler, OS, and SDE CPU model.
- Intel SDE version and setup source are pinned and checksum-verified.
- Native macOS coverage remains unchanged.

## Deferred improvements

- Extract SDE setup and execution into a reusable workflow if the primary workflow becomes difficult to read.
- Add AVX-512 CPU models only if VCTR adds AVX-512 dispatch.
- Add IPP-under-SDE coverage after non-IPP execution is stable.
- Add benchmark-oriented SDE reports separately from correctness tests.
