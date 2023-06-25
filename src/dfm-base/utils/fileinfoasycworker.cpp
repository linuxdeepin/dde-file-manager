// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileinfoasycworker.h"
#include "fileutils.h"
#include "networkutils.h"
#include "mimetype/dmimedatabase.h"

namespace dfmbase {
FileInfoAsycWorker::FileInfoAsycWorker(QObject *parent)
    : QObject(parent)
{
}

void FileInfoAsycWorker::fileConutAsync(const QUrl &url, const QSharedPointer<FileInfoHelperUeserData> data)
{
    if (isStoped())
        return;
    const int count = FileUtils::dirFfileCount(url);
    if (data) {
        data->finish = true;
        data->data = count;
    }
    emit fileConutAsyncFinish(url, count);
}

void FileInfoAsycWorker::fileMimeType(const QUrl &url,
                                      const QMimeDatabase::MatchMode mode,
                                      const QString &inod,
                                      const bool isGvfs,
                                      const QSharedPointer<FileInfoHelperUeserData> data)
{
    if (isStoped())
        return;
    DFMBASE_NAMESPACE::DMimeDatabase db;
    QMimeType type;
    if (isGvfs) {
        type = db.mimeTypeForFile(url.path(), mode, inod, isGvfs);
    } else {
        type = db.mimeTypeForFile(url, mode);
    }
    data->finish = true;
    data->data = QVariant::fromValue(type);
    emit fileMimeTypeFinished(url, type);
}

void FileInfoAsycWorker::fileRefresh(const QUrl &url, const QSharedPointer<dfmio::DFileInfo> dfileInfo)
{
    if (dfileInfo && NetworkUtils::instance()->checkFtpOrSmbBusy(url))
        dfileInfo->refresh();
}

dfmbase::FileInfoAsycWorker::~FileInfoAsycWorker()
{
}

}
