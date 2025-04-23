// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSMONITORWORKER_H
#define FSMONITORWORKER_H

#include "service_textindex_global.h"

#include <QObject>
#include <QDir>
#include <QStringList>

#include <functional>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class FSMonitorWorker : public QObject
{
    Q_OBJECT

public:
    explicit FSMonitorWorker(QObject *parent = nullptr);
    ~FSMonitorWorker() override;

    // Set exclusion checker function (moved to regular method to avoid MOC issues)
    void setExclusionChecker(const std::function<bool(const QString &)> &checker);

public slots:
    // Process a directory and find its subdirectories
    void processDirectory(const QString &path);

signals:
    // Emitted when a directory should be watched
    void directoryToWatch(const QString &path);

    // Emitted when subdirectories are found
    void subdirectoriesFound(const QStringList &directories);

private:
    // Function to check if a path should be excluded
    std::function<bool(const QString &)> exclusionChecker;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // FSMONITORWORKER_H
