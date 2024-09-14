// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginloader_p.h"

#include <QApplication>
#include <QDir>
#include <QDebug>

using namespace dfmplugin_filepreview;
QStringList PreviewPluginLoaderPrivate::pluginPaths;
static constexpr char kPluginPath[] { "/common/dfmplugin-preview/previews" };
PreviewPluginLoaderPrivate::PreviewPluginLoaderPrivate(QObject *parent)
    : QObject(parent)
{
    if (pluginPaths.isEmpty()) {
#ifdef QT_DEBUG
        QString pluginsDir(qApp->property("DFM_BUILD_PLUGIN_DIR").toString() + kPluginPath);
        pluginPaths.append(pluginsDir);
#else
        pluginPaths.append(QString::fromLocal8Bit(PLUGINDIR));
#endif
    }
}

PreviewPluginLoaderPrivate::~PreviewPluginLoaderPrivate()
{
}
