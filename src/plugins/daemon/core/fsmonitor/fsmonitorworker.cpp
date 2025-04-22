// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fsmonitorworker.h"

#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QMetaObject>

DAEMONPCORE_BEGIN_NAMESPACE

FSMonitorWorker::FSMonitorWorker(QObject *parent)
    : QObject(parent)
{
    // Default exclusion checker (excludes nothing)
    exclusionChecker = [](const QString &) { return false; };
}

FSMonitorWorker::~FSMonitorWorker()
{
}

void FSMonitorWorker::processDirectory(const QString &path)
{
    if (path.isEmpty() || !QFileInfo(path).isDir()) {
        return;
    }

    // Skip excluded directories
    if (exclusionChecker(path)) {
        return;
    }

    // Emit signal to add this directory to watch
    emit directoryToWatch(path);

    // Find subdirectories
    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (!entries.isEmpty()) {
        QStringList subDirs;
        subDirs.reserve(entries.size());

        // Collect valid subdirectories
        for (const QFileInfo &info : std::as_const(entries)) {
            const QString &subPath = info.absoluteFilePath();
            if (info.isDir() && !info.isSymLink() && !exclusionChecker(subPath)) {
                subDirs << subPath;
            }
        }

        // Emit signal with found subdirectories
        if (!subDirs.isEmpty()) {
            emit subdirectoriesFound(subDirs);
        }
    }
}

void FSMonitorWorker::setExclusionChecker(const std::function<bool(const QString &)> &checker)
{
    if (checker) {
        exclusionChecker = checker;
    }
}

DAEMONPCORE_END_NAMESPACE
