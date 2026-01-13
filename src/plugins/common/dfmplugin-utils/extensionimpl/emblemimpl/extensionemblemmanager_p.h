// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONEMBLEMMANAGER_P_H
#define EXTENSIONEMBLEMMANAGER_P_H

#include "extensionemblemmanager.h"

#include "extensionimpl/pluginsload/extensionpluginmanager.h"

#include <QThread>
#include <QMap>
#include <QSet>
#include <QTimer>

DPUTILS_BEGIN_NAMESPACE

class EmblemIconWorker : public QObject
{
    Q_OBJECT
    using CacheType = QMap<QString, QList<QPair<QString, int>>>;
Q_SIGNALS:
    void emblemIconChanged(const QString &path, const QList<QPair<QString, int>> &emblemGroup);

public Q_SLOTS:
    void onFetchEmblemIcons(const QList<QPair<QString, int>> &localPaths);
    void onClearCache();

private:
    // method 2
    bool parseLocationEmblemIcons(const QString &path, int count, DFMEXT::DFMExtEmblemIconPlugin *plugin);
    // method 1
    void parseEmblemIcons(const QString &path, int count, DFMEXT::DFMExtEmblemIconPlugin *plugin);

    CacheType makeCache(const QString &path, const QList<QPair<QString, int>> &group);
    void makeLayoutGroup(const std::vector<DFMEXT::DFMExtEmblemIconLayout> &layouts, QList<QPair<QString, int>> *group);
    QList<QPair<QString, int>> updateLayoutGroup(const QList<QPair<QString, int>> &cache, const QList<QPair<QString, int>> &group);
    void makeNormalGroup(const std::vector<std::string> &icons, int count, QList<QPair<QString, int>> *group);
    void mergeGroup(const QList<QPair<QString, int>> &oldGroup,
                    const QList<QPair<QString, int>> &newGroup,
                    QList<QPair<QString, int>> *group);
    bool hasCachedByOtherLocationEmblem(const QString &path, quint64 addr);
    void saveToPluginCache(quint64 addr, const QString &path, const QList<QPair<QString, int>> &group);

private:
    CacheType embelmCaches;   // filePath -> pair<iconPath, iconCount>
    QMap<quint64, CacheType> pluginCaches;   // plugin -> filePath -> pair<iconPath, iconCount>
};

class ExtensionEmblemManagerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(ExtensionEmblemManager)

public:
    ExtensionEmblemManagerPrivate(ExtensionEmblemManager *qq);
    ~ExtensionEmblemManagerPrivate() override;

    void addReadyLocalPath(const QPair<QString, int> &path);
    void clearReadyLocalPath();
    QIcon makeIcon(const QString &path);

public:
    ExtensionEmblemManager *q_ptr { nullptr };

    QThread workerThread;

    QTimer readyTimer;
    bool readyFlag { false };
    QList<QPair<QString, int>> readyLocalPaths;
    QMap<QString, QList<QPair<QString, int>>> positionEmbelmCaches;   // file path ->  { pairs { emblem icon path, pos }}
    // 缓存已加载的图标，避免重复加载
    // 键: 图标路径或主题名称
    // 值: 对应的QIcon对象
    QHash<QString, QIcon> iconCaches;
};

DPUTILS_END_NAMESPACE

#endif   // EXTENSIONEMBLEMMANAGER_P_H
