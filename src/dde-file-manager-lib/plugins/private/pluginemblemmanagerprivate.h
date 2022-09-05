// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINEMBLEMMANAGERPRIVATE_H
#define PLUGINEMBLEMMANAGERPRIVATE_H

#include "interfaces/durl.h"
#include "dfm-extension-global.h"

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

BEGEN_DFMEXT_NAMESPACE
class DFMExtEmblemIconPlugin;
END_DFMEXT_NAMESPACE

class QTimer;
class PluginEmblemManager;
class PluginEmblemManagerPrivate : public QThread
{
    Q_OBJECT
public:
    explicit PluginEmblemManagerPrivate(PluginEmblemManager *qq);
    ~PluginEmblemManagerPrivate() override;

    // 从缓存获取插件角标
    void getPluginEmblemIconsFromMap(const DUrl &fileUrl, int systemIconCount, QList<QIcon> &icons);

    // 清空缓存的角标
    void clearEmblemIconsMap();

private slots:
    void updateTimerTimeout();
    // 更新插件角标
    void updatePluginEmblem();
    void doUpdateWork();

private:
    // 异步调用插件接口更新角标
    void asyncUpdateEmblemIconsFromPlugin(const DUrl &fileUrl, int systemIconCount);
    void run() override;
    void startWork();
    void stopWork();
    // 获得插件中通过emblemIcons接口设置的角标
    void getEmblemIcons(QSharedPointer<DFMEXT::DFMExtEmblemIconPlugin> plugin, const QPair<QString, int> &data,
                                bool &bHaveIcon, QStringList &newIcons);
    // 获取插件中通过locationEmblemIcons接口设置的角标
    void getLocationEmblemIcons(QSharedPointer<DFMEXT::DFMExtEmblemIconPlugin> plugin, const QPair<QString, int> &data,
                                  bool &bHaveIcon, QStringList &newIcons);
    // 将从插件中获取到的角标缓存到容器
    void cacheEmblemToMap(bool &bHaveIcon, const QPair<QString, int> &data, const QStringList &newIcons);
    // 设置文件路径
    void setFilePath(const QString &iconPath, QStringList &newIcons, int index);

private:
    PluginEmblemManager *const q {};
    friend class PluginEmblemManager;
    // 缓存角标路径（key:文件路径  value:角标路径）
    QMap<QString, QStringList> mapIcons {};
    // 缓存文件路径
    QQueue<QPair<QString, int>> filePathQueue {};
    bool bWork { false };
    mutable QMutex mutexMap;
    mutable QMutex mutexQueue;
    QWaitCondition pathQueueNotEmpty;
    // 发送更新信号的定时器
    QTimer *updateTimer { nullptr };
    // 记录是否初始化了插件
    bool bInitPlugin { false };
    // 记录插件中是否存在角标类对象
    bool bHaveEmblemObj { false };
    // 缓存清空定时器
    QTimer *clearCacheTimer { nullptr };
};

#endif // PLUGINEMBLEMMANAGERPRIVATE_H
