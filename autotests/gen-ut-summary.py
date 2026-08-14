#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""Generate ut-summary.json from gtest XML reports and lcov coverage data.

Usage:
  Called from shell test scripts. Reads environment variables:
    - projectdir:   project root directory
    - builddir:     build directory name (relative to projectdir)
    - reportdir:    report output directory name (relative to projectdir)

  Optional overrides (take precedence over defaults):
    - GTEST_XML_DIR:    directory containing gtest XML reports
    - COVERAGE_INFO:    path to lcov .info file for coverage parsing

  Output: {projectdir}/{reportdir}/ut-summary.json
"""

import json
import xml.etree.ElementTree as ET
import glob
import os
import subprocess
import re
import sys


def parse_gtest_xml(xml_dir):
    """Parse Google Test / JUnit XML output and return (total, passed, failed)."""
    total = passed = failed = 0
    for xml_file in sorted(glob.glob(os.path.join(xml_dir, "*.xml"))):
        try:
            root = ET.parse(xml_file).getroot()
            # JUnit format uses <testsuites> as root with aggregated counts
            # gtest XML uses <testsuites> or <testsuite> as root
            t = int(root.get("tests", 0))
            f_count = int(root.get("failures", 0))
            err_count = int(root.get("errors", 0))
            total += t
            failed += f_count + err_count
            passed += t - f_count - err_count
        except Exception as e:
            print(f"Warning: failed to parse {xml_file}: {e}", file=sys.stderr)
    return total, passed, failed


def parse_lcov_summary(coverage_info):
    """Parse lcov --summary output and return coverage dict."""
    result = {}
    if not os.path.exists(coverage_info):
        print(f"Warning: coverage info file not found: {coverage_info}", file=sys.stderr)
        return result

    lcov_out = subprocess.run(
        ["lcov", "--summary", coverage_info, "--rc", "lcov_branch_coverage=1"],
        capture_output=True, text=True
    )
    summary_text = lcov_out.stdout + lcov_out.stderr

    # Parse lines:  "lines......: XX.X% (NNN of MMMM lines)"
    m_lines = re.search(r'lines.*?:\s*([\d.]+)%\s*\((\d+)\s+of\s+(\d+)\s+\w+\)', summary_text)
    if m_lines:
        pct, hit, total = m_lines.groups()
        result["line_coverage"] = {
            "total": int(total),
            "passed": int(hit),
            "failed": int(total) - int(hit),
            "coverage": f"{float(pct):.2f}%"
        }

    # Parse functions: "functions..: XX.X% (NNN of MMMM functions)"
    m_func = re.search(r'functions.*?:\s*([\d.]+)%\s*\((\d+)\s+of\s+(\d+)\s+\w+\)', summary_text)
    if m_func:
        pct, hit, total = m_func.groups()
        result["function_coverage"] = {
            "total": int(total),
            "passed": int(hit),
            "failed": int(total) - int(hit),
            "coverage": f"{float(pct):.2f}%"
        }

    return result


def main():
    projectdir = os.environ.get("projectdir")
    builddir = os.environ.get("builddir")
    reportdir = os.environ.get("reportdir")

    if not all([projectdir, builddir, reportdir]):
        print("Error: environment variables projectdir, builddir, reportdir are required", file=sys.stderr)
        sys.exit(1)

    # Resolve paths (env overrides take precedence)
    gtest_dir = os.environ.get("GTEST_XML_DIR",
                                os.path.join(projectdir, builddir, "report"))
    coverage_info = os.environ.get("COVERAGE_INFO",
                                    os.path.join(projectdir, builddir, "coverage.info"))

    # --- Test case counts from gtest XML ---
    total, passed, failed = parse_gtest_xml(gtest_dir)

    result = {
        "test_cases": {
            "total": total,
            "passed": passed,
            "failed": failed
        }
    }

    # --- Coverage from lcov --summary ---
    coverage_data = parse_lcov_summary(coverage_info)
    result.update(coverage_data)

    # --- Write output ---
    output_path = os.path.join(projectdir, reportdir, "ut-summary.json")
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w") as f:
        json.dump(result, f, indent=2)

    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
