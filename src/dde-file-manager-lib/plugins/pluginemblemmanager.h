/*
* Copyright (C) 2021 Deepin Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#ifndef PLUGINEMBLEMMANAGER_H
#define PLUGINEMBLEMMANAGER_H

#include "interfaces/durl.h"

#include <QObject>

class PluginEmblemManagerPrivate;
class PluginEmblemManager : public QObject
{
    Q_DISABLE_COPY(PluginEmblemManager)
    Q_OBJECT
public:
    static PluginEmblemManager *instance();
    // 从缓存获取插件角标
    void getPluginEmblemIconsFromMap(const DUrl &fileUrl, int systemIconCount, QList<QIcon> &icons);
    // 清空缓存的角标
    void clearEmblemIconsMap();

private:
    explicit PluginEmblemManager(QObject *parent = nullptr);
    ~PluginEmblemManager();

signals:
    // 角标更新信号
    void updatePluginEmblem();

private:
    PluginEmblemManagerPrivate *d { nullptr };
    friend class PluginEmblemManagerPrivate;
};

#endif // PLUGINEMBLEMMANAGER_H
