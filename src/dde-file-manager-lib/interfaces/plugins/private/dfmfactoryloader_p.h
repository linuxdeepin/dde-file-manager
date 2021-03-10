/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#ifndef DFMFACTORYLOADER_P_H
#define DFMFACTORYLOADER_P_H

#include <QString>
#include <private/qobject_p.h>
#include <private/qcoreapplication_p.h>
#include <QMutex>
#include <QPluginLoader>

#include "dfmglobal.h"
#include "dfmfactoryloader.h"

DFM_BEGIN_NAMESPACE

namespace {

// avoid duplicate QStringLiteral data:
inline QString iidKeyLiteral() { return QStringLiteral("IID"); }
#ifdef QT_SHARED
inline QString versionKeyLiteral() { return QStringLiteral("version"); }
#endif
inline QString metaDataKeyLiteral() { return QStringLiteral("MetaData"); }
inline QString keysKeyLiteral() { return QStringLiteral("Keys"); }

}

class DFMFactoryLoaderPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(DFMFactoryLoader)
public:
    DFMFactoryLoaderPrivate();
    ~DFMFactoryLoaderPrivate();
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


DFM_END_NAMESPACE

#endif // DFMFACTORYLOADER_P_H
