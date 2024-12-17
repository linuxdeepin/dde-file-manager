// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXTASK_H
#define INDEXTASK_H

#include "service_textindex_global.h"
#include "taskhandler.h"
#include "utils/taskstate.h"

#include <QObject>
#include <QString>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class IndexTask : public QObject
{
    Q_OBJECT
public:
    enum class Type {
        Create,
        Update,
        Remove
    };
    Q_ENUM(Type)

    enum class Status {
        NotStarted,
        Running,
        Finished,
        Failed
    };
    Q_ENUM(Status)

    explicit IndexTask(Type type, const QString &path, TaskHandler handler, QObject *parent = nullptr);
    ~IndexTask();

    void start();
    void stop();
    bool isRunning() const;

    QString taskPath() const;
    Type taskType() const;
    Status status() const;

    bool isIndexCorrupted() const;
    void setIndexCorrupted(bool corrupted);

Q_SIGNALS:
    void progressChanged(SERVICETEXTINDEX_NAMESPACE::IndexTask::Type type, qint64 count);
    void finished(SERVICETEXTINDEX_NAMESPACE::IndexTask::Type type, bool success);

private:
    void doTask();
    void onProgressChanged(qint64 count);

    Type m_type;
    QString m_path;
    Status m_status { Status::NotStarted };
    TaskState m_state;
    TaskHandler m_handler;
    bool m_indexCorrupted { false };
};

SERVICETEXTINDEX_END_NAMESPACE

Q_DECLARE_METATYPE(SERVICETEXTINDEX_NAMESPACE::IndexTask::Type)

#endif   // INDEXTASK_H
