// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONWINDOWSMANAGER_H
#define EXTENSIONWINDOWSMANAGER_H

#include "dfmplugin_utils_global.h"

#include <QObject>

DPUTILS_BEGIN_NAMESPACE

class ExtensionWindowsManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ExtensionWindowsManager)

public:
    static ExtensionWindowsManager &instance();
    void initialize();

private Q_SLOTS:
    void onWindowOpened(quint64 id);
    void onWindowClosed(quint64 id);
    void onLastWindowClosed(quint64 id);
    void onCurrentUrlChanged(quint64 id, const QUrl &url);
    void onAllPluginsInitialized();

private:
    explicit ExtensionWindowsManager(QObject *parent = nullptr);
    void handleWindowOpened(quint64 id);

private:
    quint64 firstWinId { 0 };
};

DPUTILS_END_NAMESPACE

#endif   // EXTENSIONWINDOWSMANAGER_H
