#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Generators package for test report generation
"""

from .html_generator import HtmlReportGenerator
from .csv_generator import CsvReportGenerator

__all__ = ['HtmlReportGenerator', 'CsvReportGenerator'] 