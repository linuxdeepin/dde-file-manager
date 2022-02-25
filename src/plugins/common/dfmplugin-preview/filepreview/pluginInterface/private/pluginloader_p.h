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
#ifndef PLUGINLOADER_P_H
#define PLUGINLOADER_P_H

#include "dfmplugin_filepreview_global.h"

#include <QObject>
#include <QMutex>
#include <QMultiMap>
#include <QPluginLoader>

DPFILEPREVIEW_BEGIN_NAMESPACE

inline QString iidKeyLiteral()
{
    return QStringLiteral("IID");
}
#ifdef QT_SHARED
inline QString versionKeyLiteral()
{
    return QStringLiteral("version");
}
#endif
inline QString metaDataKeyLiteral()
{
    return QStringLiteral("MetaData");
}
inline QString keysKeyLiteral()
{
    return QStringLiteral("Keys");
}

class PreviewPluginLoaderPrivate : public QObject
{
    Q_OBJECT
public:
    explicit PreviewPluginLoaderPrivate(QObject *parent = nullptr);
    virtual ~PreviewPluginLoaderPrivate();
    mutable QMutex mutex;
    QByteArray iid;
    QList<QPluginLoader *> pluginLoaderList;
    QMultiMap<QString, QPluginLoader *> keyMap;
    QString suffix;
    Qt::CaseSensitivity cs;
    bool rki = false;
    QStringList loadedPaths;

    static QStringList pluginPaths;
};
DPFILEPREVIEW_END_NAMESPACE
#endif   //! PLUGINLOADER_P_H
