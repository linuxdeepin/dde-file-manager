// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extensionpluginloader.h"

DPUTILS_BEGIN_NAMESPACE

ExtensionPluginLoader::ExtensionPluginLoader(const QString &filaName, QObject *parent)
    : QObject(parent)
{
    loader.setFileName(filaName);
}

QString ExtensionPluginLoader::fileName() const
{
    return loader.fileName();
}

QString ExtensionPluginLoader::lastError() const
{
    return errorMessage;
}

bool ExtensionPluginLoader::loadPlugin()
{
    if (loader.fileName().isEmpty()) {
        errorMessage = "Failed, plugin file name is empty";
        return false;
    }

    if (!loader.load()) {
        errorMessage = loader.errorString();
        return false;
    }

    return true;
}

bool ExtensionPluginLoader::initialize()
{
    if (!loader.isLoaded()) {
        errorMessage = "Plugin haven't loaded";
        return false;
    }

    // Try to resolve the correctly spelled function name first
    initFunc = reinterpret_cast<ExtInitFuncType>(loader.resolve("dfm_extension_initialize"));
    
    // If not found, try the legacy misspelled function name for backward compatibility
    if (!initFunc) {
        initFunc = reinterpret_cast<ExtInitFuncType>(loader.resolve("dfm_extension_initiliaze"));
        if (initFunc) {
            // Log deprecation warning for the misspelled function name
            qWarning() << "Warning: Plugin" << loader.fileName() 
                      << "uses deprecated function name 'dfm_extension_initiliaze'. "
                      << "Please update to 'dfm_extension_initialize'";
        }
    }
    
    if (!initFunc) {
        errorMessage = "Failed, get 'dfm_extension_initialize' or 'dfm_extension_initiliaze' import function: " + loader.fileName();
        return false;
    }

    initFunc();
    return true;
}

bool ExtensionPluginLoader::shutdown()
{
    shutdownFunc = reinterpret_cast<ExtShutdownFuncType>(loader.resolve("dfm_extension_shutdown"));
    if (!shutdownFunc) {
        errorMessage = "Failed, get 'dfm_extension_shutdown' import function: "
                + loader.fileName();
        return false;
    }

    //! delete interaface
    shutdownFunc();

    if (!loader.isLoaded()) {
        errorMessage = "Plugin has been shutdown: " + loader.fileName();
        return false;
    }

    if (!loader.unload())
        errorMessage = loader.errorString();

    return true;
}

DFMEXT::DFMExtMenuPlugin *ExtensionPluginLoader::resolveMenuPlugin()
{
    if (!loader.isLoaded()) {
        errorMessage = "Failed, called 'resolveMenuPlugin' get interface, "
                       "need call 'initialize' function befor that";
        return {};
    }

    menuFunc = reinterpret_cast<ExtMenuFuncType>(loader.resolve("dfm_extension_menu"));
    if (!menuFunc) {
        errorMessage = "Failed, get 'dfm_extension_menu' import function";
        return {};
    }

    return menuFunc();
}

DFMEXT::DFMExtEmblemIconPlugin *ExtensionPluginLoader::resolveEmblemPlugin()
{
    if (!loader.isLoaded()) {
        errorMessage = "Failed, called 'resolveEmblemPlugin' get interface, "
                       "need call 'initialize' function befor that";
        return {};
    }

    emblemFunc = reinterpret_cast<ExtEmblemFuncType>(loader.resolve("dfm_extension_emblem"));
    if (!emblemFunc) {
        errorMessage = "Failed, get 'dfm_extension_emblem' import function: "
                + loader.fileName();
        return {};
    }

    return emblemFunc();
}

DFMEXT::DFMExtWindowPlugin *ExtensionPluginLoader::resolveWindowPlugin()
{
    if (!loader.isLoaded()) {
        errorMessage = "Failed, called 'resolveWindowPlugin' get interface, "
                       "need call 'initialize' function befor that";
        return {};
    }

    windowFunc = reinterpret_cast<ExtWindowFuncType>(loader.resolve("dfm_extension_window"));
    if (!windowFunc) {
        errorMessage = "Failed, get 'dfm_extension_window' import function";
        return {};
    }

    return windowFunc();
}

DFMEXT::DFMExtFilePlugin *ExtensionPluginLoader::resolveFilePlugin()
{
    if (!loader.isLoaded()) {
        errorMessage = "Failed, called 'resolveFilePlugin' get interface, "
                       "need call 'initialize' function befor that";
        return {};
    }

    fileFunc = reinterpret_cast<ExtFileFuncType>(loader.resolve("dfm_extension_file"));
    if (!fileFunc) {
        errorMessage = "Failed, get 'dfm_extension_file' import function";
        return {};
    }

    return fileFunc();
}

DPUTILS_END_NAMESPACE
