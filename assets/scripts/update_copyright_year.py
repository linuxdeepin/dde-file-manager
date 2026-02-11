#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Copyright Year Update Script

This script updates the SPDX-FileCopyrightText year in source files to the latest year.
It scans .h and .cpp files in specified directories and updates the copyright year.

Usage:
    python3 update_copyright_year.py [--dry-run] [--year YEAR]

Options:
    --dry-run    Show what would be changed without making actual changes
    --year YEAR  Target year (default: current year)
"""

import os
import re
import sys
import argparse
from datetime import datetime
from pathlib import Path
from typing import List, Dict, Tuple


class CopyrightUpdater:
    """Handles copyright year updates in source files."""

    # Directories to scan
    TARGET_DIRS = [
        "include",
        "src",
        "tests",
        "examples",
        "autotests"
    ]

    # File extensions to process
    FILE_EXTENSIONS = {".h", ".cpp", ".cc", ".cxx"}

    # SPDX copyright patterns
    COPYRIGHT_PATTERNS = [
        r'// SPDX-FileCopyrightText:\s*(\d{4})(?:\s*-\s*(\d{4}))?',
        r'// SPDX-FileCopyrightText:\s*(\d{4})(?:\s*,\s*(\d{4}))?',
    ]

    def __init__(self, base_dir: str, target_year: int, dry_run: bool = False):
        """
        Initialize the copyright updater.

        Args:
            base_dir: Base directory of the project
            target_year: Target year to update to
            dry_run: If True, only show what would be changed
        """
        self.base_dir = Path(base_dir).resolve()
        self.target_year = target_year
        self.dry_run = dry_run
        self.report = {
            "total_files": 0,
            "scanned_files": 0,
            "updated_files": [],
            "skipped_files": [],
            "no_copyright_files": [],
            "error_files": []
        }

    def find_source_files(self) -> List[Path]:
        """
        Find all source files in target directories.

        Returns:
            List of file paths
        """
        source_files = []

        for target_dir in self.TARGET_DIRS:
            dir_path = self.base_dir / target_dir
            if not dir_path.exists():
                print(f"Warning: Directory {target_dir} does not exist, skipping...")
                continue

            for ext in self.FILE_EXTENSIONS:
                # Use rglob to find all files with the extension recursively
                source_files.extend(dir_path.rglob(f"*{ext}"))

        return sorted(source_files)

    def parse_copyright_year(self, content: str) -> Tuple[int, int, bool]:
        """
        Parse copyright year from file content.

        Args:
            content: File content

        Returns:
            Tuple of (start_year, end_year, has_copyright)
            end_year is None if only single year exists
            has_copyright is True if copyright found
        """
        # Look for copyright in the first 20 lines
        lines = content.split('\n')[:20]

        for line in lines:
            # Try pattern: 2022 - 2023 or 2023
            match = re.search(r'SPDX-FileCopyrightText:.*?(\d{4})(?:\s*[-–—]\s*(\d{4}))?', line)
            if match:
                start_year = int(match.group(1))
                end_year = int(match.group(2)) if match.group(2) else None
                return start_year, end_year, True

        return None, None, False

    def needs_update(self, start_year: int, end_year: int) -> bool:
        """
        Check if copyright needs updating.

        Args:
            start_year: Start year from copyright
            end_year: End year from copyright (None if single year)

        Returns:
            True if update is needed
        """
        if end_year is None:
            # Single year case: 2023 -> update if not target year
            return start_year != self.target_year
        else:
            # Year range case: 2022 - 2023 -> update if end year is not target year
            return end_year != self.target_year

    def update_copyright_line(self, line: str, start_year: int, end_year: int = None) -> str:
        """
        Update copyright line to new year.

        Args:
            line: Original copyright line
            start_year: Original start year
            end_year: Original end year (None if single year)

        Returns:
            Updated copyright line
        """
        if end_year is None:
            # Single year: convert to range with target year
            # Example: "2023 UnionTech" -> "2023 - 2026 UnionTech"
            return re.sub(
                r'(\d{4})',
                rf'\g<1> - {self.target_year}',
                line,
                count=1
            )
        else:
            # Year range: update end year
            # Pattern: "2022 - 2023" or "2022 - 2023"
            pattern = r'(\d{4}\s*[-–—]\s*)(\d{4})'
            replacement = rf'\g<1>{self.target_year}'
            return re.sub(pattern, replacement, line, count=1)

    def process_file(self, file_path: Path) -> bool:
        """
        Process a single file and update copyright if needed.

        Args:
            file_path: Path to the file

        Returns:
            True if file was updated, False otherwise
        """
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()

            start_year, end_year, has_copyright = self.parse_copyright_year(content)

            if not has_copyright:
                self.report["no_copyright_files"].append(str(file_path))
                return False

            if not self.needs_update(start_year, end_year):
                self.report["skipped_files"].append(str(file_path))
                return False

            # Need to update
            lines = content.split('\n')
            updated_lines = []

            for line in lines:
                if 'SPDX-FileCopyrightText:' in line:
                    updated_line = self.update_copyright_line(line, start_year, end_year)
                    updated_lines.append(updated_line)
                else:
                    updated_lines.append(line)

            new_content = '\n'.join(updated_lines)

            if not self.dry_run:
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(new_content)

            self.report["updated_files"].append(str(file_path))
            return True

        except Exception as e:
            self.report["error_files"].append({
                "file": str(file_path),
                "error": str(e)
            })
            return False

    def run(self) -> Dict:
        """
        Run the copyright update process.

        Returns:
            Report dictionary
        """
        print(f"{'=' * 70}")
        print(f"Copyright Year Update Script")
        print(f"{'=' * 70}")
        print(f"Base directory: {self.base_dir}")
        print(f"Target year: {self.target_year}")
        print(f"Dry run: {self.dry_run}")
        print(f"{'=' * 70}\n")

        # Find all source files
        source_files = self.find_source_files()
        self.report["total_files"] = len(source_files)

        print(f"Found {len(source_files)} source files to scan\n")

        # Process each file
        for i, file_path in enumerate(source_files, 1):
            self.report["scanned_files"] += 1
            relative_path = file_path.relative_to(self.base_dir)

            if self.process_file(file_path):
                status = "✓ UPDATED"
            else:
                status = "○ SKIPPED"

            print(f"[{i}/{len(source_files)}] {status} {relative_path}")

        return self.report

    def generate_report(self) -> str:
        """
        Generate a detailed report.

        Returns:
            Report text
        """
        report_lines = [
            "",
            "=" * 70,
            "COPYRIGHT YEAR UPDATE REPORT",
            "=" * 70,
            f"Base directory: {self.base_dir}",
            f"Target year: {self.target_year}",
            f"Dry run: {self.dry_run}",
            f"Execution time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
            "=" * 70,
            "",
            "SUMMARY:",
            f"  Total files scanned: {self.report['scanned_files']}",
            f"  Files updated: {len(self.report['updated_files'])}",
            f"  Files skipped (already up-to-date): {len(self.report['skipped_files'])}",
            f"  Files without copyright: {len(self.report['no_copyright_files'])}",
            f"  Files with errors: {len(self.report['error_files'])}",
            "",
        ]

        if self.report['updated_files']:
            report_lines.extend([
                "UPDATED FILES:",
                "-" * 70
            ])
            for file_path in self.report['updated_files']:
                report_lines.append(f"  ✓ {file_path}")
            report_lines.append("")

        if self.report['no_copyright_files']:
            report_lines.extend([
                "FILES WITHOUT COPYRIGHT NOTICE:",
                "-" * 70
            ])
            for file_path in self.report['no_copyright_files']:
                report_lines.append(f"  ! {file_path}")
            report_lines.append("")

        if self.report['error_files']:
            report_lines.extend([
                "FILES WITH ERRORS:",
                "-" * 70
            ])
            for error_info in self.report['error_files']:
                report_lines.append(f"  ✗ {error_info['file']}")
                report_lines.append(f"    Error: {error_info['error']}")
            report_lines.append("")

        report_lines.extend([
            "=" * 70,
            "END OF REPORT",
            "=" * 70
        ])

        return '\n'.join(report_lines)


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description='Update copyright year in source files',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Show what would be changed without making actual changes'
    )
    parser.add_argument(
        '--year',
        type=int,
        default=datetime.now().year,
        help=f'Target year (default: {datetime.now().year})'
    )
    parser.add_argument(
        '--output',
        type=str,
        default=None,
        help='Report output path (default: /tmp/copyright_update_REPORT_YYYYMMDD_HHMMSS.txt)'
    )

    args = parser.parse_args()

    # Get base directory (script location's parent)
    script_dir = Path(__file__).parent.resolve()
    base_dir = script_dir.parent.parent  # Go up to project root

    # Create updater
    updater = CopyrightUpdater(
        base_dir=base_dir,
        target_year=args.year,
        dry_run=args.dry_run
    )

    # Run update
    updater.run()

    # Generate report
    report_text = updater.generate_report()

    # Print report to console
    print(report_text)

    # Save report to file
    if args.output:
        report_path = args.output
    else:
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        report_path = f"/tmp/copyright_update_report_{timestamp}.txt"

    with open(report_path, 'w', encoding='utf-8') as f:
        f.write(report_text)

    print(f"\nReport saved to: {report_path}")

    # Return exit code
    if updater.report['error_files']:
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
