/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             huangyu<zhangyub@uniontech.com>
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
#ifndef DFMEXTPLUGINMANAGER_H
#define DFMEXTPLUGINMANAGER_H

#include "dfmextpluginloader.h"

#include <QString>
#include <QObject>

class DFMExtMenuImplProxy;
class DFMExtPluginManagerPrivate;
class DFMExtPluginManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DFMExtPluginManager)
    DFMExtPluginManagerPrivate *const d;
public:
    enum State{
        Invalid,
        Scanned,
        Loaded,
        Initialized,
        Shutdown
    };
    typedef QList<QSharedPointer<DFMEXT::DFMExtMenuPlugin>> DFMExtMenus;
    typedef QList<QSharedPointer<DFMEXT::DFMExtEmblemIconPlugin>> DFMExtEmblemIcons;
    QString pluginDefaultPath();
    void setPluginPaths(const QStringList &paths);
    QStringList pluginPaths() const;
    bool scannPlugins();
    bool loadPlugins();
    bool initPlugins();
    bool shutdownPlugins();
    DFMExtMenus menus();
    DFMExtEmblemIcons emblemIcons();
    DFMExtMenuImplProxy *pluginMenuProxy();
    State state() const;
    QString errorString() const;
    static DFMExtPluginManager &instance();
private:
    explicit DFMExtPluginManager(QObject *parent = nullptr);
};

#endif // DFMEXTPLUGINMANAGER_H
