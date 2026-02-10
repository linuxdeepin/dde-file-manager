// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginloader_p.h"

#include <QApplication>
#include <QDir>
#include <QDebug>

using namespace dfmplugin_filepreview;
QStringList PreviewPluginLoaderPrivate::pluginPaths;
PreviewPluginLoaderPrivate::PreviewPluginLoaderPrivate(QObject *parent)
    : QObject(parent)
{
    if (pluginPaths.isEmpty()) {
#ifdef QT_DEBUG
        char path[PATH_MAX] = { 0 };
        const char *defaultPath = realpath("../previews", path);
        pluginPaths.append(QString::fromLocal8Bit(defaultPath));
#else
        pluginPaths.append(QString::fromLocal8Bit(DFM_PLUGIN_PREVIEW_DIR));
#endif
    }
}

PreviewPluginLoaderPrivate::~PreviewPluginLoaderPrivate()
{
}
