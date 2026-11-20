# Known issue: Windows CPU feature detection ignores OS support for AVX

## Problem

On Windows, `vctr::getSupportedCPUInstructionSets()` (`include/vctr/Miscellaneous/Config.h`, class `detail::X64InstructionSets`) reads raw CPUID bits only:

| Flag | Source |
| --- | --- |
| `sse4_1` | CPUID leaf 1, ECX[19] |
| `avx` | CPUID leaf 1, ECX[28] |
| `fma` | CPUID leaf 1, ECX[12] |
| `avx2` | CPUID leaf 7, EBX[5] |

A CPUID bit only says that the CPU implements an instruction set. The OS must also enable the extended register state, which software checks with `OSXSAVE` (CPUID.1:ECX[27]) and `XGETBV(0)`; for AVX, bits 1 (SSE) and 2 (YMM) of XCR0 must both be set. Without these checks, VCTR can pick AVX/FMA/AVX2 code paths where they cannot run. This happens in:

- VMs or hypervisors that mask AVX state but leave some feature bits set. Example: Proxmox/QEMU with a basic CPU model, or a host where FMA or leaf-7 bits leak through.
- Systems where the OS has disabled AVX.

The result is `STATUS_ILLEGAL_INSTRUCTION` at the first AVX instruction.

The dispatch makes this worse:

- `VctrBase::assignExpressionTemplate` (`Containers/VctrBase.h`, ~line 1050) and `ReductionExpression` (`Expressions/ReductionExpression.h`, ~line 74) choose the AVX path for real floats from `.fma` alone.
- Integer paths are chosen from `.avx2` alone.
- Neither checks `.avx`.

The non-Windows path uses `__builtin_cpu_supports`, which already accounts for OS support, so only Windows is affected.

Suspected (not proven) cause of intermittent crashes of `BandSplitProcessorTest` on a Windows CI runner on Proxmox with a basic CPU type.

## Suggested fix

1. In `CPUFeatureFinder`, also read `OSXSAVE` (leaf 1, ECX[27]). If it is set, call `_xgetbv (0)` from `<immintrin.h>`, which MSVC supports. Define `osSupportsAVX = osxsave && (xcr0 & 0x6) == 0x6`.
2. Derive the flags consistently:
   - `avx = cpuidAVX && osSupportsAVX`
   - `fma = cpuidFMA && avx`
   - `avx2 = cpuidAVX2 && avx`
   - Optionally, `avx512f` additionally needs XCR0 bits 5–7 (`0xE0`).
3. Make the dispatch gates explicit in `VctrBase.h` and `ReductionExpression.h`:
   - `.fma` becomes `.fma && .avx`.
   - `.avx2` becomes `.avx2 && .avx`, or a single `Config::hasAVXFMA` helper.

   This is defensive even after step 2.
4. Testing:
   - Run the unit tests under Intel SDE with pre-AVX CPUs (`sde -p4p`, `-mrm`, `-nhm`) and with `-snb` (AVX, no FMA/AVX2).
   - Add a small test that prints and checks the consistency of the detected flags, e.g. `fma` implies `avx` and `avx2` implies `avx`.
   - If possible, run once on a Proxmox `kvm64` / `x86-64-v2-AES` VM.
