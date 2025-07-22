#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Parsers package for test report generation
"""

from .test_parser import TestOutputParser
from .coverage_parser import CoverageParser

__all__ = ['TestOutputParser', 'CoverageParser'] 