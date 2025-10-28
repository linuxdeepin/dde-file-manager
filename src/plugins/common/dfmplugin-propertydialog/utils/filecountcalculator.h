// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILECOUNTCALCULATOR_H
#define FILECOUNTCALCULATOR_H

#include <QObject>
#include <QUrl>
#include <QList>
#include <QFutureWatcher>

namespace dfmplugin_propertydialog {

/**
 * @brief 文件数量异步计算器
 *
 * 职责：
 * - 在后台线程统计文件和文件夹数量
 * - 支持分批统计，实时通知进度（避免阻塞主线程）
 * - 可取消的任务执行
 *
 */
class FileCountCalculator : public QObject
{
    Q_OBJECT

public:
    explicit FileCountCalculator(QObject *parent = nullptr);
    ~FileCountCalculator() override;

    /**
     * @brief 启动异步计算
     * @param urls 待统计的文件/文件夹 URL 列表
     * @param batchSize 批处理大小（每处理 N 个文件发送一次进度通知）
     */
    void start(const QList<QUrl> &urls, int batchSize = 100);

    /**
     * @brief 取消当前计算任务
     */
    void stop();

signals:
    /**
     * @brief 进度更新信号（可实时刷新 UI）
     * @param fileCount 当前已统计的文件数量
     * @param dirCount 当前已统计的文件夹数量
     */
    void progressNotify(int fileCount, int dirCount);

    /**
     * @brief 统计完成信号
     * @param fileCount 最终文件数量
     * @param dirCount 最终文件夹数量
     */
    void finished(int fileCount, int dirCount);

private:
    void onCalculationFinished();

    static QPair<int, int> calculateWorker(const QList<QUrl> &urls,
                                            int batchSize,
                                            FileCountCalculator *calculator);

private:
    QFutureWatcher<QPair<int, int>> *m_watcher { nullptr };
};

}   // namespace dfmplugin_propertydialog

#endif   // FILECOUNTCALCULATOR_H
