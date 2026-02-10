// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONEMBLEMMANAGER_H
#define EXTENSIONEMBLEMMANAGER_H

#include "dfmplugin_utils_global.h"

#include <QObject>

DPUTILS_BEGIN_NAMESPACE

class ExtensionEmblemManagerPrivate;
class ExtensionEmblemManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ExtensionEmblemManager)
    Q_DECLARE_PRIVATE(ExtensionEmblemManager)

public:
    static ExtensionEmblemManager &instance();
    void initialize();

Q_SIGNALS:
    void requestFetchEmblemIcon(const QList<QPair<QString, int>> &localPaths);
    void requestClearCache();

public Q_SLOTS:
    bool onFetchCustomEmblems(const QUrl &url, QList<QIcon> *emblems);
    void onEmblemIconChanged(const QString &path, const QList<QPair<QString, int>> &group);
    void onAllPluginsInitialized();
    bool onUrlChanged(quint64 windowId, const QUrl &url);

private:
    explicit ExtensionEmblemManager(QObject *parent = nullptr);
    ~ExtensionEmblemManager() override;

private:
    QScopedPointer<ExtensionEmblemManagerPrivate> d_ptr;
};

DPUTILS_END_NAMESPACE

#endif   // EXTENSIONEMBLEMMANAGER_H
