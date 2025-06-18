#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
            
            # 解析各个测试的执行时间
            test_times = re.findall(r'Test #\d+: ([\w-]+) \.+ Passed\s+([\d.]+) sec', content)
            for test_name, duration in test_times:
                test_info["test_summary"][test_name] = {
                    "status": "PASSED",
                    "duration": float(duration)
                }
                
            # 解析失败测试的时间
            failed_times = re.findall(r'Test #\d+: ([\w-]+) \.+\*\*\*Failed\s+([\d.]+) sec', content)
            for test_name, duration in failed_times:
                test_info["test_summary"][test_name] = {
                    "status": "FAILED", 
                    "duration": float(duration)
                }
            
            # 解析详细的失败信息
            test_info["detailed_failures"] = self._parse_detailed_failures(content)
                
        except Exception as e:
            print(f"解析测试输出时出错: {e}")
            
        return test_info
    
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
            "total_files": len(details)
        }
    
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
        
        # 生成覆盖率详情
        coverage_details_html = ""
        if coverage_info["success"] and coverage_info.get("details"):
            coverage_details_html = """
            <div class="mt-4">
                <h5>📁 文件覆盖率详情</h5>
                <div class="table-responsive">
                    <table class="table table-sm table-striped">
                        <thead>
                            <tr>
                                <th>文件</th>
                                <th>行覆盖率</th>
                                <th>函数覆盖率</th>
                                <th>行数</th>
                                <th>函数数</th>
                            </tr>
                        </thead>
                        <tbody>
            """
            
            for detail in sorted(coverage_info["details"], key=lambda x: x["line_coverage"], reverse=True)[:20]:  # 只显示前20个
                file_name = os.path.basename(detail["file"])
                line_coverage_color = self._get_coverage_color(detail["line_coverage"])
                func_coverage_color = self._get_coverage_color(detail["function_coverage"])
                
                coverage_details_html += f"""
                    <tr>
                        <td><code>{file_name}</code></td>
                        <td><span class="badge" style="background-color: {line_coverage_color}">{detail['line_coverage']:.1f}%</span></td>
                        <td><span class="badge" style="background-color: {func_coverage_color}">{detail['function_coverage']:.1f}%</span></td>
                        <td>{detail['lines_hit']}/{detail['lines_found']}</td>
                        <td>{detail['functions_hit']}/{detail['functions_found']}</td>
                    </tr>
                """
            
            coverage_details_html += """
                        </tbody>
                    </table>
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
</body>
</html>
        """
        
        return html_content
    
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