#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""
File utility functions for test report generation
"""

import os
import re
from pathlib import Path
from typing import Dict, List


def is_source_file(file_path: str) -> bool:
    """Check if a file is a source code file"""
    source_extensions = ['.cpp', '.cc', '.cxx', '.c', '.h', '.hpp', '.hxx']
    return any(file_path.endswith(ext) for ext in source_extensions)


def sanitize_id(name: str) -> str:
    """Convert module name to valid HTML ID"""
    # Remove special characters, keep only letters, numbers, and hyphens
    sanitized = re.sub(r'[^a-zA-Z0-9\-\s]', '', name)
    # Replace spaces with hyphens
    sanitized = re.sub(r'\s+', '-', sanitized)
    # Convert to lowercase
    return sanitized.lower().strip('-')


def get_coverage_html_path(relative_path: str, build_dir: Path) -> str:
    """Generate the coverage HTML file path for a given source file"""
    path_parts = relative_path.split('/')
    
    if len(path_parts) < 2:
        return None
    
    # Generate coverage HTML path based on the build directory structure
    coverage_base = build_dir / "coverage" / "html"
    
    # Handle different directory structures
    if path_parts[0] == "src":
        if len(path_parts) >= 3:
            if path_parts[1] == "plugins":
                if len(path_parts) >= 4:
                    # src/plugins/category/plugin-name/file.cpp -> plugins/category/plugin-name/file.cpp.gcov.html
                    plugin_category = path_parts[2]  # common, filemanager, desktop, etc.
                    plugin_name = path_parts[3]
                    remaining_path = '/'.join(path_parts[4:]) if len(path_parts) > 4 else os.path.basename(relative_path)
                    coverage_path = coverage_base / "plugins" / plugin_category / plugin_name / f"{remaining_path}.gcov.html"
                elif len(path_parts) >= 3:
                    # src/plugins/plugin-name/file.cpp (fallback for direct structure)
                    plugin_name = path_parts[2]
                    
                    # Determine plugin category based on plugin name
                    if plugin_name.startswith("dfmplugin-"):
                        plugin_category = "filemanager"
                    elif plugin_name.startswith("ddplugin-"):
                        plugin_category = "desktop"
                    elif plugin_name.startswith("dfmdaemon-"):
                        plugin_category = "daemon"
                    elif plugin_name.startswith("filedialog-"):
                        plugin_category = "filedialog"
                    else:
                        plugin_category = "common"
                    
                    # Generate coverage path with proper category
                    remaining_path = '/'.join(path_parts[3:]) if len(path_parts) > 3 else os.path.basename(relative_path)
                    coverage_path = coverage_base / "plugins" / plugin_category / plugin_name / f"{remaining_path}.gcov.html"
            elif path_parts[1] == "services":
                # src/services/service-name/file.cpp -> services/service-name/file.cpp.gcov.html
                service_name = path_parts[2]
                file_name = os.path.basename(relative_path)
                coverage_path = coverage_base / "services" / service_name / f"{file_name}.gcov.html"
            else:
                # src/module-name/file.cpp -> module-name/file.cpp.gcov.html
                module_name = path_parts[1]
                remaining_path = '/'.join(path_parts[2:])
                coverage_path = coverage_base / module_name / f"{remaining_path}.gcov.html"
        else:
            # src/file.cpp -> file.cpp.gcov.html
            file_name = os.path.basename(relative_path)
            coverage_path = coverage_base / f"{file_name}.gcov.html"
    elif path_parts[0] == "include":
        # include/module/file.h -> include/module/file.h.gcov.html
        remaining_path = '/'.join(path_parts[1:])
        coverage_path = coverage_base / "include" / f"{remaining_path}.gcov.html"
    else:
        # Other paths: preserve structure
        coverage_path = coverage_base / f"{relative_path}.gcov.html"
    
    return str(coverage_path)


def extract_module_name(relative_path: str) -> str:
    """Smart module name extraction with fine-grained plugin and service separation"""
    path_parts = relative_path.split('/')
    
    if len(path_parts) <= 1:
        return "Root Directory"
    
    # Handle src directory structure
    if path_parts[0] == "src":
        if len(path_parts) < 2:
            return "src"
        
        second_level = path_parts[1]
        
        # Handle plugins directory: src/plugins/[category]/[plugin-name]
        if second_level == "plugins":
            if len(path_parts) >= 4:
                # src/plugins/filemanager/dfmplugin-xxx or src/plugins/desktop/ddplugin-xxx
                plugin_category = path_parts[2]  # filemanager, desktop, daemon, common, filedialog
                plugin_name = path_parts[3]
                return f"Plugin: {format_plugin_name(plugin_name)} ({plugin_category})"
            elif len(path_parts) >= 3:
                # src/plugins/category (category level files)
                plugin_category = path_parts[2]
                return f"Plugin: {plugin_category.title()}"
        
        # Handle services directory: src/services/[service-name]
        elif second_level == "services" and len(path_parts) >= 3:
            service_name = path_parts[2]
            return f"Service: {format_service_name(service_name)}"
        
        # Handle other modules under src: dfm-base, dfm-framework, etc.
        else:
            return format_module_name(second_level)
    
    # Handle tests directory structure
    elif path_parts[0] == "tests":
        if len(path_parts) < 2:
            return "Tests"
        
        second_level = path_parts[1]
        
        # Handle tests/plugins directory
        if second_level == "plugins" and len(path_parts) >= 3:
            plugin_name = path_parts[2]
            return f"Test: {format_plugin_name(plugin_name)}"
        
        # Handle tests/services directory
        elif second_level == "services" and len(path_parts) >= 3:
            service_name = path_parts[2]
            return f"Test: {format_service_name(service_name)}"
        
        # Handle other tests modules
        else:
            return f"Test: {format_module_name(second_level)}"
    
    # Handle include directory
    elif path_parts[0] == "include":
        if len(path_parts) >= 2:
            return f"Include: {format_module_name(path_parts[1])}"
        return "Include"
    
    # Handle autotests directory
    elif path_parts[0] == "autotests":
        if len(path_parts) >= 2:
            second_level = path_parts[1]
            
            # Handle autotests/plugins directory
            if second_level == "plugins" and len(path_parts) >= 3:
                plugin_name = path_parts[2]
                return f"AutoTest: {format_plugin_name(plugin_name)}"
            
            # Handle autotests/services directory
            elif second_level == "services" and len(path_parts) >= 3:
                service_name = path_parts[2]
                return f"AutoTest: {format_service_name(service_name)}"
            
            # Handle other autotests modules
            else:
                return f"AutoTest: {format_module_name(second_level)}"
        return "AutoTest"
    
    # Other top-level directories
    else:
        return format_module_name(path_parts[0])


def format_plugin_name(plugin_name: str) -> str:
    """Format plugin name to be more readable"""
    # Remove common prefixes
    if plugin_name.startswith('dfmplugin-'):
        plugin_name = plugin_name[10:]  # Remove 'dfmplugin-'
    elif plugin_name.startswith('ddplugin-'):
        plugin_name = plugin_name[9:]   # Remove 'ddplugin-'
    elif plugin_name.startswith('dfmdaemon-'):
        plugin_name = plugin_name[10:]  # Remove 'dfmdaemon-'
    
    # Replace hyphens with spaces and title case
    formatted = plugin_name.replace('-', ' ').title()
    
    # Special name mappings
    name_mappings = {
        'Avfsbrowser': 'AVFS Browser',
        'Smbbrowser': 'SMB Browser',
        'Myshares': 'My Shares',
        'Fileoperations': 'File Operations',
        'Propertydialog': 'Property Dialog',
        'Detailspace': 'Detail Space',
        'Dirshare': 'Directory Share',
        'Trashcore': 'Trash Core',
        'Titlebar': 'Title Bar',
        'Wallpapersetting': 'Wallpaper Setting',
        'Disk Encrypt Entry': 'Disk Encrypt Entry',
        'Encrypt Manager': 'Encrypt Manager',
        'Filedialog Core': 'File Dialog Core'
    }
    
    return name_mappings.get(formatted, formatted)


def format_service_name(service_name: str) -> str:
    """Format service name to be more readable"""
    # Replace hyphens with spaces and title case
    formatted = service_name.replace('-', ' ').title()
    
    # Special name mappings
    name_mappings = {
        'Accesscontrol': 'Access Control',
        'Sharecontrol': 'Share Control',
        'Mountcontrol': 'Mount Control',
        'Diskencrypt': 'Disk Encrypt',
        'Textindex': 'Text Index'
    }
    
    return name_mappings.get(formatted, formatted)


def format_module_name(module_name: str) -> str:
    """Format general module name"""
    # Replace hyphens with spaces and title case
    formatted = module_name.replace('-', ' ').title()
    
    # Special name mappings
    name_mappings = {
        'Dfm Base': 'DFM Base',
        'Dfm Framework': 'DFM Framework',
        'Dfm Extension': 'DFM Extension',
        'Filedialog Core': 'File Dialog Core'
    }
    
    return name_mappings.get(formatted, formatted) 