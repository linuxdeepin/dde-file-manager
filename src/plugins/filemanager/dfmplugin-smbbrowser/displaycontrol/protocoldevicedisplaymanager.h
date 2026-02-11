// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROTOCOLDEVICEDISPLAYMANAGER_H
#define PROTOCOLDEVICEDISPLAYMANAGER_H

#include "dfmplugin_smbbrowser_global.h"
#include "typedefines.h"

#include <QObject>

DPSMBBROWSER_BEGIN_NAMESPACE

class ProtocolDeviceDisplayManagerPrivate;
class ProtocolDeviceDisplayManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProtocolDeviceDisplayManager)
    friend class ProtocolDeviceDisplayManagerPrivate;

public:
    ~ProtocolDeviceDisplayManager();
    static ProtocolDeviceDisplayManager *instance();

    SmbDisplayMode displayMode() const;
    bool isShowOfflineItem() const;

    // hook computer event
    bool hookItemInsert(const QUrl &entryUrl);
    bool hookItemsFilter(QList<QUrl> *entryUrls);
    void handleItemRenamed(const QUrl &entryUrl, const QString &name);

protected Q_SLOTS:
    void onDevMounted(const QString &id, const QString &mpt);
    void onDevUnmounted(const QString &id);
    void onDConfigChanged(const QString &g, const QString &k);
    void onJsonConfigChanged(const QString &g, const QString &k, const QVariant &v);

    void onMenuSceneAdded(const QString &scene);

private:
    explicit ProtocolDeviceDisplayManager(QObject *parent = nullptr);

private:
    QScopedPointer<ProtocolDeviceDisplayManagerPrivate> d;
};

DPSMBBROWSER_END_NAMESPACE

#endif   // PROTOCOLDEVICEDISPLAYMANAGER_H
