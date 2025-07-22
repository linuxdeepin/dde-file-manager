#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
DDE File Manager Unit Test Report Generator (Modular Version)
功能：生成包含测试结果、覆盖率、性能等信息的综合HTML报表
"""

import argparse
import sys

from report_generator import TestReportGenerator


def main():
    """Main function"""
    parser = argparse.ArgumentParser(description="Generate DDE File Manager unit test report")
    parser.add_argument("--build-dir", required=True, help="Build directory path")
    parser.add_argument("--report-dir", required=True, help="Report output directory path")
    parser.add_argument("--project-root", required=True, help="Project root directory path")
    parser.add_argument("--test-passed", required=True, help="Whether tests passed (true/false)")
    parser.add_argument("--test-duration", type=int, required=True, help="Test duration in seconds")
    parser.add_argument("--coverage-success", required=True, help="Whether coverage generation succeeded (true/false)")
    parser.add_argument("--coverage-duration", type=int, required=True, help="Coverage generation duration in seconds")
    
    args = parser.parse_args()
    
    # Convert string parameters to boolean values
    test_passed = args.test_passed.lower() == 'true'
    coverage_success = args.coverage_success.lower() == 'true'
    
    # Create report generator
    generator = TestReportGenerator(args.build_dir, args.report_dir, args.project_root)
    
    # Generate report
    success = generator.generate_report(
        test_passed, args.test_duration,
        coverage_success, args.coverage_duration
    )
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main() 