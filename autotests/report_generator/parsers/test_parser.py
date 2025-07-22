#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Test output parser for DDE File Manager test reports
"""

import re
from pathlib import Path
from typing import Dict, List


class TestOutputParser:
    """Parser for test execution output and results"""
    
    def __init__(self, report_dir: Path):
        self.report_dir = report_dir
    
    def parse_test_output(self, test_passed: bool, test_duration: int) -> Dict:
        """Parse test output log and extract test information"""
        test_output_file = self.report_dir / "test_output.log"
        
        test_info = {
            "passed": test_passed,
            "duration": test_duration,
            "total_tests": 0,
            "passed_tests": 0,
            "failed_tests": 0,
            "failed_test_details": [],
            "test_summary": {},
            "detailed_failures": []
        }
        
        if not test_output_file.exists():
            return test_info
            
        try:
            with open(test_output_file, 'r', encoding='utf-8') as f:
                content = f.read()
                
            # Parse test counts
            total_match = re.search(r'(\d+)% tests passed, (\d+) tests failed out of (\d+)', content)
            if total_match:
                failed_count = int(total_match.group(2))
                total_count = int(total_match.group(3))
                passed_count = total_count - failed_count
                
                test_info.update({
                    "total_tests": total_count,
                    "passed_tests": passed_count,
                    "failed_tests": failed_count
                })
            
            # Parse failed test details
            failed_tests = re.findall(r'(\d+) - ([\w-]+) \(Failed\)\s+([\w-]+)', content)
            for test_num, test_name, component in failed_tests:
                test_info["failed_test_details"].append({
                    "number": test_num,
                    "name": test_name,
                    "component": component
                })
            
            # Parse test execution details - support multiple formats
            self._parse_test_execution_details(content, test_info)
            
            # Parse detailed failure information
            test_info["detailed_failures"] = self._parse_detailed_failures(content)
                
        except Exception as e:
            print(f"Error parsing test output: {e}")
            
        return test_info
    
    def _parse_test_execution_details(self, content: str, test_info: Dict):
        """Parse test execution details, supporting multiple formats"""
        # Format 1: Test #N: test_name .+ Passed/Failed X.XXX sec
        test_pattern1 = re.findall(r'Test #\d+: ([\w-]+) \.+ (Passed|Failed|\*\*\*Failed)\s+([\d.]+) sec', content)
        for test_name, status, duration in test_pattern1:
            status_normalized = "PASSED" if status == "Passed" else "FAILED"
            test_info["test_summary"][test_name] = {
                "status": status_normalized,
                "duration": float(duration)
            }
        
        # Format 2: Test #N: test_name .+ Passed/FAILED X.XXX sec
        test_pattern2 = re.findall(r'Test #\d+: ([\w-]+) \.+ (PASSED|FAILED)\s+([\d.]+) sec', content)
        for test_name, status, duration in test_pattern2:
            if test_name not in test_info["test_summary"]:  # Avoid duplicates
                test_info["test_summary"][test_name] = {
                    "status": status,
                    "duration": float(duration)
                }
        
        # Format 3: Extract from ctest output
        start_test_pattern = re.findall(r'Start\s+\d+:\s+([\w-]+)', content)
        for test_name in start_test_pattern:
            if test_name not in test_info["test_summary"]:
                # Set default values if execution time not found
                test_info["test_summary"][test_name] = {
                    "status": "UNKNOWN",
                    "duration": 0.0
                }
        
        # Format 4: Parse from GoogleTest output
        gtest_summary = re.search(r'\[==========\] Running (\d+) tests from (\d+) test suites?\.', content)
        if gtest_summary:
            total_gtest_tests = int(gtest_summary.group(1))
            # Find each test result
            gtest_results = re.findall(r'\[\s*(OK|FAILED)\s*\]\s+(\w+)\.(\w+)\s+\((\d+)\s+ms\)', content)
            for status, suite, test_case, duration_ms in gtest_results:
                full_test_name = f"{suite}.{test_case}"
                test_info["test_summary"][full_test_name] = {
                    "status": "PASSED" if status == "OK" else "FAILED",
                    "duration": float(duration_ms) / 1000.0  # Convert to seconds
                }
    
    def _parse_detailed_failures(self, content: str) -> List[Dict]:
        """Parse detailed failure information"""
        detailed_failures = []
        processed_tests = set()  # For deduplication
        
        # Find detailed information for each failed test
        # Match pattern: [  FAILED  ] TestSuite.TestCase
        failed_test_pattern = r'\[\s*FAILED\s*\]\s+(\w+)\.(\w+)'
        failed_matches = re.findall(failed_test_pattern, content)
        
        for suite_name, test_case in failed_matches:
            full_test_name = f"{suite_name}.{test_case}"
            
            # Avoid duplicate processing of same test
            if full_test_name in processed_tests:
                continue
            processed_tests.add(full_test_name)
            
            # Find failure details for this test
            failure_details = self._extract_failure_details(content, full_test_name)
            
            # Only add if failure details found
            if failure_details:
                detailed_failures.append({
                    "suite": suite_name,
                    "test_case": test_case,
                    "full_name": full_test_name,
                    "failures": failure_details
                })
        
        return detailed_failures
    
    def _extract_failure_details(self, content: str, test_name: str) -> List[Dict]:
        """Extract failure details for specific test"""
        failures = []
        
        # Find all matching test start and end positions
        run_pattern = rf'\[\s*RUN\s*\]\s+{re.escape(test_name)}'
        failed_pattern = rf'\[\s*FAILED\s*\]\s+{re.escape(test_name)}'
        
        run_matches = list(re.finditer(run_pattern, content))
        failed_matches = list(re.finditer(failed_pattern, content))
        
        if not run_matches or not failed_matches:
            return failures
        
        # Find best matching pair (prefer direct test output without number prefixes)
        best_run_match = None
        best_failed_match = None
        
        for run_match in run_matches:
            for failed_match in failed_matches:
                if failed_match.start() > run_match.end():
                    # Check content quality in this range (no number prefix is better)
                    test_content = content[run_match.end():failed_match.start()]
                    # If content doesn't start with numbers, it's direct test output, prefer it
                    if not re.match(r'^\s*\d+:', test_content):
                        best_run_match = run_match
                        best_failed_match = failed_match
                        break
                    elif best_run_match is None:  # If no better match found yet, record this one
                        best_run_match = run_match
                        best_failed_match = failed_match
            if best_run_match and not re.match(r'^\s*\d+:', content[best_run_match.end():best_failed_match.start()]):
                break  # Found best match, exit
        
        if best_run_match and best_failed_match:
            # Extract test execution content
            test_content = content[best_run_match.end():best_failed_match.start()]
            
            # Clean content, remove line number prefixes
            cleaned_content = re.sub(r'^\s*\d+:\s*', '', test_content, flags=re.MULTILINE)
            
            # Find failure assertions - improved regex, process by priority
            processed_positions = set()  # Record processed text positions to avoid duplicates
            
            # 1. First find specific failure formats (Expected equality and Value of)
            specific_patterns = [
                (r'Expected equality of these values:\s*([^\n]+)\s*Which is: ([^\n]+)\s*([^\n]+)\s*Which is: ([^\n]+)', "equality_failure"),
                (r'Value of: ([^\n]+)\s*Actual: ([^\n]+)\s*Expected: ([^\n]+)', "value_failure"),
            ]
            
            for pattern, failure_type in specific_patterns:
                for match in re.finditer(pattern, cleaned_content, re.DOTALL | re.MULTILINE):
                    start_pos, end_pos = match.span()
                    # Check if this position has been processed
                    if any(start_pos < end and end_pos > start for start, end in processed_positions):
                        continue
                    
                    processed_positions.add((start_pos, end_pos))
                    match_groups = match.groups()
                    
                    failure_detail = {
                        "type": failure_type,
                        "details": match_groups
                    }
                    
                    # Format failure information
                    if failure_type == "equality_failure" and len(match_groups) >= 4:
                        failure_detail["formatted"] = f"Expected equality:\n  {match_groups[0].strip()} (actual: {match_groups[1].strip()})\n  {match_groups[2].strip()} (actual: {match_groups[3].strip()})"
                    elif failure_type == "value_failure" and len(match_groups) >= 3:
                        failure_detail["formatted"] = f"Value comparison failed:\n  Expression: {match_groups[0].strip()}\n  Actual: {match_groups[1].strip()}\n  Expected: {match_groups[2].strip()}"
                    else:
                        failure_detail["formatted"] = str(match_groups)
                        
                    failures.append(failure_detail)
            
            # 2. Then find general file:line format, but skip processed areas
            file_line_pattern = r'(/[^:]+):(\d+): Failure\s*\n([^[]+?)(?=\n\[|\n/[^:]+:\d+:|$)'
            for match in re.finditer(file_line_pattern, cleaned_content, re.DOTALL | re.MULTILINE):
                start_pos, end_pos = match.span()
                # Check if this position overlaps with processed areas
                if any(start_pos < end and end_pos > start for start, end in processed_positions):
                    continue
                
                processed_positions.add((start_pos, end_pos))
                match_groups = match.groups()
                
                failure_detail = {
                    "type": "file_line_failure",
                    "details": match_groups,
                    "formatted": f"File: {match_groups[0]}:{match_groups[1]}\n{match_groups[2].strip()}"
                }
                
                failures.append(failure_detail)
        
        return failures 