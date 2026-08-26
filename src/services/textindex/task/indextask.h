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
        Paused,
        Finished,
        Failed
    };
    Q_ENUM(Status)

    enum class Grade {
        None,     // 未分级
        Light,    // 轻量增量
        Medium,   // 中量增量
        Heavy,    // 重型（首次建库/完整重建/补全未完成Create）
        Manual    // 手动更新（绕过环境门槛）
    };
    Q_ENUM(Grade)

    explicit IndexTask(Type type, const QString &path, TaskHandler handler, QObject *parent = nullptr);
    ~IndexTask();

    void start();
    void stop();
    void requestPause();
    bool isRunning() const;

    QString taskPath() const;
    Type taskType() const;
    Status status() const;
    Grade grade() const;
    void setGrade(Grade grade);

    bool forceBypass() const;
    void setForceBypass(bool bypass);

    bool isIndexCorrupted() const;
    void setIndexCorrupted(bool corrupted);

Q_SIGNALS:
    void progressChanged(SERVICETEXTINDEX_NAMESPACE::IndexTask::Type type, qint64 count, qint64 total);
    void finished(SERVICETEXTINDEX_NAMESPACE::IndexTask::Type type, SERVICETEXTINDEX_NAMESPACE::HandlerResult result);
    void paused(SERVICETEXTINDEX_NAMESPACE::IndexTask::Type type, SERVICETEXTINDEX_NAMESPACE::HandlerResult result);

private:
    void applyResourcePolicy();
    void releaseResourcePolicy();
    void doTask();
    void onProgressChanged(qint64 count, qint64 total);

    Type m_type;
    QString m_path;
    Status m_status { Status::NotStarted };
    Grade m_grade { Grade::None };
    bool m_forceBypass { false };
    TaskState m_state;
    TaskHandler m_handler;
    bool m_indexCorrupted { false };
};

SERVICETEXTINDEX_END_NAMESPACE

Q_DECLARE_METATYPE(SERVICETEXTINDEX_NAMESPACE::IndexTask::Type)

#endif   // INDEXTASK_H
