#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
HTML report generator for DDE File Manager test reports
"""

import os
from datetime import datetime
from pathlib import Path
from typing import Dict, List

from ..utils.ui_utils import (
    get_module_icon, get_coverage_color, get_coverage_class, get_section_icon
)
from ..utils.file_utils import sanitize_id


class HtmlReportGenerator:
    """Generate HTML test reports with navigation"""
    
    def __init__(self, build_dir: Path, project_root: Path = None):
        self.build_dir = build_dir
        self.project_root = project_root or build_dir.parent
    
    def generate_html_report(self, test_info: Dict, coverage_info: Dict, build_info: Dict) -> str:
        """Generate complete HTML report with global navigation"""
        
        # Calculate overall status
        overall_status = "SUCCESS" if test_info["passed"] and coverage_info["success"] else "FAILED"
        status_color = "#28a745" if overall_status == "SUCCESS" else "#dc3545"
        
        # Generate global navigation
        global_nav_html = self._generate_global_navigation(test_info, coverage_info)
        
        # Generate failed tests section
        failed_tests_html = self._generate_failed_tests_html(test_info)
        
        # Generate test execution details
        test_details_html = self._generate_test_details_html(test_info)
        
        # Generate coverage details with module navigation
        coverage_details_html = self._generate_coverage_details_html(coverage_info)
        
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
        {self._get_css_styles()}
    </style>
</head>
<body class="bg-light">
    {global_nav_html}
    
    <div class="container-fluid py-4 main-content">
        <!-- 页头 -->
        <div class="row mb-4" id="header-section">
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
        <div class="row mb-4" id="overview-section">
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
                        <h3 class="mb-1">{coverage_info['function_coverage']:.1f}%</h3>
                        <p class="text-muted mb-0">覆盖率</p>
                    </div>
                </div>
            </div>
        </div>

        <!-- 主要内容区域 -->
        <div class="row">
            <!-- 左侧：测试结果 -->
            <div class="col-lg-8">
                <!-- 测试状态总览 -->
                <div class="card status-card mb-4" id="test-overview-section">
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
                <div class="card mb-4" id="coverage-overview-section">
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
                                <div class="coverage-fill" style="width: {coverage_info['line_coverage']}%; background-color: {get_coverage_color(coverage_info['line_coverage'])};"></div>
                            </div>
                        </div>
                        
                        <div class="mb-3">
                            <div class="d-flex justify-content-between">
                                <span>函数覆盖率</span>
                                <span>{coverage_info['function_coverage']:.1f}%</span>
                            </div>
                            <div class="coverage-bar">
                                <div class="coverage-fill" style="width: {coverage_info['function_coverage']}%; background-color: {get_coverage_color(coverage_info['function_coverage'])};"></div>
                            </div>
                        </div>

                        <div class="mb-3">
                            <div class="d-flex justify-content-between">
                                <span>分支覆盖率</span>
                                <span>{coverage_info['branch_coverage']:.1f}%</span>
                            </div>
                            <div class="coverage-bar">
                                <div class="coverage-fill" style="width: {coverage_info['branch_coverage']}%; background-color: {get_coverage_color(coverage_info['branch_coverage'])};"></div>
                            </div>
                        </div>

                        <hr>
                        <p class="mb-1"><strong>覆盖文件:</strong> {coverage_info['files_covered']}/{coverage_info['total_files']}</p>
                        <p class="mb-0"><strong>生成时间:</strong> {coverage_info['duration']} 秒</p>
                    </div>
                </div>

                <!-- 构建信息 -->
                <div class="card" id="build-info-section">
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
        {self._get_javascript()}
    </script>
</body>
</html>
        """
        
        return html_content
    
    def _generate_global_navigation(self, test_info: Dict, coverage_info: Dict) -> str:
        """Generate global navigation bar"""
        
        # Determine section visibility
        has_failed_tests = test_info.get("failed_test_details") or test_info.get("detailed_failures")
        has_test_details = bool(test_info.get("test_summary"))
        has_coverage_details = coverage_info.get("success") and coverage_info.get("tree_structure")
        
        nav_html = """
        <!-- 全局导航栏 -->
        <nav class="navbar navbar-expand-lg navbar-dark bg-primary sticky-top global-nav">
            <div class="container-fluid">
                <a class="navbar-brand" href="#header-section">
                    <i class="fas fa-vial"></i> 测试报告
                </a>
                <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#globalNavbar">
                    <span class="navbar-toggler-icon"></span>
                </button>
                <div class="collapse navbar-collapse" id="globalNavbar">
                    <ul class="navbar-nav me-auto">
                        <li class="nav-item">
                            <a class="nav-link" href="#overview-section">
                                📊 总览
                            </a>
                        </li>
                        <li class="nav-item">
                            <a class="nav-link" href="#test-overview-section">
                                📋 测试概览
                            </a>
                        </li>
        """
        
        if has_failed_tests:
            nav_html += """
                        <li class="nav-item">
                            <a class="nav-link" href="#failed-tests-section">
                                ❌ 失败测试
                            </a>
                        </li>
            """
        
        if has_test_details:
            nav_html += """
                        <li class="nav-item">
                            <a class="nav-link" href="#test-details-section">
                                🔍 测试详情
                            </a>
                        </li>
            """
        
        nav_html += """
                        <li class="nav-item">
                            <a class="nav-link" href="#coverage-overview-section">
                                📈 覆盖率
                            </a>
                        </li>
        """
        
        if has_coverage_details:
            nav_html += """
                        <li class="nav-item">
                            <a class="nav-link" href="#coverage-details-section">
                                📁 模块覆盖率
                            </a>
                        </li>
            """
        
        nav_html += """
                        <li class="nav-item">
                            <a class="nav-link" href="#build-info-section">
                                🔨 构建信息
                            </a>
                        </li>
                    </ul>
                    <ul class="navbar-nav">
                        <li class="nav-item">
                            <button class="btn btn-outline-light btn-sm" id="toggleQuickNav">
                                <i class="fas fa-compass"></i> 快速导航
                            </button>
                        </li>
                    </ul>
                </div>
            </div>
        </nav>
        """
        
        return nav_html
    
    def _generate_failed_tests_html(self, test_info: Dict) -> str:
        """Generate failed tests section HTML"""
        if not (test_info.get("failed_test_details") or test_info.get("detailed_failures")):
            return ""
        
        failed_tests_html = """
        <div class="alert alert-danger" id="failed-tests-section">
            <h5>❌ 失败的测试</h5>
        """
        
        # Display basic failure information
        if test_info.get("failed_test_details"):
            failed_tests_html += "<ul>"
            for failed_test in test_info["failed_test_details"]:
                failed_tests_html += f"""
                    <li><strong>{failed_test['name']}</strong> (组件: {failed_test['component']})</li>
                """
            failed_tests_html += "</ul>"
        
        # Display detailed failure information
        if test_info.get("detailed_failures"):
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
                
                if failure.get("failures"):
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
        return failed_tests_html
    
    def _generate_test_details_html(self, test_info: Dict) -> str:
        """Generate test execution details HTML"""
        if not test_info.get("test_summary"):
            return ""
        
        test_details_html = """
        <div class="mt-4" id="test-details-section">
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
        
        return test_details_html
    
    def _generate_coverage_details_html(self, coverage_info: Dict) -> str:
        """Generate coverage details section with module navigation"""
        if not (coverage_info.get("success") and coverage_info.get("tree_structure")):
            return ""
        
        # Generate module navigation
        module_nav_html = self._generate_module_navigation(coverage_info["tree_structure"])
        
        # Generate floating navigation button
        floating_nav_button = self._generate_floating_nav_button()
        
        # Generate module cards
        modules_html = self._generate_coverage_modules_html(coverage_info["tree_structure"])
        
        coverage_details_html = f"""
        <div class="mt-4" id="coverage-details-section">
            <div class="d-flex justify-content-between align-items-center mb-3">
                <h5>📁 模块覆盖率详情</h5>
            </div>
            {module_nav_html}
            {floating_nav_button}
            <div class="coverage-modules">
                {modules_html}
            </div>
        </div>
        """
        
        return coverage_details_html
    
    def _generate_floating_nav_button(self) -> str:
        """Generate floating navigation button that's always accessible"""
        return """
        <!-- Floating Module Navigation Button -->
        <div id="floatingNavButton" class="floating-nav-button">
            <button class="btn btn-primary btn-lg" id="toggleModuleNavigation" title="Module Navigation">
                <i class="fas fa-list"></i>
            </button>
        </div>
        """
    
    def _generate_module_navigation(self, modules: Dict) -> str:
        """Generate module navigation panel with detailed plugin categorization"""
        
        # Group modules by type with detailed plugin categorization
        plugin_categories = {
            "daemon": [],
            "desktop": [],
            "filemanager": [],
            "common": [],
            "filedialog": []
        }
        service_modules = []
        core_modules = []
        test_modules = []
        
        for module_name, module_data in modules.items():
            stats = module_data["stats"]
            icon = get_module_icon(module_name)
            
            module_info = {
                "name": module_name,
                "icon": icon,
                "coverage": stats["line_coverage"],
                "file_count": len(module_data["files"]),
                "id": sanitize_id(module_name)
            }
            
            if module_name.startswith("Plugin:"):
                # Categorize plugins based on directory structure from module name
                plugin_info_str = module_name[8:]  # Remove "Plugin: " prefix
                
                # Extract category from module name if present (e.g., "Plugin Name (category)")
                if "(" in plugin_info_str and ")" in plugin_info_str:
                    category = plugin_info_str.split("(")[-1].split(")")[0].lower()
                    if category in plugin_categories:
                        plugin_categories[category].append(module_info)
                    else:
                        plugin_categories["common"].append(module_info)
                else:
                    # Fallback: try to categorize based on plugin name patterns
                    plugin_name_lower = plugin_info_str.lower()
                    if any(name in plugin_name_lower for name in ["ddplugin", "desktop", "canvas", "organizer", "wallpaper", "background"]):
                        plugin_categories["desktop"].append(module_info)
                    elif any(name in plugin_name_lower for name in ["filedialog", "dialog"]):
                        plugin_categories["filedialog"].append(module_info)
                    elif any(name in plugin_name_lower for name in ["daemon", "filemanager1"]):
                        plugin_categories["daemon"].append(module_info)
                    elif any(name in plugin_name_lower for name in ["dfmplugin"]):
                        # Most dfmplugin are either filemanager or common, default to filemanager
                        plugin_categories["filemanager"].append(module_info)
                    else:
                        plugin_categories["common"].append(module_info)
            elif module_name.startswith("Service:"):
                service_modules.append(module_info)
            elif module_name.startswith(("Test:", "AutoTest:")):
                test_modules.append(module_info)
            else:
                core_modules.append(module_info)
        
        # Sort each category by coverage
        for category in plugin_categories.values():
            category.sort(key=lambda x: x["coverage"], reverse=True)
        service_modules.sort(key=lambda x: x["coverage"], reverse=True)
        core_modules.sort(key=lambda x: x["coverage"], reverse=True)
        test_modules.sort(key=lambda x: x["coverage"], reverse=True)
        
        navigation_html = """
        <!-- 模块导航栏 -->
        <div id="moduleNavigation" class="module-navigation" style="display: none;">
            <div class="navigation-content">
                <div class="navigation-header">
                    <div class="d-flex justify-content-between align-items-center">
                        <h6 class="mb-0">📍 模块导航</h6>
                        <button class="btn btn-sm btn-outline-secondary" id="closeModuleNavigation">
                            <i class="fas fa-times"></i>
                        </button>
                    </div>
                    <div class="mt-2">
                        <input type="text" class="form-control form-control-sm" id="moduleSearch" 
                               placeholder="🔍 搜索模块..." autocomplete="off">
                    </div>
                </div>
                <div class="navigation-body">
        """
        
        # Generate plugin navigation by category
        plugin_category_names = {
            "daemon": ("守护进程插件", "fas fa-server"),
            "desktop": ("桌面插件", "fas fa-desktop"),
            "filemanager": ("文件管理器插件", "fas fa-folder-open"),
            "common": ("通用插件", "fas fa-plug"),
            "filedialog": ("文件对话框插件", "fas fa-file")
        }
        
        for category_key, (category_name, icon_class) in plugin_category_names.items():
            if plugin_categories[category_key]:
                navigation_html += self._generate_nav_category(category_name, icon_class, plugin_categories[category_key])
        
        # Generate service navigation
        if service_modules:
            navigation_html += self._generate_nav_category("服务模块", "fas fa-server", service_modules)
        
        # Generate core module navigation
        if core_modules:
            navigation_html += self._generate_nav_category("核心模块", "fas fa-cogs", core_modules)
        
        # Generate test module navigation
        if test_modules:
            navigation_html += self._generate_nav_category("测试模块", "fas fa-vial", test_modules)
        
        navigation_html += """
                </div>
            </div>
        </div>
        """
        
        return navigation_html
    
    def _generate_nav_category(self, category_name: str, icon_class: str, modules: List[Dict]) -> str:
        """Generate navigation category HTML"""
        category_html = f"""
        <div class="nav-category">
            <div class="nav-category-header">
                <i class="{icon_class}"></i> {category_name} ({len(modules)})
            </div>
            <div class="nav-items">
        """
        
        for module in modules:
            coverage_class = get_coverage_class(module["coverage"])
            category_html += f"""
                <div class="nav-item" data-target="module-{module['id']}" data-search="{module['name'].lower()}">
                    <div class="nav-item-content">
                        <span class="nav-icon">{module['icon']}</span>
                        <span class="nav-name">{module['name']}</span>
                        <span class="nav-badge badge {coverage_class}">{module['coverage']:.1f}%</span>
                    </div>
                    <small class="nav-files">{module['file_count']} 文件</small>
                </div>
            """
        
        category_html += """
            </div>
        </div>
        """
        
        return category_html
    
    def _generate_coverage_modules_html(self, modules: Dict) -> str:
        """Generate modern module coverage cards HTML"""
        html = ""
        
        # Sort modules by coverage
        sorted_modules = sorted(modules.values(), key=lambda x: x["stats"]["line_coverage"], reverse=True)
        
        for module in sorted_modules:
            stats = module["stats"]
            line_coverage_color = get_coverage_color(stats["line_coverage"])
            func_coverage_color = get_coverage_color(stats["function_coverage"])
            
            # Smart module icon selection
            module_icon = get_module_icon(module["name"])
            
            # Module ID for navigation
            module_id = sanitize_id(module["name"])
            
            html += f"""
            <div class="module-card card mb-3" id="module-{module_id}">
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
            
            # Generate file list
            for file_info in module["files"]:
                file_stats = file_info["stats"]
                file_line_color = get_coverage_color(file_stats["line_coverage"])
                file_func_color = get_coverage_color(file_stats["function_coverage"])
                
                # File icon
                file_icon = "📄"
                if file_info["name"].endswith(('.cpp', '.cc', '.cxx')):
                    file_icon = "⚙️"
                elif file_info["name"].endswith(('.h', '.hpp', '.hxx')):
                    file_icon = "📋"
                
                # Generate clickable file name - restore original functionality
                file_name_html = file_info['name']
                
                # Primary link: Coverage HTML (original functionality)
                if file_info.get('coverage_html_path') and os.path.exists(file_info['coverage_html_path']):
                    file_name_html = f'<a href="file://{file_info["coverage_html_path"]}" target="_blank" class="file-link coverage-primary">{file_info["name"]}</a>'
                    
                    # Secondary link: Source file
                    source_file_path = self.project_root / file_info['path']
                    if source_file_path.exists():
                        file_name_html += f' <a href="file://{source_file_path}" target="_blank" class="file-link source-secondary" title="View Source Code"><i class="fas fa-code"></i></a>'
                else:
                    # Fallback: Source file link if coverage not available
                    source_file_path = self.project_root / file_info['path']
                    if source_file_path.exists():
                        file_name_html = f'<a href="file://{source_file_path}" target="_blank" class="file-link source-primary">{file_info["name"]}</a>'
                
                html += f"""
                        <div class="file-item">
                            <div class="file-header">
                                <span class="file-icon">{file_icon}</span>
                                <span class="file-name">{file_name_html}</span>
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
    
    def _get_css_styles(self) -> str:
        """Get CSS styles for the report"""
        return """
        .status-card {
            border-left: 4px solid #28a745;
        }
        .metric-card {
            transition: transform 0.2s;
        }
        .metric-card:hover {
            transform: translateY(-2px);
        }
        .coverage-bar {
            height: 8px;
            border-radius: 4px;
            background-color: #e9ecef;
            overflow: hidden;
        }
        .coverage-fill {
            height: 100%;
            transition: width 0.3s ease;
        }
        .timestamp {
            color: #6c757d;
            font-size: 0.875rem;
        }
        
        /* 全局导航样式 */
        .global-nav {
            box-shadow: 0 2px 4px rgba(0,0,0,.1);
            z-index: 1030;
        }
        
        .main-content {
            padding-top: 20px;
        }
        
        /* 平滑滚动 */
        html {
            scroll-behavior: smooth;
        }
        
        /* 导航链接悬停效果 */
        .navbar-nav .nav-link:hover {
            background-color: rgba(255, 255, 255, 0.1);
            border-radius: 4px;
        }
        
        /* 现代化模块覆盖率样式 */
        .coverage-modules {
            max-height: 800px;
            overflow-y: auto;
        }
        
        .module-card {
            border: 1px solid #e3e6f0;
            border-radius: 0.5rem;
            box-shadow: 0 0.125rem 0.25rem rgba(0, 0, 0, 0.075);
            transition: all 0.15s ease-in-out;
        }
        
        .module-card:hover {
            box-shadow: 0 0.5rem 1rem rgba(0, 0, 0, 0.15);
            transform: translateY(-2px);
        }
        
        .module-card .card-header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border-bottom: none;
            border-radius: 0.5rem 0.5rem 0 0;
            padding: 0.75rem 1rem;
        }
        
        .module-icon {
            font-size: 1.1rem;
        }
        
        .module-name {
            font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, sans-serif;
            font-weight: 600;
            color: white;
        }
        
        .module-stats .badge {
            font-size: 0.8rem;
            padding: 0.25rem 0.5rem;
            border-radius: 0.25rem;
            font-weight: 500;
        }
        
        .coverage-metric {
            margin-bottom: 1rem;
        }
        
        .coverage-metric .progress {
            border-radius: 0.25rem;
            background-color: #f1f3f4;
        }
        
        .coverage-metric .progress-bar {
            border-radius: 0.25rem;
            transition: width 0.6s ease;
        }
        
        .files-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
            gap: 0.75rem;
            margin-top: 1rem;
        }
        
        .file-item {
            background: #f8f9fa;
            border: 1px solid #e9ecef;
            border-radius: 0.375rem;
            padding: 0.75rem;
            transition: all 0.2s ease;
        }
        
        .file-item:hover {
            background: #e9ecef;
            border-color: #dee2e6;
            transform: translateY(-1px);
        }
        
        .file-header {
            display: flex;
            align-items: center;
            margin-bottom: 0.5rem;
        }
        
        .file-icon {
            font-size: 1rem;
            margin-right: 0.5rem;
        }
        
        .file-name {
            font-family: 'Consolas', 'Monaco', 'Courier New', monospace;
            font-size: 0.85rem;
            font-weight: 500;
            color: #495057;
            flex-grow: 1;
        }
        
        .file-link {
            color: #007bff;
            text-decoration: none;
            transition: color 0.2s ease;
        }
        
        .file-link:hover {
            color: #0056b3;
            text-decoration: underline;
        }
        
        .coverage-primary {
            color: #007bff;
            font-weight: 500;
        }
        
        .coverage-primary:hover {
            color: #0056b3;
            text-decoration: underline;
        }
        
        .source-primary {
            color: #007bff;
            font-weight: 500;
        }
        
        .source-primary:hover {
            color: #0056b3;
            text-decoration: underline;
        }
        
        .source-secondary {
            color: #28a745;
            margin-left: 0.5rem;
            padding: 0.2rem 0.4rem;
            border-radius: 0.2rem;
            background-color: #28a745;
            color: white;
            font-size: 0.8rem;
            transition: all 0.2s ease;
        }
        
        .source-secondary:hover {
            background-color: #218838;
            color: white;
            text-decoration: none;
            transform: scale(1.05);
        }
        
        .file-stats {
            display: flex;
            gap: 0.25rem;
            margin-bottom: 0.25rem;
        }
        
        .file-stats .badge {
            font-size: 0.7rem;
            padding: 0.15rem 0.4rem;
            border-radius: 0.2rem;
            font-weight: 500;
        }
        
        .file-details {
            font-size: 0.75rem;
            color: #6c757d;
        }
        
        .badge-sm {
            font-size: 0.7rem;
            padding: 0.15rem 0.4rem;
        }
        
        /* 浮动导航按钮样式 */
        .floating-nav-button {
            position: fixed;
            bottom: 30px;
            right: 30px;
            z-index: 1060;
        }
        
        .floating-nav-button .btn {
            border-radius: 50%;
            width: 60px;
            height: 60px;
            box-shadow: 0 4px 20px rgba(0, 123, 255, 0.3);
            transition: all 0.3s ease;
            border: none;
        }
        
        .floating-nav-button .btn:hover {
            transform: scale(1.1);
            box-shadow: 0 6px 25px rgba(0, 123, 255, 0.4);
        }
        
        .floating-nav-button .btn i {
            font-size: 1.2rem;
        }
        
        /* 模块导航样式 */
        .module-navigation {
            position: fixed;
            top: 50%;
            right: 20px;
            transform: translateY(-50%);
            width: 400px;
            max-height: 80vh;
            background: white;
            border-radius: 12px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2);
            z-index: 1050;
            border: 1px solid #e3e6f0;
        }
        
        .navigation-content {
            display: flex;
            flex-direction: column;
            height: 100%;
            max-height: 70vh;
        }
        
        .navigation-header {
            padding: 1rem;
            border-bottom: 1px solid #e3e6f0;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border-radius: 12px 12px 0 0;
        }
        
        .navigation-header h6 {
            color: white;
            margin: 0;
        }
        
        .navigation-header .btn-outline-secondary {
            border-color: rgba(255, 255, 255, 0.3);
            color: white;
        }
        
        .navigation-header .btn-outline-secondary:hover {
            background-color: rgba(255, 255, 255, 0.1);
            border-color: rgba(255, 255, 255, 0.5);
        }
        
        .navigation-body {
            flex: 1;
            overflow-y: auto;
            padding: 0.5rem;
        }
        
        .nav-category {
            margin-bottom: 1rem;
        }
        
        .nav-category-header {
            font-weight: 600;
            color: #495057;
            font-size: 0.9rem;
            padding: 0.5rem 0.75rem;
            background: #f8f9fa;
            border-radius: 6px;
            margin-bottom: 0.5rem;
            border-left: 4px solid #007bff;
        }
        
        .nav-items {
            display: flex;
            flex-direction: column;
            gap: 0.25rem;
        }
        
        .nav-item {
            padding: 0.5rem 0.75rem;
            border-radius: 6px;
            cursor: pointer;
            transition: all 0.2s ease;
            border: 1px solid transparent;
        }
        
        .nav-item:hover {
            background-color: #f8f9fa;
            border-color: #dee2e6;
            transform: translateX(2px);
        }
        
        .nav-item.hidden {
            display: none;
        }
        
        .nav-item-content {
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }
        
        .nav-icon {
            font-size: 1rem;
            width: 20px;
            text-align: center;
        }
        
        .nav-name {
            flex: 1;
            font-size: 0.85rem;
            font-weight: 500;
            color: #495057;
        }
        
        .nav-badge {
            font-size: 0.7rem;
            padding: 0.2rem 0.4rem;
        }
        
        .nav-files {
            color: #6c757d;
            margin-left: 2rem;
            font-size: 0.75rem;
        }
        
        /* 高亮目标模块 */
        .module-card.highlight {
            animation: highlightPulse 2s ease-in-out;
        }
        
        @keyframes highlightPulse {
            0% {
                box-shadow: 0 0 0 0 rgba(0, 123, 255, 0.7);
            }
            50% {
                box-shadow: 0 0 0 10px rgba(0, 123, 255, 0.2);
                transform: translateY(-2px);
            }
            100% {
                box-shadow: 0 0 0 0 rgba(0, 123, 255, 0);
                transform: translateY(0);
            }
        }
        
        /* 快速导航面板 */
        .quick-nav-panel {
            position: fixed;
            top: 70px;
            right: 20px;
            width: 250px;
            max-height: 60vh;
            background: white;
            border-radius: 8px;
            box-shadow: 0 4px 20px rgba(0, 0, 0, 0.15);
            z-index: 1040;
            border: 1px solid #e3e6f0;
            overflow-y: auto;
        }
        
        .quick-nav-header {
            padding: 1rem;
            background: #f8f9fa;
            border-bottom: 1px solid #e3e6f0;
            border-radius: 8px 8px 0 0;
        }
        
        .quick-nav-item {
            padding: 0.75rem 1rem;
            border-bottom: 1px solid #f1f3f4;
            cursor: pointer;
            transition: background-color 0.2s ease;
        }
        
        .quick-nav-item:hover {
            background-color: #f8f9fa;
        }
        
        .quick-nav-item:last-child {
            border-bottom: none;
        }
        
        /* 响应式设计 */
        @media (max-width: 768px) {
            .module-navigation {
                position: fixed;
                top: 0;
                right: 0;
                left: 0;
                bottom: 0;
                width: 100%;
                max-height: 100vh;
                border-radius: 0;
                transform: none;
            }
            
            .navigation-header {
                border-radius: 0;
            }
            
            .quick-nav-panel {
                position: fixed;
                top: 60px;
                right: 10px;
                left: 10px;
                width: auto;
            }
            
            .floating-nav-button {
                bottom: 20px;
                right: 20px;
            }
            
            .floating-nav-button .btn {
                width: 50px;
                height: 50px;
            }
            
            .floating-nav-button .btn i {
                font-size: 1rem;
            }
            
            .files-grid {
                grid-template-columns: 1fr;
            }
        }
        """
    
    def _get_javascript(self) -> str:
        """Get JavaScript code for the report"""
        return """
        // 全局导航和动画效果
        document.addEventListener('DOMContentLoaded', function() {
            // 为进度条添加动画效果
            const progressBars = document.querySelectorAll('.progress-bar');
            progressBars.forEach(function(bar) {
                const width = bar.style.width;
                bar.style.width = '0%';
                setTimeout(function() {
                    bar.style.width = width;
                }, 100);
            });
            
            // 为模块卡片添加渐入效果
            const moduleCards = document.querySelectorAll('.module-card');
            moduleCards.forEach(function(card, index) {
                card.style.opacity = '0';
                card.style.transform = 'translateY(20px)';
                setTimeout(function() {
                    card.style.transition = 'all 0.5s ease';
                    card.style.opacity = '1';
                    card.style.transform = 'translateY(0)';
                }, index * 100);
            });
            
            // 全局导航平滑滚动
            const navLinks = document.querySelectorAll('.navbar-nav .nav-link');
            navLinks.forEach(function(link) {
                link.addEventListener('click', function(e) {
                    e.preventDefault();
                    const targetId = this.getAttribute('href');
                    const targetElement = document.querySelector(targetId);
                    
                    if (targetElement) {
                        targetElement.scrollIntoView({
                            behavior: 'smooth',
                            block: 'start'
                        });
                        
                        // 高亮目标元素
                        targetElement.classList.add('highlight');
                        setTimeout(() => {
                            targetElement.classList.remove('highlight');
                        }, 2000);
                    }
                });
            });
            
            // 快速导航功能
            const toggleQuickNavBtn = document.getElementById('toggleQuickNav');
            let quickNavPanel = null;
            
            if (toggleQuickNavBtn) {
                toggleQuickNavBtn.addEventListener('click', function() {
                    if (quickNavPanel && quickNavPanel.style.display !== 'none') {
                        hideQuickNav();
                    } else {
                        showQuickNav();
                    }
                });
            }
            
            function showQuickNav() {
                if (!quickNavPanel) {
                    createQuickNavPanel();
                }
                quickNavPanel.style.display = 'block';
                toggleQuickNavBtn.innerHTML = '<i class="fas fa-compass"></i> 隐藏导航';
                
                // 添加显示动画
                quickNavPanel.style.opacity = '0';
                quickNavPanel.style.transform = 'translateY(-10px)';
                setTimeout(() => {
                    quickNavPanel.style.transition = 'all 0.3s ease';
                    quickNavPanel.style.opacity = '1';
                    quickNavPanel.style.transform = 'translateY(0)';
                }, 10);
            }
            
            function hideQuickNav() {
                if (quickNavPanel) {
                    quickNavPanel.style.opacity = '0';
                    quickNavPanel.style.transform = 'translateY(-10px)';
                    setTimeout(() => {
                        quickNavPanel.style.display = 'none';
                        toggleQuickNavBtn.innerHTML = '<i class="fas fa-compass"></i> 快速导航';
                    }, 300);
                }
            }
            
            function createQuickNavPanel() {
                quickNavPanel = document.createElement('div');
                quickNavPanel.className = 'quick-nav-panel';
                quickNavPanel.style.display = 'none';
                
                const sections = [
                    { id: 'overview-section', name: '📊 总览', icon: 'fas fa-chart-bar' },
                    { id: 'test-overview-section', name: '📋 测试概览', icon: 'fas fa-list-check' },
                    { id: 'failed-tests-section', name: '❌ 失败测试', icon: 'fas fa-times-circle' },
                    { id: 'test-details-section', name: '🔍 测试详情', icon: 'fas fa-search' },
                    { id: 'coverage-overview-section', name: '📈 覆盖率', icon: 'fas fa-chart-pie' },
                    { id: 'coverage-details-section', name: '📁 模块覆盖率', icon: 'fas fa-folder-open' },
                    { id: 'build-info-section', name: '🔨 构建信息', icon: 'fas fa-hammer' }
                ];
                
                let panelHTML = `
                    <div class="quick-nav-header">
                        <div class="d-flex justify-content-between align-items-center">
                            <h6 class="mb-0">🧭 快速导航</h6>
                            <button class="btn btn-sm btn-outline-secondary" id="closeQuickNav">
                                <i class="fas fa-times"></i>
                            </button>
                        </div>
                    </div>
                `;
                
                sections.forEach(section => {
                    const element = document.getElementById(section.id);
                    if (element) {
                        panelHTML += `
                            <div class="quick-nav-item" data-target="${section.id}">
                                <div class="d-flex align-items-center">
                                    <span class="me-2">${section.name}</span>
                                </div>
                            </div>
                        `;
                    }
                });
                
                quickNavPanel.innerHTML = panelHTML;
                document.body.appendChild(quickNavPanel);
                
                // 绑定快速导航点击事件
                const quickNavItems = quickNavPanel.querySelectorAll('.quick-nav-item');
                quickNavItems.forEach(item => {
                    item.addEventListener('click', function() {
                        const targetId = this.getAttribute('data-target');
                        const targetElement = document.getElementById(targetId);
                        
                        if (targetElement) {
                            targetElement.scrollIntoView({
                                behavior: 'smooth',
                                block: 'start'
                            });
                            
                            // 高亮目标元素
                            targetElement.classList.add('highlight');
                            setTimeout(() => {
                                targetElement.classList.remove('highlight');
                            }, 2000);
                            
                            // 在移动设备上自动关闭导航
                            if (window.innerWidth <= 768) {
                                setTimeout(hideQuickNav, 500);
                            }
                        }
                    });
                });
                
                // 关闭按钮事件
                const closeBtn = quickNavPanel.querySelector('#closeQuickNav');
                closeBtn.addEventListener('click', hideQuickNav);
            }
            
            // 模块导航功能
            const toggleModuleNavBtn = document.getElementById('toggleModuleNavigation');
            const closeModuleNavBtn = document.getElementById('closeModuleNavigation');
            const moduleNavigation = document.getElementById('moduleNavigation');
            const moduleSearch = document.getElementById('moduleSearch');
            
            // 显示/隐藏模块导航
            if (toggleModuleNavBtn) {
                toggleModuleNavBtn.addEventListener('click', function() {
                    if (moduleNavigation.style.display === 'none' || !moduleNavigation.style.display) {
                        moduleNavigation.style.display = 'block';
                        toggleModuleNavBtn.innerHTML = '<i class="fas fa-times"></i>';
                        toggleModuleNavBtn.setAttribute('title', 'Close Navigation');
                        
                        // 添加显示动画
                        moduleNavigation.style.opacity = '0';
                        moduleNavigation.style.transform = 'translateY(-50%) scale(0.9)';
                        setTimeout(() => {
                            moduleNavigation.style.transition = 'all 0.3s ease';
                            moduleNavigation.style.opacity = '1';
                            moduleNavigation.style.transform = 'translateY(-50%) scale(1)';
                        }, 10);
                    } else {
                        hideModuleNavigation();
                    }
                });
            }
            
            // 关闭模块导航
            if (closeModuleNavBtn) {
                closeModuleNavBtn.addEventListener('click', hideModuleNavigation);
            }
            
            function hideModuleNavigation() {
                moduleNavigation.style.opacity = '0';
                moduleNavigation.style.transform = 'translateY(-50%) scale(0.9)';
                setTimeout(() => {
                    moduleNavigation.style.display = 'none';
                    toggleModuleNavBtn.innerHTML = '<i class="fas fa-list"></i>';
                    toggleModuleNavBtn.setAttribute('title', 'Module Navigation');
                }, 300);
            }
            
            // 点击模块导航项跳转到对应模块
            const navItems = document.querySelectorAll('.nav-item');
            navItems.forEach(function(item) {
                item.addEventListener('click', function() {
                    const targetId = item.getAttribute('data-target');
                    const targetElement = document.getElementById(targetId);
                    
                    if (targetElement) {
                        // 平滑滚动到目标
                        targetElement.scrollIntoView({
                            behavior: 'smooth',
                            block: 'start'
                        });
                        
                        // 添加高亮效果
                        targetElement.classList.add('highlight');
                        setTimeout(() => {
                            targetElement.classList.remove('highlight');
                        }, 2000);
                        
                        // 在移动设备上自动关闭导航
                        if (window.innerWidth <= 768) {
                            setTimeout(hideModuleNavigation, 500);
                        }
                    }
                });
            });
            
            // 模块搜索功能
            if (moduleSearch) {
                moduleSearch.addEventListener('input', function() {
                    const searchTerm = this.value.toLowerCase().trim();
                    
                    navItems.forEach(function(item) {
                        const searchData = item.getAttribute('data-search');
                        const category = item.closest('.nav-category');
                        
                        if (searchTerm === '' || searchData.includes(searchTerm)) {
                            item.classList.remove('hidden');
                        } else {
                            item.classList.add('hidden');
                        }
                    });
                    
                    // 隐藏空分类
                    const categories = document.querySelectorAll('.nav-category');
                    categories.forEach(function(category) {
                        const visibleItems = category.querySelectorAll('.nav-item:not(.hidden)');
                        if (visibleItems.length === 0 && searchTerm !== '') {
                            category.style.display = 'none';
                        } else {
                            category.style.display = 'block';
                        }
                    });
                });
            }
            
            // 键盘快捷键
            document.addEventListener('keydown', function(e) {
                // ESC键关闭导航
                if (e.key === 'Escape') {
                    if (moduleNavigation && moduleNavigation.style.display !== 'none') {
                        hideModuleNavigation();
                    }
                    if (quickNavPanel && quickNavPanel.style.display !== 'none') {
                        hideQuickNav();
                    }
                }
            });
            
            // 点击外部关闭导航
            document.addEventListener('click', function(e) {
                if (moduleNavigation && moduleNavigation.style.display !== 'none' && 
                    !moduleNavigation.contains(e.target) && 
                    !toggleModuleNavBtn.contains(e.target)) {
                    hideModuleNavigation();
                }
                
                if (quickNavPanel && quickNavPanel.style.display !== 'none' &&
                    !quickNavPanel.contains(e.target) &&
                    !toggleQuickNavBtn.contains(e.target)) {
                    hideQuickNav();
                }
            });
        });
        """ 