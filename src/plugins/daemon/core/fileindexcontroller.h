// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINDEXCONTROLLER_H
#define FILEINDEXCONTROLLER_H

#include "daemonplugin_core_global.h"

#include <QObject>

DAEMONPCORE_BEGIN_NAMESPACE

class FileIndexController : public QObject
{
    Q_OBJECT

public:
    explicit FileIndexController(QObject *parent = nullptr);
    ~FileIndexController() override;

    void initialize();

private:
    void handleConfigChanged(const QString &config, const QString &key);
    void scheduleApply();
    bool enableFileIndex();
    bool disableFileIndex();
    bool isServiceEnabled() const;
    bool isServiceActive() const;
    bool runSystemctlCommand(const QStringList &arguments) const;
    bool createRefreshIndexFile() const;
    QString refreshFilePath() const;

private:
    bool isConfigEnabled { false };
    bool m_operationInProgress { false };
};

DAEMONPCORE_END_NAMESPACE

#endif   // FILEINDEXCONTROLLER_H
