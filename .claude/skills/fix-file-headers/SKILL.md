---
name: fix-file-headers
description: Check and fix the LGPL copyright file header at the top of VCTR C++ files (.h, .hpp, .tpp, .cpp) — leading spaces or blank lines before it, mangled indentation, inconsistent "Copyright YYYY- by" line, or a missing header in new files. Use after creating or editing C++ files, or when asked to fix, check or normalise file headers.
---

# Fix VCTR file headers

Every C++ file must start (byte 0, no BOM, no leading whitespace or blank lines) with the canonical header defined in
`fix_headers.py`, followed by exactly one blank line and then the code.

## Workflow

1. Check (run from the repo root, exit code 1 if anything is off):
   ```bash
   python3 .claude/skills/fix-file-headers/fix_headers.py --check
   ```
   Pass file paths to limit it to specific files (e.g. only those you just created or touched).
2. Fix: run the same command without `--check`.
3. Review `git diff` — only the lines up to and including the blank line after `*/` may change.

## What the script does

- Strips a BOM, CRLF line endings and any whitespace/blank lines before the header.
- Re-writes the whole header block from the template, so indentation and `===` rule drift is repaired.
- Keeps the `Copyright ...` line verbatim (only its indentation is fixed). Variants like `2023 by`, `2023 - by` and
  `2023- by` all exist in the repo. Only pass `--normalise-year` (rewrites to `<first year>- by sonible GmbH.`) when the
  user explicitly asks for it, since it alters copyright notices. Never bump the year of an existing file.
- Files without the header (first block lacks `COPYRIGHT NOTICES`) get it prepended as `<current year>- by`.
  An existing leading `/** doc comment */` is kept below it.
- Ensures exactly one blank line between `*/` and the first code line.

Files under `ext/` (third-party submodules) are never touched.
