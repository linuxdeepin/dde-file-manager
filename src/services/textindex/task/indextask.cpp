// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indextask.h"

#include "progressnotifier.h"

#include <LuceneException.h>

#include <QDebug>
#include <QThread>
#include <QCoreApplication>

SERVICETEXTINDEX_USE_NAMESPACE
using namespace Lucene;

IndexTask::IndexTask(Type type, const QString &path, TaskHandler handler, QObject *parent)
    : QObject(parent), m_type(type), m_path(path), m_handler(handler)
{
    fmDebug() << "Created new task for path:" << path;

    // 连接进度通知
    connect(ProgressNotifier::instance(), &ProgressNotifier::progressChanged,
            this, &IndexTask::onProgressChanged);
}

IndexTask::~IndexTask()
{
    disconnect(ProgressNotifier::instance(), &ProgressNotifier::progressChanged,
               this, &IndexTask::onProgressChanged);
}

void IndexTask::onProgressChanged(qint64 count)
{
    if (m_state.isRunning()) {
        fmDebug() << "Task progress:" << count;
        emit progressChanged(m_type, count);
    }
}

void IndexTask::start()
{
    if (m_state.isRunning()) {
        fmWarning() << "Task already running, ignoring start request";
        return;
    }

    fmInfo() << "Starting task for path:" << m_path;

    m_state.start();
    m_status = Status::Running;

    Q_ASSERT(QThread::currentThread() != QCoreApplication::instance()->thread());
    fmDebug() << "Task running in worker thread:" << QThread::currentThread();

    doTask();
}

void IndexTask::stop()
{
    fmInfo() << "Stopping task for path:" << m_path;
    m_state.stop();
}

bool IndexTask::isRunning() const
{
    return m_state.isRunning();
}

QString IndexTask::taskPath() const
{
    return m_path;
}

IndexTask::Type IndexTask::taskType() const
{
    return m_type;
}

IndexTask::Status IndexTask::status() const
{
    return m_status;
}

bool IndexTask::isIndexCorrupted() const
{
    return m_indexCorrupted;
}

void IndexTask::setIndexCorrupted(bool corrupted)
{
    m_indexCorrupted = corrupted;
}

void IndexTask::doTask()
{
    fmInfo() << "Processing task for path:" << m_path;

    bool success = false;
    if (m_handler) {
        try {
            setIndexCorrupted(false);
            success = m_handler(m_path, m_state);
        } catch (const LuceneException &) {
            // 捕获到 Lucene 异常，说明索引损坏
            setIndexCorrupted(true);
            success = false;
        }
    } else {
        fmWarning() << "No task handler provided";
    }

    m_state.stop();
    m_status = success ? Status::Finished : Status::Failed;

    if (success) {
        fmInfo() << "Task completed successfully for path:" << m_path;
    } else {
        fmWarning() << "Task failed for path:" << m_path;
    }

    emit finished(m_type, success);
}
