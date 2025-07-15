#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
DDE文件管理器单元测试报表生成器
功能：生成包含测试结果、覆盖率、性能等信息的综合HTML报表
"""

import argparse
import json
import os
import re
import sys
import xml.etree.ElementTree as ET
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple


class TestReportGenerator:
    """测试报表生成器"""
    
    def __init__(self, build_dir: str, report_dir: str, project_root: str):
        self.build_dir = Path(build_dir)
        self.report_dir = Path(report_dir)
        self.project_root = Path(project_root)
        self.test_data = {}
        
    def parse_test_output(self, test_passed: bool, test_duration: int) -> Dict:
        """解析测试输出日志"""
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
                
            # 解析测试总数和通过数
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
            
            # 解析失败的测试详情
            failed_tests = re.findall(r'(\d+) - ([\w-]+) \(Failed\)\s+([\w-]+)', content)
            for test_num, test_name, component in failed_tests:
                test_info["failed_test_details"].append({
                    "number": test_num,
                    "name": test_name,
                    "component": component
                })
            
            # 改进测试执行详情解析 - 支持更多格式
            self._parse_test_execution_details(content, test_info)
            
            # 解析详细的失败信息
            test_info["detailed_failures"] = self._parse_detailed_failures(content)
                
        except Exception as e:
            print(f"解析测试输出时出错: {e}")
            
        return test_info
    
    def _parse_test_execution_details(self, content: str, test_info: Dict):
        """解析测试执行详情，支持多种格式"""
        # 格式1: Test #N: test_name .+ Passed/Failed X.XXX sec
        test_pattern1 = re.findall(r'Test #\d+: ([\w-]+) \.+ (Passed|Failed|\*\*\*Failed)\s+([\d.]+) sec', content)
        for test_name, status, duration in test_pattern1:
            status_normalized = "PASSED" if status == "Passed" else "FAILED"
            test_info["test_summary"][test_name] = {
                "status": status_normalized,
                "duration": float(duration)
            }
        
        # 格式2: Test #N: test_name .+ Passed/FAILED X.XXX sec
        test_pattern2 = re.findall(r'Test #\d+: ([\w-]+) \.+ (PASSED|FAILED)\s+([\d.]+) sec', content)
        for test_name, status, duration in test_pattern2:
            if test_name not in test_info["test_summary"]:  # 避免重复
                test_info["test_summary"][test_name] = {
                    "status": status,
                    "duration": float(duration)
                }
        
        # 格式3: 从ctest输出中提取更多信息
        # 查找所有测试开始的标记
        start_test_pattern = re.findall(r'Start\s+\d+:\s+([\w-]+)', content)
        for test_name in start_test_pattern:
            if test_name not in test_info["test_summary"]:
                # 如果没有找到执行时间，设置默认值
                test_info["test_summary"][test_name] = {
                    "status": "UNKNOWN",
                    "duration": 0.0
                }
        
        # 格式4: 从GoogleTest输出中解析
        # [==========] Running X tests from Y test suites.
        gtest_summary = re.search(r'\[==========\] Running (\d+) tests from (\d+) test suites?\.', content)
        if gtest_summary:
            total_gtest_tests = int(gtest_summary.group(1))
            # 查找每个测试的结果
            gtest_results = re.findall(r'\[\s*(OK|FAILED)\s*\]\s+(\w+)\.(\w+)\s+\((\d+)\s+ms\)', content)
            for status, suite, test_case, duration_ms in gtest_results:
                full_test_name = f"{suite}.{test_case}"
                test_info["test_summary"][full_test_name] = {
                    "status": "PASSED" if status == "OK" else "FAILED",
                    "duration": float(duration_ms) / 1000.0  # 转换为秒
                }
    
    def _parse_detailed_failures(self, content: str) -> List[Dict]:
        """解析详细的失败信息"""
        detailed_failures = []
        processed_tests = set()  # 用于去重
        
        # 查找每个失败测试的详细信息
        # 匹配模式：[  FAILED  ] TestSuite.TestCase
        failed_test_pattern = r'\[\s*FAILED\s*\]\s+(\w+)\.(\w+)'
        failed_matches = re.findall(failed_test_pattern, content)
        
        for suite_name, test_case in failed_matches:
            full_test_name = f"{suite_name}.{test_case}"
            
            # 避免重复处理同一个测试
            if full_test_name in processed_tests:
                continue
            processed_tests.add(full_test_name)
            
            # 查找该测试的失败详情
            failure_details = self._extract_failure_details(content, full_test_name)
            
            # 只有当找到失败详情时才添加
            if failure_details:
                detailed_failures.append({
                    "suite": suite_name,
                    "test_case": test_case,
                    "full_name": full_test_name,
                    "failures": failure_details
                })
        
        return detailed_failures
    
    def _extract_failure_details(self, content: str, test_name: str) -> List[Dict]:
        """提取特定测试的失败详情"""
        failures = []
        
        # 查找所有匹配的测试开始和结束位置
        run_pattern = rf'\[\s*RUN\s*\]\s+{re.escape(test_name)}'
        failed_pattern = rf'\[\s*FAILED\s*\]\s+{re.escape(test_name)}'
        
        run_matches = list(re.finditer(run_pattern, content))
        failed_matches = list(re.finditer(failed_pattern, content))
        
        if not run_matches or not failed_matches:
            return failures
        
        # 找到最佳的匹配对（优先选择没有数字前缀的，即直接的测试输出）
        best_run_match = None
        best_failed_match = None
        
        for run_match in run_matches:
            for failed_match in failed_matches:
                if failed_match.start() > run_match.end():
                    # 检查这个区间的内容质量（没有数字前缀的更好）
                    test_content = content[run_match.end():failed_match.start()]
                    # 如果内容不是以数字开头，说明是直接的测试输出，优先选择
                    if not re.match(r'^\s*\d+:', test_content):
                        best_run_match = run_match
                        best_failed_match = failed_match
                        break
                    elif best_run_match is None:  # 如果还没有找到更好的，先记录这个
                        best_run_match = run_match
                        best_failed_match = failed_match
            if best_run_match and not re.match(r'^\s*\d+:', content[best_run_match.end():best_failed_match.start()]):
                break  # 找到了最佳匹配，退出
        
        if best_run_match and best_failed_match:
            # 提取测试执行部分的内容
            test_content = content[best_run_match.end():best_failed_match.start()]
            
            # 清理内容，移除行号前缀
            cleaned_content = re.sub(r'^\s*\d+:\s*', '', test_content, flags=re.MULTILINE)
            
            # 查找失败断言 - 改进正则表达式，按优先级顺序处理
            processed_positions = set()  # 记录已处理的文本位置，避免重复
            
            # 1. 首先查找具体的失败格式（Expected equality 和 Value of）
            specific_patterns = [
                (r'Expected equality of these values:\s*([^\n]+)\s*Which is: ([^\n]+)\s*([^\n]+)\s*Which is: ([^\n]+)', "equality_failure"),
                (r'Value of: ([^\n]+)\s*Actual: ([^\n]+)\s*Expected: ([^\n]+)', "value_failure"),
            ]
            
            for pattern, failure_type in specific_patterns:
                for match in re.finditer(pattern, cleaned_content, re.DOTALL | re.MULTILINE):
                    start_pos, end_pos = match.span()
                    # 检查这个位置是否已经被处理
                    if any(start_pos < end and end_pos > start for start, end in processed_positions):
                        continue
                    
                    processed_positions.add((start_pos, end_pos))
                    match_groups = match.groups()
                    
                    failure_detail = {
                        "type": failure_type,
                        "details": match_groups
                    }
                    
                    # 格式化失败信息
                    if failure_type == "equality_failure" and len(match_groups) >= 4:
                        failure_detail["formatted"] = f"期望相等:\n  {match_groups[0].strip()} (实际: {match_groups[1].strip()})\n  {match_groups[2].strip()} (实际: {match_groups[3].strip()})"
                    elif failure_type == "value_failure" and len(match_groups) >= 3:
                        failure_detail["formatted"] = f"值比较失败:\n  表达式: {match_groups[0].strip()}\n  实际值: {match_groups[1].strip()}\n  期望值: {match_groups[2].strip()}"
                    else:
                        failure_detail["formatted"] = str(match_groups)
                        
                    failures.append(failure_detail)
            
            # 2. 然后查找通用的文件行号格式，但跳过已处理的区域
            file_line_pattern = r'(/[^:]+):(\d+): Failure\s*\n([^[]+?)(?=\n\[|\n/[^:]+:\d+:|$)'
            for match in re.finditer(file_line_pattern, cleaned_content, re.DOTALL | re.MULTILINE):
                start_pos, end_pos = match.span()
                # 检查这个位置是否与已处理的区域重叠
                if any(start_pos < end and end_pos > start for start, end in processed_positions):
                    continue
                
                processed_positions.add((start_pos, end_pos))
                match_groups = match.groups()
                
                failure_detail = {
                    "type": "file_line_failure",
                    "details": match_groups,
                    "formatted": f"文件: {match_groups[0]}:{match_groups[1]}\n{match_groups[2].strip()}"
                }
                
                failures.append(failure_detail)
        
        return failures
    
    def parse_coverage_data(self, coverage_success: bool, coverage_duration: int) -> Dict:
        """解析覆盖率数据"""
        coverage_info = {
            "success": coverage_success,
            "duration": coverage_duration,
            "line_coverage": 0.0,
            "function_coverage": 0.0,
            "branch_coverage": 0.0,
            "files_covered": 0,
            "total_files": 0,
            "details": []
        }
        
        if not coverage_success:
            return coverage_info
            
        # 尝试解析lcov输出
        coverage_output_file = self.report_dir / "coverage_output.log"
        if coverage_output_file.exists():
            try:
                with open(coverage_output_file, 'r', encoding='utf-8') as f:
                    content = f.read()
                    
                # 解析覆盖率百分比
                line_match = re.search(r'lines\.*: ([\d.]+)%', content)
                if line_match:
                    coverage_info["line_coverage"] = float(line_match.group(1))
                    
                func_match = re.search(r'functions\.*: ([\d.]+)%', content)
                if func_match:
                    coverage_info["function_coverage"] = float(func_match.group(1))
                    
                branch_match = re.search(r'branches\.*: ([\d.]+)%', content)
                if branch_match:
                    coverage_info["branch_coverage"] = float(branch_match.group(1))
                    
            except Exception as e:
                print(f"解析覆盖率输出时出错: {e}")
        
        # 尝试解析lcov info文件
        coverage_info_file = self.build_dir / "coverage" / "filtered.info"
        if coverage_info_file.exists():
            try:
                coverage_info.update(self._parse_lcov_info(coverage_info_file))
            except Exception as e:
                print(f"解析lcov info文件时出错: {e}")
                
        return coverage_info
    
    def _parse_lcov_info(self, info_file: Path) -> Dict:
        """解析lcov info文件"""
        details = []
        current_file = None
        
        try:
            with open(info_file, 'r', encoding='utf-8') as f:
                for line in f:
                    line = line.strip()
                    if line.startswith('SF:'):
                        current_file = {
                            "file": line[3:],
                            "lines_found": 0,
                            "lines_hit": 0,
                            "functions_found": 0,
                            "functions_hit": 0
                        }
                    elif line.startswith('LF:'):
                        if current_file:
                            current_file["lines_found"] = int(line[3:])
                    elif line.startswith('LH:'):
                        if current_file:
                            current_file["lines_hit"] = int(line[3:])
                    elif line.startswith('FNF:'):
                        if current_file:
                            current_file["functions_found"] = int(line[4:])
                    elif line.startswith('FNH:'):
                        if current_file:
                            current_file["functions_hit"] = int(line[4:])
                    elif line == 'end_of_record' and current_file:
                        # 计算该文件的覆盖率
                        if current_file["lines_found"] > 0:
                            current_file["line_coverage"] = (current_file["lines_hit"] / current_file["lines_found"]) * 100
                        else:
                            current_file["line_coverage"] = 0.0
                            
                        if current_file["functions_found"] > 0:
                            current_file["function_coverage"] = (current_file["functions_hit"] / current_file["functions_found"]) * 100
                        else:
                            current_file["function_coverage"] = 0.0
                            
                        details.append(current_file)
                        current_file = None
                        
        except Exception as e:
            print(f"解析lcov info文件内容时出错: {e}")
            
        return {
            "details": details,
            "files_covered": len([d for d in details if d["lines_hit"] > 0]),
            "total_files": len(details),
            "tree_structure": self._build_coverage_tree(details)
        }
    
    def _build_coverage_tree(self, details: List[Dict]) -> Dict:
        """构建简化的两级覆盖率结构：项目 -> 文件列表"""
        # 按模块分组文件
        modules = {}
        
        for detail in details:
            file_path = detail["file"]
            # 移除项目根目录前缀，获取相对路径
            if file_path.startswith(str(self.project_root)):
                relative_path = file_path[len(str(self.project_root)):].lstrip('/')
            else:
                relative_path = file_path
            
            # 跳过非源码文件
            if not self._is_source_file(relative_path):
                continue
            
            # 获取模块名（第一级目录）
            path_parts = relative_path.split('/')
            if len(path_parts) > 1:
                module_name = path_parts[0]  # 如 "src", "include", "tests"
                if module_name == "src" and len(path_parts) > 2:
                    module_name = path_parts[1]  # 如 "dfm-base", "dfm-framework"
            else:
                module_name = "根目录"
            
            if module_name not in modules:
                modules[module_name] = {
                    "name": module_name,
                    "files": [],
                    "stats": {
                        "lines_found": 0,
                        "lines_hit": 0,
                        "functions_found": 0,
                        "functions_hit": 0,
                        "line_coverage": 0.0,
                        "function_coverage": 0.0
                    }
                }
            
            # 添加文件到模块
            modules[module_name]["files"].append({
                "name": os.path.basename(relative_path),
                "path": relative_path,
                "stats": {
                    "lines_found": detail["lines_found"],
                    "lines_hit": detail["lines_hit"],
                    "functions_found": detail["functions_found"],
                    "functions_hit": detail["functions_hit"],
                    "line_coverage": detail["line_coverage"],
                    "function_coverage": detail["function_coverage"]
                }
            })
            
            # 累加模块统计
            modules[module_name]["stats"]["lines_found"] += detail["lines_found"]
            modules[module_name]["stats"]["lines_hit"] += detail["lines_hit"]
            modules[module_name]["stats"]["functions_found"] += detail["functions_found"]
            modules[module_name]["stats"]["functions_hit"] += detail["functions_hit"]
        
        # 计算每个模块的覆盖率
        for module in modules.values():
            stats = module["stats"]
            if stats["lines_found"] > 0:
                stats["line_coverage"] = (stats["lines_hit"] / stats["lines_found"]) * 100
            if stats["functions_found"] > 0:
                stats["function_coverage"] = (stats["functions_hit"] / stats["functions_found"]) * 100
            
            # 按覆盖率排序文件
            module["files"].sort(key=lambda x: x["stats"]["line_coverage"], reverse=True)
        
        return modules
    
    def _is_source_file(self, file_path: str) -> bool:
        """判断是否为源码文件"""
        source_extensions = ['.cpp', '.cc', '.cxx', '.c', '.h', '.hpp', '.hxx']
        return any(file_path.endswith(ext) for ext in source_extensions)
    

    
    def collect_build_info(self) -> Dict:
        """收集构建信息"""
        build_info = {
            "cmake_version": "Unknown",
            "compiler": "Unknown",
            "build_type": "Unknown",
            "build_time": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        }
        
        # 尝试从CMakeCache.txt读取信息
        cmake_cache = self.build_dir / "CMakeCache.txt"
        if cmake_cache.exists():
            try:
                with open(cmake_cache, 'r', encoding='utf-8') as f:
                    content = f.read()
                    
                cmake_version_match = re.search(r'CMAKE_VERSION:INTERNAL=(.+)', content)
                if cmake_version_match:
                    build_info["cmake_version"] = cmake_version_match.group(1)
                    
                compiler_match = re.search(r'CMAKE_CXX_COMPILER:FILEPATH=(.+)', content)
                if compiler_match:
                    build_info["compiler"] = os.path.basename(compiler_match.group(1))
                    
                build_type_match = re.search(r'CMAKE_BUILD_TYPE:STRING=(.+)', content)
                if build_type_match:
                    build_info["build_type"] = build_type_match.group(1)
                    
            except Exception as e:
                print(f"读取CMake缓存时出错: {e}")
                
        return build_info
    
    def generate_html_report(self, test_info: Dict, coverage_info: Dict, build_info: Dict) -> str:
        """生成HTML报表"""
        
        # 计算总体状态
        overall_status = "SUCCESS" if test_info["passed"] and coverage_info["success"] else "FAILED"
        status_color = "#28a745" if overall_status == "SUCCESS" else "#dc3545"
        
        # 生成失败测试的详细信息
        failed_tests_html = ""
        if test_info["failed_test_details"] or test_info["detailed_failures"]:
            failed_tests_html = """
            <div class="alert alert-danger">
                <h5>❌ 失败的测试</h5>
            """
            
            # 显示基本失败信息
            if test_info["failed_test_details"]:
                failed_tests_html += "<ul>"
                for failed_test in test_info["failed_test_details"]:
                    failed_tests_html += f"""
                        <li><strong>{failed_test['name']}</strong> (组件: {failed_test['component']})</li>
                    """
                failed_tests_html += "</ul>"
            
            # 显示详细失败信息
            if test_info["detailed_failures"]:
                failed_tests_html += """
                <div class="mt-3">
                    <h6>🔍 详细失败信息:</h6>
                """
                for failure in test_info["detailed_failures"]:
                    failed_tests_html += f"""
                    <div class="card mb-2">
                        <div class="card-header py-2">
                            <strong>{failure['suite']}.{failure['test_case']}</strong>
                        </div>
                        <div class="card-body py-2">
                    """
                    
                    if failure["failures"]:
                        for fail_detail in failure["failures"]:
                            formatted_msg = fail_detail.get("formatted", "断言失败")
                            failed_tests_html += f"""
                            <div class="mb-2">
                                <div class="alert alert-light p-2">
                                    <small class="text-muted">{fail_detail['type'].replace('_', ' ').title()}</small>
                                    <pre class="text-danger mb-0" style="font-size: 0.85em; white-space: pre-wrap;">{formatted_msg}</pre>
                                </div>
                            </div>
                            """
                    else:
                        failed_tests_html += "<em>详细信息解析失败</em>"
                    
                    failed_tests_html += """
                        </div>
                    </div>
                    """
                
                failed_tests_html += "</div>"
            
            failed_tests_html += "</div>"
        
        # 生成覆盖率详情 - 模块化卡片布局
        coverage_details_html = ""
        if coverage_info["success"] and coverage_info.get("tree_structure"):
            coverage_details_html = """
            <div class="mt-4">
                <h5>📁 模块覆盖率详情</h5>
                <div class="coverage-modules">
            """
            coverage_details_html += self._generate_coverage_modules_html(coverage_info["tree_structure"])
            coverage_details_html += """
                </div>
            </div>
            """
        
        # 生成测试执行详情
        test_details_html = ""
        if test_info["test_summary"]:
            test_details_html = """
            <div class="mt-4">
                <h5>🔍 测试执行详情</h5>
                <div class="table-responsive">
                    <table class="table table-sm table-striped">
                        <thead>
                            <tr>
                                <th>测试名称</th>
                                <th>状态</th>
                                <th>执行时间</th>
                            </tr>
                        </thead>
                        <tbody>
            """
            
            for test_name, test_detail in sorted(test_info["test_summary"].items()):
                status_badge = "success" if test_detail["status"] == "PASSED" else "danger"
                status_icon = "✅" if test_detail["status"] == "PASSED" else "❌"
                
                test_details_html += f"""
                    <tr>
                        <td><code>{test_name}</code></td>
                        <td><span class="badge badge-{status_badge}">{status_icon} {test_detail['status']}</span></td>
                        <td>{test_detail['duration']:.3f}s</td>
                    </tr>
                """
            
            test_details_html += """
                        </tbody>
                    </table>
                </div>
            </div>
            """
        
        html_content = f"""
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>DDE文件管理器 - 单元测试报告</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
    <style>
        .status-card {{
            border-left: 4px solid {status_color};
        }}
        .metric-card {{
            transition: transform 0.2s;
        }}
        .metric-card:hover {{
            transform: translateY(-2px);
        }}
        .coverage-bar {{
            height: 8px;
            border-radius: 4px;
            background-color: #e9ecef;
            overflow: hidden;
        }}
        .coverage-fill {{
            height: 100%;
            transition: width 0.3s ease;
        }}
        .timestamp {{
            color: #6c757d;
            font-size: 0.875rem;
        }}
        
        /* 现代化模块覆盖率样式 */
        .coverage-modules {{
            max-height: 800px;
            overflow-y: auto;
        }}
        
        .module-card {{
            border: 1px solid #e3e6f0;
            border-radius: 0.5rem;
            box-shadow: 0 0.125rem 0.25rem rgba(0, 0, 0, 0.075);
            transition: all 0.15s ease-in-out;
        }}
        
        .module-card:hover {{
            box-shadow: 0 0.5rem 1rem rgba(0, 0, 0, 0.15);
            transform: translateY(-2px);
        }}
        
        .module-card .card-header {{
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border-bottom: none;
            border-radius: 0.5rem 0.5rem 0 0;
            padding: 0.75rem 1rem;
        }}
        
        .module-icon {{
            font-size: 1.1rem;
        }}
        
        .module-name {{
            font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, sans-serif;
            font-weight: 600;
            color: white;
        }}
        
        .module-stats .badge {{
            font-size: 0.8rem;
            padding: 0.25rem 0.5rem;
            border-radius: 0.25rem;
            font-weight: 500;
        }}
        
        .coverage-metric {{
            margin-bottom: 1rem;
        }}
        
        .coverage-metric .progress {{
            border-radius: 0.25rem;
            background-color: #f1f3f4;
        }}
        
        .coverage-metric .progress-bar {{
            border-radius: 0.25rem;
            transition: width 0.6s ease;
        }}
        
        .files-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
            gap: 0.75rem;
            margin-top: 1rem;
        }}
        
        .file-item {{
            background: #f8f9fa;
            border: 1px solid #e9ecef;
            border-radius: 0.375rem;
            padding: 0.75rem;
            transition: all 0.2s ease;
        }}
        
        .file-item:hover {{
            background: #e9ecef;
            border-color: #dee2e6;
            transform: translateY(-1px);
        }}
        
        .file-header {{
            display: flex;
            align-items: center;
            margin-bottom: 0.5rem;
        }}
        
        .file-icon {{
            font-size: 1rem;
            margin-right: 0.5rem;
        }}
        
        .file-name {{
            font-family: 'Consolas', 'Monaco', 'Courier New', monospace;
            font-size: 0.85rem;
            font-weight: 500;
            color: #495057;
            flex-grow: 1;
        }}
        
        .file-stats {{
            display: flex;
            gap: 0.25rem;
            margin-bottom: 0.25rem;
        }}
        
        .file-stats .badge {{
            font-size: 0.7rem;
            padding: 0.15rem 0.4rem;
            border-radius: 0.2rem;
            font-weight: 500;
        }}
        
        .file-details {{
            font-size: 0.75rem;
            color: #6c757d;
        }}
        
        .badge-sm {{
            font-size: 0.7rem;
            padding: 0.15rem 0.4rem;
        }}
    </style>
