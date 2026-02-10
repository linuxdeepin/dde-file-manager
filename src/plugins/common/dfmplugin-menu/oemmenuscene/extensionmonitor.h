// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONMONITOR_H
#define EXTENSIONMONITOR_H

#include "dfmplugin_menu_global.h"

#include <QObject>
#include <QMap>

namespace dfmplugin_menu {

class ExtensionMonitor : public QObject
{
    Q_OBJECT
public:
    static ExtensionMonitor *instance();
    void start();

private:
    explicit ExtensionMonitor(QObject *parent = nullptr);
    void setupFileWatchers();
    void checkAndMkpath(const QString &path);
    void copyInitialFiles();
    void processExtensionDirectory(const QString &sourcePath, const QString &targetPath);

private Q_SLOTS:
    void onFileAdded(const QUrl &url);
    void onFileDeleted(const QUrl &url);

private:
    QMap<QString, QString> extensionMap;
};
}

#endif   // EXTENSIONMONITOR_H
