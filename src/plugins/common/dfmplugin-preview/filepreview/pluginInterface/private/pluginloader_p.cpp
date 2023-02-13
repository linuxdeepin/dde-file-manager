// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginloader_p.h"

#include <QApplication>
#include <QDir>
#include <QDebug>

using namespace dfmplugin_filepreview;
QStringList PreviewPluginLoaderPrivate::pluginPaths;
static constexpr char kPluginPath[] { "/../../plugins/common/dfmplugin-preview/previews" };
PreviewPluginLoaderPrivate::PreviewPluginLoaderPrivate(QObject *parent)
    : QObject(parent)
{
    if (pluginPaths.isEmpty()) {
        QString pluginsDir(qApp->applicationDirPath() + kPluginPath);
        qInfo() << pluginsDir;
        if (!QDir(pluginsDir).exists()) {
            pluginPaths.append(QString::fromLocal8Bit(PLUGINDIR));
        } else {
            pluginPaths.append(pluginsDir);
        }
    }
}

PreviewPluginLoaderPrivate::~PreviewPluginLoaderPrivate()
{
}