</head>
<body class="bg-light">
    <div class="container-fluid py-4">
        <!-- 页头 -->
        <div class="row mb-4">
            <div class="col">
                <div class="d-flex justify-content-between align-items-center">
                    <div>
                        <h1 class="h2 mb-1">
                            <i class="fas fa-vial text-primary"></i>
                            DDE文件管理器 - 单元测试报告
                        </h1>
                        <p class="text-muted mb-0">
                            生成时间: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
                        </p>
                    </div>
                    <div class="text-end">
                        <span class="badge fs-6" style="background-color: {status_color}">
                            {overall_status}
                        </span>
                    </div>
                </div>
            </div>
        </div>

        <!-- 概览卡片 -->
        <div class="row mb-4">
            <div class="col-md-3">
                <div class="card metric-card h-100">
                    <div class="card-body text-center">
                        <div class="text-primary mb-2">
                            <i class="fas fa-check-circle fa-2x"></i>
                        </div>
                        <h3 class="mb-1">{test_info['passed_tests']}</h3>
                        <p class="text-muted mb-0">通过测试</p>
                    </div>
                </div>
            </div>
            <div class="col-md-3">
                <div class="card metric-card h-100">
                    <div class="card-body text-center">
                        <div class="text-danger mb-2">
                            <i class="fas fa-times-circle fa-2x"></i>
                        </div>
                        <h3 class="mb-1">{test_info['failed_tests']}</h3>
                        <p class="text-muted mb-0">失败测试</p>
                    </div>
                </div>
            </div>
            <div class="col-md-3">
                <div class="card metric-card h-100">
                    <div class="card-body text-center">
                        <div class="text-info mb-2">
                            <i class="fas fa-clock fa-2x"></i>
                        </div>
                        <h3 class="mb-1">{test_info['duration']}s</h3>
                        <p class="text-muted mb-0">测试时长</p>
                    </div>
                </div>
            </div>
            <div class="col-md-3">
                <div class="card metric-card h-100">
                    <div class="card-body text-center">
                        <div class="text-success mb-2">
                            <i class="fas fa-percentage fa-2x"></i>
                        </div>
                        <h3 class="mb-1">{coverage_info['line_coverage']:.1f}%</h3>
                        <p class="text-muted mb-0">代码覆盖率</p>
                    </div>
                </div>
            </div>
        </div>

        <!-- 主要内容区域 -->
        <div class="row">
            <!-- 左侧：测试结果 -->
            <div class="col-lg-8">
                <!-- 测试状态总览 -->
                <div class="card status-card mb-4">
                    <div class="card-header">
                        <h5 class="mb-0">
                            <i class="fas fa-list-check"></i>
                            测试执行总览
                        </h5>
                    </div>
                    <div class="card-body">
                        <div class="row">
                            <div class="col-md-6">
                                <p><strong>总测试数:</strong> {test_info['total_tests']}</p>
                                <p><strong>通过率:</strong> {(test_info['passed_tests']/test_info['total_tests']*100) if test_info['total_tests'] > 0 else 0:.1f}%</p>
                            </div>
                            <div class="col-md-6">
                                <p><strong>执行时间:</strong> {test_info['duration']} 秒</p>
                                <p><strong>平均用时:</strong> {(test_info['duration']/test_info['total_tests']) if test_info['total_tests'] > 0 else 0:.3f} 秒/测试</p>
                            </div>
                        </div>
                    </div>
                </div>

                {failed_tests_html}
                {test_details_html}
            </div>

            <!-- 右侧：覆盖率和构建信息 -->
            <div class="col-lg-4">
                <!-- 覆盖率信息 -->
                <div class="card mb-4">
                    <div class="card-header">
                        <h5 class="mb-0">
                            <i class="fas fa-chart-pie"></i>
                            代码覆盖率
                        </h5>
                    </div>
                    <div class="card-body">
                        <div class="mb-3">
                            <div class="d-flex justify-content-between">
                                <span>行覆盖率</span>
                                <span>{coverage_info['line_coverage']:.1f}%</span>
                            </div>
                            <div class="coverage-bar">
                                <div class="coverage-fill" style="width: {coverage_info['line_coverage']}%; background-color: {self._get_coverage_color(coverage_info['line_coverage'])};"></div>
                            </div>
                        </div>
                        
                        <div class="mb-3">
                            <div class="d-flex justify-content-between">
                                <span>函数覆盖率</span>
                                <span>{coverage_info['function_coverage']:.1f}%</span>
                            </div>
                            <div class="coverage-bar">
                                <div class="coverage-fill" style="width: {coverage_info['function_coverage']}%; background-color: {self._get_coverage_color(coverage_info['function_coverage'])};"></div>
                            </div>
                        </div>

                        <div class="mb-3">
                            <div class="d-flex justify-content-between">
                                <span>分支覆盖率</span>
                                <span>{coverage_info['branch_coverage']:.1f}%</span>
                            </div>
                            <div class="coverage-bar">
                                <div class="coverage-fill" style="width: {coverage_info['branch_coverage']}%; background-color: {self._get_coverage_color(coverage_info['branch_coverage'])};"></div>
                            </div>
                        </div>

                        <hr>
                        <p class="mb-1"><strong>覆盖文件:</strong> {coverage_info['files_covered']}/{coverage_info['total_files']}</p>
                        <p class="mb-0"><strong>生成时间:</strong> {coverage_info['duration']} 秒</p>
                    </div>
                </div>

                <!-- 构建信息 -->
                <div class="card">
                    <div class="card-header">
                        <h5 class="mb-0">
                            <i class="fas fa-hammer"></i>
                            构建信息
                        </h5>
                    </div>
                    <div class="card-body">
                        <p class="mb-2"><strong>CMake版本:</strong> {build_info['cmake_version']}</p>
                        <p class="mb-2"><strong>编译器:</strong> {build_info['compiler']}</p>
                        <p class="mb-2"><strong>构建类型:</strong> {build_info['build_type']}</p>
                        <p class="mb-0 timestamp"><strong>构建时间:</strong> {build_info['build_time']}</p>
                    </div>
                </div>
            </div>
        </div>

        {coverage_details_html}

        <!-- 页脚 -->
        <div class="row mt-5">
            <div class="col">
                <div class="text-center text-muted">
                    <p class="mb-0">
                        <i class="fas fa-code"></i>
                        Generated by DDE File Manager Test Suite
                    </p>
                </div>
            </div>
        </div>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.bundle.min.js"></script>
    <script>
        // 模块卡片动画效果
        document.addEventListener('DOMContentLoaded', function() {{
            // 为进度条添加动画效果
            const progressBars = document.querySelectorAll('.progress-bar');
            progressBars.forEach(function(bar) {{
                const width = bar.style.width;
                bar.style.width = '0%';
                setTimeout(function() {{
                    bar.style.width = width;
                }}, 100);
            }});
            
            // 为模块卡片添加渐入效果
            const moduleCards = document.querySelectorAll('.module-card');
            moduleCards.forEach(function(card, index) {{
                card.style.opacity = '0';
                card.style.transform = 'translateY(20px)';
                setTimeout(function() {{
                    card.style.transition = 'all 0.5s ease';
                    card.style.opacity = '1';
                    card.style.transform = 'translateY(0)';
                }}, index * 100);
            }});
        }});
    </script>
