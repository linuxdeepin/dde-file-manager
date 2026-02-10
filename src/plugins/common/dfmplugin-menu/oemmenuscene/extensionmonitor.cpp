// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extensionmonitor.h"

#include <dfm-base/file/local/localfilewatcher.h>

#include <QDir>
#include <QFile>
#include <QStandardPaths>

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

static const char *const kOemMenuExtensionsPath = "/usr/share/deepin/dde-file-manager/oem-menuextensions";

ExtensionMonitor::ExtensionMonitor(QObject *parent)
    : QObject(parent)
{
    // Get XDG_DATA_HOME path
    QString xdgDataHome = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QString oemDataPath = xdgDataHome + "/deepin/dde-file-manager/oem-menuextensions";

    extensionMap.insert(kOemMenuExtensionsPath, oemDataPath);
}

ExtensionMonitor *ExtensionMonitor::instance()
{
    static ExtensionMonitor ins;
    return &ins;
}

void ExtensionMonitor::start()
{
    QTimer::singleShot(5000, this, [this] {
        copyInitialFiles();
        setupFileWatchers();
    });
}

void ExtensionMonitor::setupFileWatchers()
{
    for (const auto &path : extensionMap.keys()) {
        auto watcher = new LocalFileWatcher(QUrl::fromLocalFile(path), this);
        if (watcher) {
            connect(watcher, &LocalFileWatcher::subfileCreated,
                    this, &ExtensionMonitor::onFileAdded);
            connect(watcher, &LocalFileWatcher::fileDeleted,
                    this, &ExtensionMonitor::onFileDeleted);
            watcher->startWatcher();
        }
    }
}

void ExtensionMonitor::checkAndMkpath(const QString &path)
{
    // Create directory if it doesn't exist
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(".");
}

void ExtensionMonitor::copyInitialFiles()
{
    for (auto iter = extensionMap.cbegin(); iter != extensionMap.cend(); ++iter) {
        processExtensionDirectory(iter.key(), iter.value());
    }
}

void ExtensionMonitor::processExtensionDirectory(const QString &sourcePath, const QString &targetPath)
{
    checkAndMkpath(targetPath);
    QDir sourceDir(sourcePath);
    QDir targetDir(targetPath);
    QStringList desktopFiles = sourceDir.entryList(QStringList() << "*.desktop", QDir::Files);
    for (const auto &file : std::as_const(desktopFiles)) {
        QString sourceFile = sourceDir.filePath(file);
        QString targetFile = targetDir.filePath(file);
        if (QFile::exists(targetFile))
            continue;

        if (!QFile::copy(sourceFile, targetFile))
            fmWarning() << "Failed to copy file:" << sourceFile << "to" << targetFile;
    }
}

void ExtensionMonitor::onFileAdded(const QUrl &url)
{
    QString sourceFile = url.toLocalFile();
    // Only handle .desktop files
    if (!sourceFile.endsWith(".desktop"))
        return;

    QFileInfo fileInfo(sourceFile);
    QString targetPath = extensionMap.value(fileInfo.absolutePath());
    checkAndMkpath(targetPath);
    QString targetFile = QDir(targetPath).filePath(fileInfo.fileName());
    if (!QFile::copy(sourceFile, targetFile))
        fmWarning() << "Failed to copy file:" << sourceFile << "to" << targetFile;
}

void ExtensionMonitor::onFileDeleted(const QUrl &url)
{
    QString sourceFile = url.toLocalFile();
    // Only handle .desktop files
    if (!sourceFile.endsWith(".desktop"))
        return;

    QFileInfo fileInfo(sourceFile);
    QString targetPath = extensionMap.value(fileInfo.absolutePath());
    QString targetFile = QDir(targetPath).filePath(fileInfo.fileName());
    if (!QFile::exists(targetFile))
        return;

    if (!QFile::remove(targetFile))
        fmWarning() << "Failed to remove file:" <<  targetFile;
}
