---
name: vctr-cpp-review
description: Review VCTR C++ changes against the sonible C++ code style and VCTR's own conventions (expression templates, SIMD/platform paths, constexpr, tests) and give a merge recommendation (APPROVE, REQUEST CHANGES, or NEEDS DISCUSSION). Use when asked to review, code-review, or critique C++ files, a folder, a branch, or a diff, or to decide whether C++ code is ready to merge.
---

# sonible C++ Review

Read-only review of VCTR C++ code for bugs, poor design, and deviations from the sonible C++ code style and the VCTR
conventions. It ends with a prioritized summary and a merge recommendation.

Rule sources, in order of precedence:

1. The repository's own instructions: `AGENTS.md`, `include/vctr/Expressions/AGENTS.md`, `test/AGENTS.md`, and the
   checked-in `.clang-format`. Do not report deviations those files explicitly allow.
2. [references/checklist.md](references/checklist.md), the condensed sonible C++ code style plus VCTR-specific checks.

Do not modify files, commit, or post comments unless the user asks.

## 1. Determine scope

1. **User named files or folders**: review those only. Expand folders to C++ sources (`.h`, `.tpp`, `.cpp`) and skip
   `build*/`, `ext/` (third-party submodules) and generated code. Review whole files; there is no diff.
2. **Nothing named**: ask the user (with the `AskUserQuestion` tool if available) whether to continue with the current
   branch's diff from `develop`. Do not start reviewing before they answer.
   - **Yes**: use the diff described below.
   - **No**: ask which files or folders to review.
   - If there is no `develop` branch (`git rev-parse --verify develop`, then `origin/develop`), say so and ask for a
     base branch or files.

**Branch diff**: base = `git merge-base <develop> HEAD`. Review `git diff <base>` (committed, staged, and unstaged
changes) plus untracked files from `git ls-files --others --exclude-standard`, limited to C++ sources and the CMake
files that register them. If the branch is `develop` itself or the diff is empty, say so and ask for files. Use
`git log <base>..HEAD` for intent.

If the scope exceeds roughly 20 files or 2000 changed lines, tell the user and propose reviewing it in slices before
continuing.

## 2. Gather context

- Read the full changed files, not only hunks; open the base classes, concepts, SIMD register wrappers and platform
  wrappers (`include/vctr/SIMD/`, `include/vctr/PlatformVectorOps/`) the change calls into, and the matching tests.
- For a new or changed expression, open the closest existing expression of the same kind (see the table in
  `include/vctr/Expressions/AGENTS.md`) and compare structure.
- Read [references/checklist.md](references/checklist.md). The `EXPR` section applies to everything under
  `include/vctr/Expressions/`; the `TEST` section to `test/`.
- In diff mode, report issues on changed lines or caused by them. Mention pre-existing problems only if the change
  makes them worse or depends on them.
- Checks to run, each only if cheap; otherwise say they were not run:
  - `clang-format --dry-run -Werror -style=file <files>` on the reviewed files. Never apply it.
  - `python3 .claude/skills/fix-file-headers/fix_headers.py --check <files>`.
  - `scripts/verify.sh` (build + tests + header check) or `scripts/verify.sh -s test -f "[tag]"` for the affected
    tests. The build is only one compiler on one architecture: code paths for other architectures (AVX/SSE vs Neon),
    Apple Accelerate or IPP that this machine does not compile must be reviewed by reading, and the summary says so.

## 3. Review passes

Work through these in order and keep notes per finding.

1. **Bugs and poor design**: "Can you find any bug or poor design choice here?" Trace lifetimes of sources held by
   expressions, every evaluation path (scalar `operator[]`, SIMD, platform vector op, constexpr), alignment and tail
   handling, numerical edge cases, and missing tests using the checklist categories. Read the code as each supported
   compiler would.
2. **Strongest argument against merging**: state it as the best case a skeptical maintainer would make, then check
   whether it holds. If it holds, it is a finding; if not, drop it.
3. **User perspective**: "Can this be more intuitive?" Consider a library user writing `vctr::exp << (a + b)` or
   calling a container member: naming consistent with existing expressions and `std` counterparts, hard-to-misuse
   signatures, compile errors that point at the actual mistake (concepts over deep template failures), and documented
   numerical caveats.
4. **Simplification**: "Can this become less convoluted?" Look for needless abstraction, duplicated logic, code that
   an existing macro, concept or helper already covers (`VCTR_COMMON_*_EXPRESSION_MEMBERS`, `is::suitableFor...`,
   `Expression::CommonElement`), and accelerated paths that add complexity without a measurable benefit.

Then check the remaining checklist items (style, naming, layout, documentation, headers) that the passes did not
already cover.

## 4. Calibrate severity

- **Must fix**: likely or provable defect or regression: dangling references, UB, wrong results on one evaluation path,
  a hidden heap allocation where VCTR promises none, code that fails to compile on a CI compiler or platform, a
  missing include in `vctr.h`, tests that are not registered, or edits to `ext/`.
- **Should fix**: real problems that can follow in a separate change: missing regression tests or unverified evaluation
  paths, unclear contracts, avoidable complexity, guideline deviations that affect maintainability.
- **Nitpick**: style and preference. Group formatter-detectable issues into one item.

Only report what you can point to in the code, with a concrete reason. Do not pad the list; when unsure, state the
uncertainty and the question to ask instead of asserting a defect. Never invent behavior or call sites that you did
not read.

## 5. Output format

Start with one line stating the scope (files or `develop...HEAD`, with the number of files) and what was or was not
run (formatter, header check, build, tests, and which evaluation paths could not be compiled here).

Then, in this order:

**Answers to the four questions** (brief, a few lines each):

1. Bugs or poor design
2. Strongest argument against merging as-is
3. Intuitiveness
4. Simplification

**Summary**

```markdown
### Must fix
- `include/vctr/Expressions/BasicMath/Foo.h:42` [EXPR-3] `getAVX` is defined but there is no matching
  `prepareAVXEvaluation`. Fails to compile once the AVX path is selected; add the forward macro.

### Should fix
- `test/TestCases/Expressions/Foo.cpp:10` [TEST-2] ...

### Nitpicks
- `include/vctr/Expressions/BasicMath/Foo.h:7` [NAME-1] ...
```

Use the checklist ID when one applies, omit it for findings outside the checklist. Write `None.` for an empty section.
Keep each finding to one or two sentences: location, problem, impact, suggested fix.

**Recommendation**: end with exactly one of the following and a one-sentence reason.

- **APPROVE**: no Must fix items; Should fix items are acceptable as follow-ups.
- **REQUEST CHANGES**: one or more Must fix items.
- **NEEDS DISCUSSION**: the outcome depends on an open design, API or guideline-conflict question that the author or a
  maintainer must settle. State the question.

Offer to fix findings afterwards, but wait for the user to ask.