</body>
</html>
        """
        
        return html_content
    
    def _generate_coverage_modules_html(self, modules: Dict) -> str:
        """生成现代化的模块覆盖率卡片HTML"""
        html = ""
        
        # 按覆盖率排序模块
        sorted_modules = sorted(modules.values(), key=lambda x: x["stats"]["line_coverage"], reverse=True)
        
        for module in sorted_modules:
            stats = module["stats"]
            line_coverage_color = self._get_coverage_color(stats["line_coverage"])
            func_coverage_color = self._get_coverage_color(stats["function_coverage"])
            
            # 模块图标
            module_icon = "📦"
            if "dfm" in module["name"].lower():
                module_icon = "🔧"
            elif "test" in module["name"].lower():
                module_icon = "🧪"
            elif "include" in module["name"].lower():
                module_icon = "📋"
            
            html += f"""
            <div class="module-card card mb-3">
                <div class="card-header d-flex justify-content-between align-items-center">
                    <div class="d-flex align-items-center">
                        <span class="module-icon me-2">{module_icon}</span>
                        <h6 class="mb-0 module-name">{module["name"]}</h6>
                        <span class="badge ms-2 text-muted">{len(module["files"])} 文件</span>
                    </div>
                    <div class="module-stats">
                        <span class="badge me-1" style="background-color: {line_coverage_color}">{stats['line_coverage']:.1f}%</span>
                        <span class="badge" style="background-color: {func_coverage_color}">函数 {stats['function_coverage']:.1f}%</span>
                    </div>
                </div>
                <div class="card-body">
                    <div class="row mb-3">
                        <div class="col-md-6">
                            <div class="coverage-metric">
                                <div class="d-flex justify-content-between">
                                    <span>代码行覆盖率</span>
                                    <span class="fw-bold">{stats['line_coverage']:.1f}%</span>
                                </div>
                                <div class="progress" style="height: 6px;">
                                    <div class="progress-bar" style="width: {stats['line_coverage']:.1f}%; background-color: {line_coverage_color};"></div>
                                </div>
                                <small class="text-muted">{stats['lines_hit']}/{stats['lines_found']} 行</small>
                            </div>
                        </div>
                        <div class="col-md-6">
                            <div class="coverage-metric">
                                <div class="d-flex justify-content-between">
                                    <span>函数覆盖率</span>
                                    <span class="fw-bold">{stats['function_coverage']:.1f}%</span>
                                </div>
                                <div class="progress" style="height: 6px;">
                                    <div class="progress-bar" style="width: {stats['function_coverage']:.1f}%; background-color: {func_coverage_color};"></div>
                                </div>
                                <small class="text-muted">{stats['functions_hit']}/{stats['functions_found']} 函数</small>
                            </div>
                        </div>
                    </div>
                    
                    <div class="files-grid">
            """
            
            # 生成文件列表
            for file_info in module["files"]:
                file_stats = file_info["stats"]
                file_line_color = self._get_coverage_color(file_stats["line_coverage"])
                file_func_color = self._get_coverage_color(file_stats["function_coverage"])
                
                # 文件图标
                file_icon = "📄"
                if file_info["name"].endswith(('.cpp', '.cc', '.cxx')):
                    file_icon = "⚙️"
                elif file_info["name"].endswith(('.h', '.hpp', '.hxx')):
                    file_icon = "📋"
                
                html += f"""
                        <div class="file-item">
                            <div class="file-header">
                                <span class="file-icon">{file_icon}</span>
                                <span class="file-name">{file_info['name']}</span>
                            </div>
                            <div class="file-stats">
                                <span class="badge badge-sm" style="background-color: {file_line_color}">{file_stats['line_coverage']:.1f}%</span>
                                <span class="badge badge-sm" style="background-color: {file_func_color}">F:{file_stats['function_coverage']:.1f}%</span>
                            </div>
                            <div class="file-details">
                                <small class="text-muted">
                                    {file_stats['lines_hit']}/{file_stats['lines_found']} 行 | 
                                    {file_stats['functions_hit']}/{file_stats['functions_found']} 函数
                                </small>
                            </div>
                        </div>
                """
            
            html += """
                    </div>
                </div>
            </div>
            """
        
        return html
    
    def _get_coverage_color(self, percentage: float) -> str:
        """根据覆盖率百分比返回对应的颜色"""
        if percentage >= 80:
            return "#28a745"  # 绿色
        elif percentage >= 60:
            return "#ffc107"  # 黄色
        elif percentage >= 40:
            return "#fd7e14"  # 橙色
        else:
            return "#dc3545"  # 红色
    
    def generate_report(self, test_passed: bool, test_duration: int, 
                       coverage_success: bool, coverage_duration: int) -> bool:
        """生成完整的测试报表"""
        try:
            # 收集各种数据
            print("📊 收集测试数据...")
            test_info = self.parse_test_output(test_passed, test_duration)
            
            print("📊 收集覆盖率数据...")
            coverage_info = self.parse_coverage_data(coverage_success, coverage_duration)
            
            print("📊 收集构建信息...")
            build_info = self.collect_build_info()
            
            # 生成HTML报表
            print("📊 生成HTML报表...")
            html_content = self.generate_html_report(test_info, coverage_info, build_info)
            
            # 写入文件
            report_file = self.report_dir / "test_report.html"
            with open(report_file, 'w', encoding='utf-8') as f:
                f.write(html_content)
            
            # 生成JSON数据文件（供其他工具使用）
            json_data = {
                "test_info": test_info,
                "coverage_info": coverage_info,
                "build_info": build_info,
                "generated_at": datetime.now().isoformat()
            }
            
            json_file = self.report_dir / "test_data.json"
            with open(json_file, 'w', encoding='utf-8') as f:
                json.dump(json_data, f, indent=2, ensure_ascii=False)
            
            print(f"✅ 报表生成成功: {report_file}")
            return True
            
        except Exception as e:
            print(f"❌ 生成报表时出错: {e}")
            import traceback
            traceback.print_exc()
            return False


def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="生成DDE文件管理器单元测试报表")
    parser.add_argument("--build-dir", required=True, help="构建目录路径")
    parser.add_argument("--report-dir", required=True, help="报告输出目录路径")
    parser.add_argument("--project-root", required=True, help="项目根目录路径")
    parser.add_argument("--test-passed", required=True, help="测试是否通过 (true/false)")
    parser.add_argument("--test-duration", type=int, required=True, help="测试持续时间（秒）")
    parser.add_argument("--coverage-success", required=True, help="覆盖率生成是否成功 (true/false)")
    parser.add_argument("--coverage-duration", type=int, required=True, help="覆盖率生成持续时间（秒）")
    
    args = parser.parse_args()
    
    # 转换字符串参数为布尔值
    test_passed = args.test_passed.lower() == 'true'
    coverage_success = args.coverage_success.lower() == 'true'
    
    # 创建报表生成器
    generator = TestReportGenerator(args.build_dir, args.report_dir, args.project_root)
    
    # 生成报表
    success = generator.generate_report(
        test_passed, args.test_duration,
        coverage_success, args.coverage_duration
    )
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main() 