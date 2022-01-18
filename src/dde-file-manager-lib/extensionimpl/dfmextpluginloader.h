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
#ifndef DFMEXTPLUGINLOADER_H
#define DFMEXTPLUGINLOADER_H

#include <menu/dfmextmenuplugin.h>
#include <emblemicon/dfmextemblemiconplugin.h>
#include <QString>
#include <QSharedPointer>

class DFMExtPluginLoaderPrivate;
class DFMExtPluginLoader
{
    Q_DISABLE_COPY(DFMExtPluginLoader)
    DFMExtPluginLoaderPrivate *const d;
public:
    explicit DFMExtPluginLoader(const QString &filaName);
    virtual ~DFMExtPluginLoader();
    static bool hasSymbol(const QString &fileName);
    bool loadPlugin();
    bool initialize();
    void shutdown();
    QString fileName() const;
    QString errorString() const;
    QSharedPointer<DFMEXT::DFMExtMenuPlugin> extMenuPlugin();
    QSharedPointer<DFMEXT::DFMExtEmblemIconPlugin> extEmbleIconPlugin();
};

typedef QSharedPointer<DFMExtPluginLoader> DFMExtPluginLoaderPointer;

#endif // DFMEXTPLUGINLOADER_H
