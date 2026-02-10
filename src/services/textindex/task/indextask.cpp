// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indextask.h"

#include "progressnotifier.h"
#include "utils/systemdcpuutils.h"
#include "utils/textindexconfig.h"

#include <LuceneException.h>

#include <QDebug>
#include <QThread>
#include <QCoreApplication>

SERVICETEXTINDEX_USE_NAMESPACE
using namespace Lucene;

IndexTask::IndexTask(Type type, const QString &path, TaskHandler handler, QObject *parent)
    : QObject(parent), m_type(type), m_path(path), m_handler(handler)
{
    fmInfo() << "[IndexTask] Created new task - type:" << static_cast<int>(type) << "path:" << path;

    // 连接进度通知
    connect(ProgressNotifier::instance(), &ProgressNotifier::progressChanged,
            this, &IndexTask::onProgressChanged);
}

IndexTask::~IndexTask()
{
    fmDebug() << "[IndexTask] Destroying task for path:" << m_path;
    disconnect(ProgressNotifier::instance(), &ProgressNotifier::progressChanged,
               this, &IndexTask::onProgressChanged);
}

void IndexTask::onProgressChanged(qint64 count, qint64 total)
{
    if (m_state.isRunning()) {
        // 避免在高频进度更新中打印过多日志，只在特定条件下打印
        static qint64 lastLoggedCount = 0;
        if (count == 0 || total == 0 || count == total || (count - lastLoggedCount) >= 1000) {
            fmDebug() << "[IndexTask::onProgressChanged] Task progress - type:" << static_cast<int>(m_type)
                      << "processed:" << count << "total:" << total;
            lastLoggedCount = count;
        }
        emit progressChanged(m_type, count, total);
    }
}

bool IndexTask::silent() const
{
    return m_silent;
}

void IndexTask::setSilent(bool newSilent)
{
    fmDebug() << "[IndexTask::setSilent] Silent mode changed to:" << newSilent << "for path:" << m_path;
    m_silent = newSilent;
}

void IndexTask::throttleCpuUsage()
{
    if (!silent()) {
        fmDebug() << "[IndexTask::throttleCpuUsage] Skipping CPU throttling - not in silent mode";
        return;
    }

    int limit = TextIndexConfig::instance().cpuUsageLimitPercent();
    fmDebug() << "[IndexTask::throttleCpuUsage] Applying CPU usage limit:" << limit << "% for service:"
              << Defines::kTextIndexServiceName;

    QString msg;
    if (!SystemdCpuUtils::setCpuQuota(Defines::kTextIndexServiceName, limit, &msg)) {
        fmWarning() << "[IndexTask::throttleCpuUsage] Failed to set CPU quota:" << msg
                    << "service:" << Defines::kTextIndexServiceName << "limit:" << limit << "%";
    } else {
        fmDebug() << "[IndexTask::throttleCpuUsage] CPU quota applied successfully - limit:" << limit << "%";
    }
}

void IndexTask::start()
{
    if (m_state.isRunning()) {
        fmWarning() << "[IndexTask::start] Task already running, ignoring start request - path:" << m_path;
        return;
    }

    fmInfo() << "[IndexTask::start] Starting task - type:" << static_cast<int>(m_type)
             << "path:" << m_path << "silent:" << m_silent;

    m_state.start();
    m_status = Status::Running;

    Q_ASSERT(QThread::currentThread() != QCoreApplication::instance()->thread());
    fmDebug() << "[IndexTask::start] Task executing in worker thread:" << QThread::currentThread()
              << "main thread:" << QCoreApplication::instance()->thread();

    doTask();
}

void IndexTask::stop()
{
    fmInfo() << "[IndexTask::stop] Stopping task - type:" << static_cast<int>(m_type) << "path:" << m_path;
    m_state.stop();
}

bool IndexTask::isRunning() const
{
    return m_status == Status::Running;
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
    if (m_indexCorrupted != corrupted) {
        fmWarning() << "[IndexTask::setIndexCorrupted] Index corruption status changed to:" << corrupted
                    << "for path:" << m_path;
        m_indexCorrupted = corrupted;
    }
}

void IndexTask::doTask()
{
    fmInfo() << "[IndexTask::doTask] Executing task handler - type:" << static_cast<int>(m_type)
             << "path:" << m_path;

    HandlerResult result { false, false };
    if (m_handler) {
        try {
            setIndexCorrupted(false);
            throttleCpuUsage();

            fmDebug() << "[IndexTask::doTask] Invoking task handler for path:" << m_path;
            result = m_handler(m_path, m_state);

            if (result.fatal) {
                fmCritical() << "[IndexTask::doTask] Task handler reported fatal error - path:" << m_path;
                setIndexCorrupted(true);
            } else if (!result.success) {
                fmWarning() << "[IndexTask::doTask] Task handler failed - path:" << m_path;
            } else {
                fmDebug() << "[IndexTask::doTask] Task handler completed successfully - path:" << m_path;
            }
        } catch (const LuceneException &e) {
            // 捕获到 Lucene 异常，说明索引损坏
            fmCritical() << "[IndexTask::doTask] Lucene exception caught, index corrupted - path:" << m_path
                         << "error:" << QString::fromStdWString(e.getError());
            setIndexCorrupted(true);
            result.success = false;
        } catch (const std::exception &e) {
            fmCritical() << "[IndexTask::doTask] Standard exception caught - path:" << m_path
                         << "error:" << e.what();
            result.success = false;
        } catch (...) {   // 捕获所有异常
            fmCritical() << "[IndexTask::doTask] Unknown exception caught - path:" << m_path;
            result.success = false;
        }
    } else {
        fmCritical() << "[IndexTask::doTask] No task handler provided - path:" << m_path;
    }

    m_state.stop();
    m_status = result.success ? Status::Finished : Status::Failed;

    if (result.success) {
        fmDebug() << "[IndexTask::doTask] Task completed successfully - type:" << static_cast<int>(m_type)
                  << "path:" << m_path;
    } else {
        fmWarning() << "[IndexTask::doTask] Task failed - type:" << static_cast<int>(m_type)
                    << "path:" << m_path << "corrupted:" << m_indexCorrupted;
    }

    emit finished(m_type, result);
}
