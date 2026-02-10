// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
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
        Create,   // 创建索引
        Update,   // 更新索引
        CreateFileList,   // 基于文件列表创建索引
        UpdateFileList,   // 基于文件列表更新索引
        RemoveFileList,   // 基于文件列表删除索引
        MoveFileList   // 基于文件移动列表更新索引路径
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

    bool silent() const;
    void setSilent(bool newSilent);

Q_SIGNALS:
    void progressChanged(SERVICETEXTINDEX_NAMESPACE::IndexTask::Type type, qint64 count, qint64 total);
    void finished(SERVICETEXTINDEX_NAMESPACE::IndexTask::Type type, SERVICETEXTINDEX_NAMESPACE::HandlerResult result);

private:
    void throttleCpuUsage();
    void doTask();
    void onProgressChanged(qint64 count, qint64 total);

    Type m_type;
    QString m_path;
    Status m_status { Status::NotStarted };
    TaskState m_state;
    TaskHandler m_handler;
    bool m_indexCorrupted { false };
    bool m_silent { false };
};

SERVICETEXTINDEX_END_NAMESPACE

Q_DECLARE_METATYPE(SERVICETEXTINDEX_NAMESPACE::IndexTask::Type)

#endif   // INDEXTASK_H
