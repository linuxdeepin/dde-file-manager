// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filecountcalculator.h"

#include <QtConcurrent>
#include <QUrl>

#include <sys/stat.h>
#include <unistd.h>

using namespace dfmplugin_propertydialog;

FileCountCalculator::FileCountCalculator(QObject *parent)
    : QObject(parent)
{
}

FileCountCalculator::~FileCountCalculator()
{
    stop();
}

void FileCountCalculator::start(const QList<QUrl> &urls, int batchSize)
{
    // 如果已有任务在运行，忽略新请求
    if (m_watcher && m_watcher->isRunning()) {
        qWarning() << "FileCountCalculator: Already running, ignoring new start request";
        return;
    }

    if (urls.isEmpty()) {
        emit finished(0, 0);
        return;
    }

    // 清理旧的 watcher
    if (m_watcher) {
        m_watcher->cancel();
        m_watcher->waitForFinished();
        m_watcher->deleteLater();
    }

    // 创建新的 watcher
    m_watcher = new QFutureWatcher<QPair<int, int>>(this);

    // 连接完成信号
    connect(m_watcher, &QFutureWatcher<QPair<int, int>>::finished,
            this, &FileCountCalculator::onCalculationFinished);

    // 在后台线程执行计算
    QFuture<QPair<int, int>> future = QtConcurrent::run(calculateWorker, urls, batchSize, this);
    m_watcher->setFuture(future);
}

void FileCountCalculator::onCalculationFinished()
{
    if (!m_watcher)
        return;

    QPair<int, int> result = m_watcher->result();
    emit finished(result.first, result.second);
}

QPair<int, int> FileCountCalculator::calculateWorker(const QList<QUrl> &urls,
                                                      int batchSize,
                                                      FileCountCalculator *calculator)
{
    int fileCount = 0;
    int dirCount = 0;
    int processedCount = 0;

    for (const QUrl &url : urls) {
        // 检查是否被取消（通过 watcher 检查）
        if (calculator && calculator->m_watcher && calculator->m_watcher->isCanceled())
            break;

        // 仅处理本地文件（file:// scheme）
        if (!url.isLocalFile())
            continue;

        // 转换为本地路径
        QString localPath = url.toLocalFile();
        if (localPath.isEmpty())
            continue;

        // 使用 lstat 获取文件信息（不跟随符号链接）
        struct stat statBuf;
        if (lstat(localPath.toUtf8().constData(), &statBuf) != 0)
            continue;

        // 判断文件类型
        if (S_ISLNK(statBuf.st_mode)) {
            // 符号链接：需要 stat 获取目标类型
            struct stat targetBuf;
            if (stat(localPath.toUtf8().constData(), &targetBuf) == 0) {
                if (S_ISDIR(targetBuf.st_mode))
                    ++dirCount;
                else
                    ++fileCount;
            } else {
                // 符号链接目标不存在，计为文件
                ++fileCount;
            }
        } else if (S_ISDIR(statBuf.st_mode)) {
            // 目录
            ++dirCount;
        } else if (S_ISREG(statBuf.st_mode)) {
            // 普通文件
            ++fileCount;
        }
        // 忽略其他类型（socket、设备文件等）

        // 每处理 batchSize 个文件，发送一次进度通知
        // Qt 自动将信号排队到主线程
        if (++processedCount % batchSize == 0) {
            if (calculator)
                emit calculator->progressNotify(fileCount, dirCount);
        }
    }

    // 最后再发送一次进度（确保最终状态被更新）
    if (calculator)
        emit calculator->progressNotify(fileCount, dirCount);

    return qMakePair(fileCount, dirCount);
}

void FileCountCalculator::stop()
{
    if (!m_watcher)
        return;

    m_watcher->cancel();
    m_watcher->waitForFinished();
    m_watcher->deleteLater();
    m_watcher = nullptr;
}
