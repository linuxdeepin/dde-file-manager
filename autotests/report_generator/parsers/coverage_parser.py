#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Coverage data parser for DDE File Manager test reports
"""

import os
import re
from pathlib import Path
from typing import Dict, List

from ..utils.file_utils import (
    is_source_file, extract_module_name, get_coverage_html_path
)


class CoverageParser:
    """Parser for code coverage data"""
    
    def __init__(self, build_dir: Path, report_dir: Path, project_root: Path):
        self.build_dir = build_dir
        self.report_dir = report_dir
        self.project_root = project_root
    
    def parse_coverage_data(self, coverage_success: bool, coverage_duration: int) -> Dict:
        """Parse coverage data from lcov output"""
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
            
        # Try to parse lcov output
        coverage_output_file = self.report_dir / "coverage_output.log"
        if coverage_output_file.exists():
            try:
                with open(coverage_output_file, 'r', encoding='utf-8') as f:
                    content = f.read()
                    
                # Parse coverage percentages
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
                print(f"Error parsing coverage output: {e}")
        
        # Try to parse lcov info file
        coverage_info_file = self.build_dir / "coverage" / "filtered.info"
        if coverage_info_file.exists():
            try:
                coverage_info.update(self._parse_lcov_info(coverage_info_file))
            except Exception as e:
                print(f"Error parsing lcov info file: {e}")
                
        return coverage_info
    
    def _parse_lcov_info(self, info_file: Path) -> Dict:
        """Parse lcov info file"""
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
                        # Calculate file coverage
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
            print(f"Error parsing lcov info file content: {e}")
            
        return {
            "details": details,
            "files_covered": len([d for d in details if d["lines_hit"] > 0]),
            "total_files": len(details),
            "tree_structure": self._build_coverage_tree(details)
        }
    
    def _build_coverage_tree(self, details: List[Dict]) -> Dict:
        """Build detailed module coverage structure: support fine-grained plugin and service splitting"""
        # Group files by module
        modules = {}
        
        for detail in details:
            file_path = detail["file"]
            # Remove project root prefix to get relative path
            if file_path.startswith(str(self.project_root)):
                relative_path = file_path[len(str(self.project_root)):].lstrip('/')
            else:
                relative_path = file_path
            
            # Skip non-source files
            if not is_source_file(relative_path):
                continue
            
            # Smart module name parsing
            module_name = extract_module_name(relative_path)
            
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
            
            # Add file to module
            modules[module_name]["files"].append({
                "name": os.path.basename(relative_path),
                "path": relative_path,
                "coverage_html_path": get_coverage_html_path(relative_path, self.build_dir),
                "stats": {
                    "lines_found": detail["lines_found"],
                    "lines_hit": detail["lines_hit"],
                    "functions_found": detail["functions_found"],
                    "functions_hit": detail["functions_hit"],
                    "line_coverage": detail["line_coverage"],
                    "function_coverage": detail["function_coverage"]
                }
            })
            
            # Accumulate module statistics
            modules[module_name]["stats"]["lines_found"] += detail["lines_found"]
            modules[module_name]["stats"]["lines_hit"] += detail["lines_hit"]
            modules[module_name]["stats"]["functions_found"] += detail["functions_found"]
            modules[module_name]["stats"]["functions_hit"] += detail["functions_hit"]
        
        # Calculate coverage for each module
        for module in modules.values():
            stats = module["stats"]
            if stats["lines_found"] > 0:
                stats["line_coverage"] = (stats["lines_hit"] / stats["lines_found"]) * 100
            if stats["functions_found"] > 0:
                stats["function_coverage"] = (stats["functions_hit"] / stats["functions_found"]) * 100
            
            # Sort files by coverage
            module["files"].sort(key=lambda x: x["stats"]["line_coverage"], reverse=True)
        
        return modules 