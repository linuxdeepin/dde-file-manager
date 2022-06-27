/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "pluginloader_p.h"

#include <QApplication>
#include <QDir>
#include <QDebug>

using namespace dfmplugin_filepreview;
QStringList PreviewPluginLoaderPrivate::pluginPaths;
constexpr char kPluginPath[] { "/../../plugins/common/dfmplugin-preview/previews" };
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
