// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file dfm_extension_stub.cpp
 * @brief Stub implementation for dfm-extension global functions used in unit tests
 * 
 * This file provides stub implementations of the dfm-extension global API functions
 * that are declared in dfm-extension.h but are intended to be implemented by 
 * extension plugins. For unit testing purposes, we provide default implementations
 * that allow the tests to compile and run.
 */

#include <dfm-extension/dfm-extension.h>

USING_DFMEXT_NAMESPACE

// Global plugin instances for testing
static DFMExtMenuPlugin *g_testMenuPlugin = nullptr;
static DFMExtEmblemIconPlugin *g_testEmblemPlugin = nullptr;
static DFMExtWindowPlugin *g_testWindowPlugin = nullptr;
static DFMExtFilePlugin *g_testFilePlugin = nullptr;

// Global initialization state
static bool g_testInitialized = false;

extern "C" void dfm_extension_initialize()
{
    if (g_testInitialized)
        return;
    
    // Initialize global plugin instances for testing
    if (!g_testMenuPlugin)
        g_testMenuPlugin = new DFMExtMenuPlugin();
    if (!g_testEmblemPlugin)
        g_testEmblemPlugin = new DFMExtEmblemIconPlugin();
    if (!g_testWindowPlugin)
        g_testWindowPlugin = new DFMExtWindowPlugin();
    if (!g_testFilePlugin)
        g_testFilePlugin = new DFMExtFilePlugin();
    
    g_testInitialized = true;
}

// Legacy misspelled function name for backward compatibility
extern "C" void dfm_extension_initiliaze()
{
    dfm_extension_initialize();
}

extern "C" void dfm_extension_shutdown()
{
    if (!g_testInitialized)
        return;
    
    // Clean up global plugin instances
    delete g_testMenuPlugin;
    g_testMenuPlugin = nullptr;
    
    delete g_testEmblemPlugin;
    g_testEmblemPlugin = nullptr;
    
    delete g_testWindowPlugin;
    g_testWindowPlugin = nullptr;
    
    delete g_testFilePlugin;
    g_testFilePlugin = nullptr;
    
    g_testInitialized = false;
}

extern "C" DFMExtMenuPlugin *dfm_extension_menu()
{
    return g_testMenuPlugin;
}

extern "C" DFMExtEmblemIconPlugin *dfm_extension_emblem()
{
    return g_testEmblemPlugin;
}

extern "C" DFMExtWindowPlugin *dfm_extension_window()
{
    return g_testWindowPlugin;
}

extern "C" DFMExtFilePlugin *dfm_extension_file()
{
    return g_testFilePlugin;
} 