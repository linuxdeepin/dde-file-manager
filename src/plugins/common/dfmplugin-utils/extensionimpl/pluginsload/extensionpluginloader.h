// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONPLUGINLOADER_H
#define EXTENSIONPLUGINLOADER_H

#include "dfmplugin_utils_global.h"

// dfm-extension
#include <dfm-extension/menu/dfmextmenuplugin.h>
#include <dfm-extension/emblemicon/dfmextemblemiconplugin.h>
#include <dfm-extension/window/dfmextwindowplugin.h>
#include <dfm-extension/file/dfmextfileplugin.h>

#include <QObject>
#include <QLibrary>

DPUTILS_BEGIN_NAMESPACE

class ExtensionPluginLoader : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ExtensionPluginLoader)

public:
    using ExtInitFuncType = void (*)();
    using ExtShutdownFuncType = void (*)();
    using ExtMenuFuncType = DFMEXT::DFMExtMenuPlugin *(*)();
    using ExtEmblemFuncType = DFMEXT::DFMExtEmblemIconPlugin *(*)();
    using ExtWindowFuncType = DFMEXT::DFMExtWindowPlugin *(*)();
    using ExtFileFuncType = DFMEXT::DFMExtFilePlugin *(*)();

public:
    explicit ExtensionPluginLoader(const QString &filaName, QObject *parent = nullptr);
    ~ExtensionPluginLoader() override { }

    QString fileName() const;
    QString lastError() const;

    bool loadPlugin();
    bool initialize();
    bool shutdown();

    [[nodiscard]] DFMEXT::DFMExtMenuPlugin *resolveMenuPlugin();
    [[nodiscard]] DFMEXT::DFMExtEmblemIconPlugin *resolveEmblemPlugin();
    [[nodiscard]] DFMEXT::DFMExtWindowPlugin *resolveWindowPlugin();
    [[nodiscard]] DFMEXT::DFMExtFilePlugin *resolveFilePlugin();

private:
    QLibrary loader;
    QString errorMessage;

    ExtInitFuncType initFunc { nullptr };
    ExtShutdownFuncType shutdownFunc { nullptr };
    ExtMenuFuncType menuFunc { nullptr };
    ExtEmblemFuncType emblemFunc { nullptr };
    ExtWindowFuncType windowFunc { nullptr };
    ExtFileFuncType fileFunc { nullptr };
};

using ExtPluginLoaderPointer = QSharedPointer<ExtensionPluginLoader>;

DPUTILS_END_NAMESPACE

#endif   // EXTENSIONPLUGINLOADER_H
