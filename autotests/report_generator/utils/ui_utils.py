#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
UI utility functions for test report generation
"""


def get_module_icon(module_name: str) -> str:
    """Return appropriate icon based on module name"""
    name_lower = module_name.lower()
    
    # Plugin icons
    if name_lower.startswith("plugin:"):
        plugin_name = name_lower[7:].strip()  # Remove "plugin:" prefix
        
        # Specific plugin icons
        if "canvas" in plugin_name:
            return "🖼️"  # Canvas/Desktop
        elif "organizer" in plugin_name:
            return "📂"  # File organizer
        elif "wallpaper" in plugin_name:
            return "🌅"  # Wallpaper
        elif "background" in plugin_name:
            return "🎨"  # Background
        elif "menu" in plugin_name:
            return "📋"  # Menu
        elif "search" in plugin_name:
            return "🔍"  # Search
        elif "bookmark" in plugin_name:
            return "⭐"  # Bookmark
        elif "recent" in plugin_name:
            return "⏰"  # Recent
        elif "trash" in plugin_name:
            return "🗑️"  # Trash
        elif "burn" in plugin_name or "optical" in plugin_name:
            return "💿"  # Optical/Burn
        elif "vault" in plugin_name or "encrypt" in plugin_name:
            return "🔐"  # Encryption/Vault
        elif "computer" in plugin_name:
            return "💻"  # Computer
        elif "sidebar" in plugin_name:
            return "📑"  # Sidebar
        elif "titlebar" in plugin_name:
            return "📊"  # Title bar
        elif "property" in plugin_name:
            return "⚙️"  # Properties
        elif "file" in plugin_name and "operation" in plugin_name:
            return "✂️"  # File operations
        elif "share" in plugin_name or "smb" in plugin_name:
            return "🌐"  # Network/Share
        elif "tag" in plugin_name:
            return "🏷️"  # Tags
        elif "emblem" in plugin_name:
            return "🎖️"  # Emblems
        elif "avfs" in plugin_name:
            return "📁"  # Archive browser
        else:
            return "🔌"  # Generic plugin
    
    # Service icons
    elif name_lower.startswith("service:"):
        service_name = name_lower[8:].strip()  # Remove "service:" prefix
        
        if "access" in service_name and "control" in service_name:
            return "🛡️"  # Access control
        elif "share" in service_name and "control" in service_name:
            return "🔗"  # Share control
        elif "mount" in service_name and "control" in service_name:
            return "📀"  # Mount control
        elif "disk" in service_name and "encrypt" in service_name:
            return "🔒"  # Disk encryption
        elif "text" in service_name and "index" in service_name:
            return "📝"  # Text indexing
        else:
            return "⚡"  # Generic service
    
    # Test modules
    elif name_lower.startswith("test:") or name_lower.startswith("autotest:"):
        return "🧪"  # Test modules
    
    # Include modules
    elif name_lower.startswith("include:"):
        return "📋"  # Include files
    
    # Core framework modules
    elif "dfm" in name_lower:
        if "base" in name_lower:
            return "🏗️"  # Base framework
        elif "framework" in name_lower:
            return "🔧"  # Framework
        elif "extension" in name_lower:
            return "🧩"  # Extensions
        else:
            return "⚙️"  # Generic DFM module
    
    # File dialog
    elif "dialog" in name_lower:
        return "💬"  # Dialog
    
    # Tools
    elif "tool" in name_lower:
        return "🔨"  # Tools
    
    # Default icon
    else:
        return "📦"  # Generic module


def get_coverage_color(percentage: float) -> str:
    """Return color based on coverage percentage"""
    if percentage >= 80:
        return "#28a745"  # Green
    elif percentage >= 60:
        return "#ffc107"  # Yellow
    elif percentage >= 40:
        return "#fd7e14"  # Orange
    else:
        return "#dc3545"  # Red


def get_coverage_class(coverage: float) -> str:
    """Return Bootstrap class based on coverage percentage"""
    if coverage >= 90:
        return "badge-success"
    elif coverage >= 80:
        return "badge-primary"
    elif coverage >= 70:
        return "badge-warning"
    elif coverage >= 50:
        return "badge-secondary"
    else:
        return "badge-danger"


def get_section_icon(section_name: str) -> str:
    """Return appropriate icon for report sections"""
    section_lower = section_name.lower()
    
    if "test" in section_lower and ("result" in section_lower or "overview" in section_lower):
        return "📊"  # Test overview
    elif "failed" in section_lower and "test" in section_lower:
        return "❌"  # Failed tests
    elif "test" in section_lower and ("detail" in section_lower or "execution" in section_lower):
        return "🔍"  # Test details
    elif "coverage" in section_lower:
        return "📁"  # Coverage details
    elif "build" in section_lower:
        return "🔨"  # Build info
    else:
        return "📋"  # Generic section 