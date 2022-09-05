// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
