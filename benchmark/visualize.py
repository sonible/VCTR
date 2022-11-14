"""
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2022- by sonible GmbH.

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
"""

"""
Benchmarks must be structured as follows:

 TEST_CASE ("Name of testcase) // each test case will lead to a separate plot
 {
    auto i = GENERATE;

    SECTION (std::to_strin g(i)) # these will be plotted on the x axis.
    {
        // These benchmarkes will be plotted in the same figure
        BENCHMARK ("Something1") { return something1 (i); }
        BENCHMARK ("Something2") { return something2 (i); }
    }
 }
"""

import argparse
import pathlib
import xmltodict
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import rc


# ====================================================================
# Configuration
class Colours:
    green = "#00C79C"
    black = "#262626"
    orange = "#FF6D4A"
    blue = "#6989C4"
    red = "#FF4554"


class Config:
    marker = 'o'
    markersize = 3
    linewidth = 2
    capsize = 3
    capthick = 2
    background_colour = Colours.black
    palette = [Colours.orange, Colours.blue, Colours.green, Colours.red]
    time_base = "ns"  # s, ms, mus, ns
    suffix = ".pdf"  # determines the output format


if Config.background_colour == Colours.black:
    plt.style.use('dark_background')
    plt.rcParams['axes.facecolor'] = Config.background_colour

rc('font', **{'family': 'sans-serif', 'sans-serif': ['Helvetica']})


# ====================================================================
def from_ms(v, new_base: str):
    if new_base == "s":
        return v * 1e-9
    elif new_base == "ms":
        return v * 1e-9 * 1e3
    elif new_base == "mus":
        return v * 1e-9 * 1e6
    elif new_base == "ns":
        return v

    assert (False)


def parse_xml(path) -> dict:
    with pathlib.Path(path).open('r') as f:
        testcases = xmltodict.parse(f.read())["Catch2TestRun"]["TestCase"]
    results = dict()

    if type(testcases) == dict:
        testcases = [testcases]

    for testcase in testcases:
        testcase_name = testcase["@name"]

        print("========================================")
        print("Testcase:", testcase_name)

        results[testcase_name] = dict()

        for section in testcase["Section"]:
            print("  Section:", section["@name"])
            section_name = section["@name"]

            benchmark_results = section["BenchmarkResults"]
            if type(benchmark_results) == dict:
                benchmark_results = [benchmark_results]

            for benchmark in benchmark_results:
                benchmark_name = benchmark["@name"]

                if benchmark_name not in results[testcase_name].keys():
                    results[testcase_name][benchmark_name] = dict()
                    results[testcase_name][benchmark_name]["mean"] = []
                    results[testcase_name][benchmark_name]["std"] = []
                    results[testcase_name][benchmark_name]["xdata"] = []

                mean = float(benchmark["mean"]["@value"])
                std = float(benchmark["standardDeviation"]["@value"])

                print("    Benchmark", benchmark_name)
                print("      section:", section_name)
                print("      samples:", benchmark["@samples"])
                print("      mean:   ", mean, "ns")
                print("      std:    ", std, "ns")

                results[testcase_name][benchmark_name]["mean"].append(mean)
                results[testcase_name][benchmark_name]["std"].append(std)
                results[testcase_name][benchmark_name]["xdata"].append(section_name)

    return results


# ====================================================================
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plots the benchmark statistics of a given Catch2 XML file.")
    parser.add_argument("file",
                        help="The benchmark XML file. To create such a file, call catch with '--reporter XML::out=filename.xml'")
    args = parser.parse_args()

    results = parse_xml(args.file)

    for testcase_name, testcase in results.items():
        fig = plt.figure(testcase_name)

        for i, (benchmark_name, benchmark) in enumerate(testcase.items()):
            ydata = from_ms(np.array(benchmark["mean"]), Config.time_base)
            error = from_ms(np.array(benchmark["std"]), Config.time_base)

            assert (i < len(Config.palette))

            plt.errorbar(range(len(ydata)),
                         ydata,
                         yerr=error,
                         marker=Config.marker,
                         markersize=Config.markersize,
                         linewidth=Config.linewidth,
                         capsize=Config.capsize,
                         capthick=Config.capthick,
                         color=Config.palette[i],
                         label=benchmark_name)

        plt.title(testcase_name, fontweight="bold")
        plt.legend()
        plt.grid()
        plt.xticks(ticks=range(len(ydata)), labels=benchmark["xdata"])
        plt.xlabel("Vector size")
        time_units = {"s": "$s$", "ms": r"$ms$", "mus": r"$\mu s$", "ns": r"$ns$"}
        plt.ylabel("Time in " + time_units[Config.time_base] + ".")

        filename = testcase_name + Config.suffix
        plt.savefig(filename, facecolor=Config.background_colour, edgecolor='none')
        print("Wrote", filename)

    # plt.show()
