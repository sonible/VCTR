#!/usr/bin/env python3
"""Normalises the LGPL copyright header at the top of VCTR C++ files.

Usage: fix_headers.py [--check] [--normalise-year] [files...]

Without files, all tracked *.h, *.hpp, *.tpp and *.cpp files (excluding ext/) are processed.
With --check, nothing is written and the exit code is 1 if any file would change.
With --normalise-year, the copyright line is rewritten to "Copyright <first year>- by sonible GmbH.",
otherwise it is kept verbatim (only its indentation is fixed).
"""

import datetime
import re
import subprocess
import sys

HEADER = """/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright {year}- by sonible GmbH.

    This file is part of VCTR - Versatile Container Templates Reconceptualized.

    VCTR is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    VCTR is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License version 3 for more details.

    You should have received a copy of the GNU Lesser General Public License
    version 3 along with VCTR.  If not, see <https://www.gnu.org/licenses/>.
  ==============================================================================
*/
"""

MARKER = "COPYRIGHT NOTICES"
COPYRIGHT_LINE = re.compile (r"^[ \t]*Copyright[ \t]+(\d{4})(.*?)[ \t]*$", re.MULTILINE)


def fixed_content (text: str, normalise_year: bool = False) -> str:
    text = text.lstrip ("﻿").replace ("\r\n", "\n")
    stripped = text.lstrip()
    header = HEADER.format (year=datetime.date.today().year)
    body = stripped

    if stripped.startswith ("/*"):
        end = stripped.find ("*/")
        block = stripped[: end + 2] if end != -1 else ""

        if MARKER in block:
            header = HEADER.format (year="YEAR")
            match = COPYRIGHT_LINE.search (block)
            year, rest = (match.group (1), match.group (2)) if match else (str (datetime.date.today().year), "- by sonible GmbH.")

            # The copyright notice itself is kept verbatim unless explicitly asked to normalise it
            if normalise_year:
                rest = "- by sonible GmbH."

            header = header.replace ("YEAR- by sonible GmbH.", year + rest)
            body = stripped[end + 2 :]

    body = re.sub (r"\A(?:[ \t]*\n)+", "", body)
    return header + "\n" + body


def tracked_files() -> list[str]:
    out = subprocess.run (["git", "ls-files", "*.h", "*.hpp", "*.tpp", "*.cpp"],
                          capture_output=True, text=True, check=True).stdout
    return [f for f in out.splitlines() if not f.startswith ("ext/")]


def main() -> int:
    args = sys.argv[1:]
    check = "--check" in args
    normalise_year = "--normalise-year" in args
    files = [a for a in args if not a.startswith ("--")] or tracked_files()
    changed = []

    for path in files:
        with open (path, encoding="utf-8", newline="") as f:
            original = f.read()

        new = fixed_content (original, normalise_year)

        if new != original:
            changed.append (path)
            if not check:
                with open (path, "w", encoding="utf-8", newline="\n") as f:
                    f.write (new)

    for path in changed:
        print (("would fix: " if check else "fixed: ") + path)

    return 1 if (check and changed) else 0


if __name__ == "__main__":
    sys.exit (main())
