/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dostatisticsfilesworker.h"
#include "fileoperationsutils.h"

#include <QDebug>

#include <sys/vfs.h>
#include <sys/stat.h>
#include <fts.h>
#include <unistd.h>
#include <sys/utsname.h>

DSC_USE_NAMESPACE
DoStatisticsFilesWorker::DoStatisticsFilesWorker(const QList<QUrl> &files, const bool isRecordAll, QObject *parent)
    : QObject(parent), files(files), isRecordAll(isRecordAll)
{
}
/*!
 * \brief DoStatisticsFilesWorker::doWork 统计所有文件大小信息的线程执行函数
 */
void DoStatisticsFilesWorker::doWork()
{
    // ToDo::统计文件大小信息
    QSharedPointer<FileOperationsUtils::FilesSizeInfo> filesSizeInfo(new FileOperationsUtils::FilesSizeInfo);
    qint64 total = 0;
    if (isRecordAll)
        filesSizeInfo->allFiles.clear();
    for (auto url : files) {
        // 停止就退出
        if (isStoped) {
            filesSizeInfo->dirSize = filesSizeInfo->dirSize <= 0 ? FileOperationsUtils::getMemoryPageSize() : filesSizeInfo->dirSize;
            emit finished(filesSizeInfo);
        }
        char *paths[2] = { nullptr, nullptr };
        paths[0] = strdup(url.path().toUtf8().toStdString().data());
        FTS *fts = fts_open(paths, 0, nullptr);
        if (paths[0])
            free(paths[0]);

        while (1) {
            // 停止就退出
            if (isStoped) {
                fts_close(fts);
                filesSizeInfo->dirSize = filesSizeInfo->dirSize <= 0 ? FileOperationsUtils::getMemoryPageSize() : filesSizeInfo->dirSize;
                emit finished(filesSizeInfo);
            }

            FTSENT *ent = fts_read(fts);
            if (ent == nullptr) {
                break;
            }
            unsigned short flag = ent->fts_info;
            if (isRecordAll)
                filesSizeInfo->allFiles.append(QUrl::fromLocalFile(ent->fts_path));
            if (flag != FTS_DP)
                total += ent->fts_statp->st_size <= 0 ? FileOperationsUtils::getMemoryPageSize() : ent->fts_statp->st_size;
            if (filesSizeInfo->dirSize == 0 && flag == FTS_D)
                filesSizeInfo->dirSize = ent->fts_statp->st_size <= 0 ? FileOperationsUtils::getMemoryPageSize() : static_cast<qint32>(ent->fts_statp->st_size);
            if (flag == FTS_F)
                filesSizeInfo->fileCount++;
        }
        fts_close(fts);
    }
}
/*!
 * \brief DoStatisticsFilesWorker::stopWork 停止统计文件大小信息
 */
void DoStatisticsFilesWorker::stopWork()
{
    // ToDo::停止所有统计信息大小
    isStoped = true;
}
