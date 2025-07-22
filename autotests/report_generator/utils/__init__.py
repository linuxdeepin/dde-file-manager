#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Utilities package for test report generation
"""

from .file_utils import (
    is_source_file, sanitize_id, get_coverage_html_path,
    extract_module_name, format_plugin_name, format_service_name, format_module_name
)
from .ui_utils import (
    get_module_icon, get_coverage_color, get_coverage_class, get_section_icon
)

__all__ = [
    'is_source_file', 'sanitize_id', 'get_coverage_html_path',
    'extract_module_name', 'format_plugin_name', 'format_service_name', 'format_module_name',
    'get_module_icon', 'get_coverage_color', 'get_coverage_class', 'get_section_icon'
] 