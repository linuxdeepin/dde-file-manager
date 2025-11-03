#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
CSV report generator for module coverage statistics
"""

import csv
from pathlib import Path
from typing import Dict, List


class CsvReportGenerator:
    """Generator for CSV format coverage reports"""

    def __init__(self, report_dir: Path):
        """
        Initialize CSV report generator

        Args:
            report_dir: Directory to save the CSV report
        """
        self.report_dir = report_dir

    def generate_coverage_csv(self, coverage_info: Dict, output_filename: str = "coverage_report.csv") -> bool:
        """
        Generate CSV report from coverage data

        Args:
            coverage_info: Coverage data dictionary containing tree_structure
            output_filename: Output CSV filename

        Returns:
            bool: True if generation succeeded, False otherwise
        """
        try:
            # Extract module coverage data
            tree_structure = coverage_info.get("tree_structure", {})
            if not tree_structure:
                print("⚠️ No module coverage data found")
                return False

            # Prepare CSV data
            csv_data = self._prepare_csv_data(tree_structure)

            # Sort by module name for better readability
            csv_data.sort(key=lambda x: x["module_name"])

            # Write to CSV file
            output_path = self.report_dir / output_filename
            with open(output_path, 'w', newline='', encoding='utf-8') as csvfile:
                fieldnames = [
                    "module_name",
                    "line_coverage",
                    "function_coverage",
                    "lines_hit",
                    "lines_found",
                    "functions_hit",
                    "functions_found",
                    "file_count"
                ]

                writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
                writer.writeheader()
                writer.writerows(csv_data)

            print(f"✅ CSV report generated: {output_path}")
            return True

        except Exception as e:
            print(f"❌ Error generating CSV report: {e}")
            import traceback
            traceback.print_exc()
            return False

    def _prepare_csv_data(self, tree_structure: Dict) -> List[Dict]:
        """
        Prepare CSV data from tree structure

        Args:
            tree_structure: Module coverage tree structure

        Returns:
            List of dictionaries containing module coverage data
        """
        csv_data = []

        for module_name, module_data in tree_structure.items():
            stats = module_data.get("stats", {})

            csv_row = {
                "module_name": module_name,
                "line_coverage": f"{stats.get('line_coverage', 0.0):.2f}%",
                "function_coverage": f"{stats.get('function_coverage', 0.0):.2f}%",
                "lines_hit": stats.get('lines_hit', 0),
                "lines_found": stats.get('lines_found', 0),
                "functions_hit": stats.get('functions_hit', 0),
                "functions_found": stats.get('functions_found', 0),
                "file_count": len(module_data.get("files", []))
            }

            csv_data.append(csv_row)

        return csv_data
