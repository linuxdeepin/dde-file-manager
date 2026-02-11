// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTINDEXDBUS_H
#define TEXTINDEXDBUS_H

#include "service_textindex_global.h"

#include <QObject>
#include <QDBusContext>
#include <QStringList>
#include <QHash>

SERVICETEXTINDEX_BEGIN_NAMESPACE
class TextIndexDBusPrivate;
SERVICETEXTINDEX_END_NAMESPACE

class TextIndexDBus : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.TextIndex")

public:
    explicit TextIndexDBus(const char *name, QObject *parent = nullptr);
    ~TextIndexDBus();

    void cleanup();

public Q_SLOTS:
    void Init();
    bool IsEnabled();
    void SetEnabled(bool enabled);
    bool CreateIndexTask(const QStringList &paths);
    bool UpdateIndexTask(const QStringList &paths);
    bool StopCurrentTask();
    bool HasRunningTask();
    bool IndexDatabaseExists();
    QString GetLastUpdateTime();
    bool ProcessFileChanges(const QStringList &createdFiles, const QStringList &modifiedFiles, const QStringList &deletedFiles);
    bool ProcessFileMoves(const QHash<QString, QString> &movedFiles);

Q_SIGNALS:
    void TaskFinished(const QString &type, const QString &path, bool success);
    void TaskProgressChanged(const QString &type, const QString &path, qint64 count, qint64 total);

private:
    QScopedPointer<SERVICETEXTINDEX_NAMESPACE::TextIndexDBusPrivate> d;
};

#endif   // TEXTINDEXDBUS_H
