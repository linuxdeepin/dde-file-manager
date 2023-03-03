// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONPLUGINLOADER_H
#define EXTENSIONPLUGINLOADER_H

#include "dfmplugin_utils_global.h"

// dfm-extension
#include <dfm-extension/menu/dfmextmenuplugin.h>
#include <dfm-extension/emblemicon/dfmextemblemiconplugin.h>

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

public:
    explicit ExtensionPluginLoader(const QString &filaName, QObject *parent = nullptr);
    ~ExtensionPluginLoader() override {}

    QString fileName() const;
    QString lastError() const;

    bool loadPlugin();
    bool initialize();
    bool shutdown();

    [[nodiscard]] DFMEXT::DFMExtMenuPlugin *resolveMenuPlugin();
    [[nodiscard]] DFMEXT::DFMExtEmblemIconPlugin *resolveEmblemPlugin();

private:
    QLibrary loader;
    QString errorMessage;

    ExtInitFuncType initFunc { nullptr };
    ExtShutdownFuncType shutdownFunc { nullptr };
    ExtMenuFuncType menuFunc { nullptr };
    ExtEmblemFuncType emblemFunc { nullptr };
};

using ExtPluginLoaderPointer = QSharedPointer<ExtensionPluginLoader>;

DPUTILS_END_NAMESPACE

#endif   // EXTENSIONPLUGINLOADER_H
