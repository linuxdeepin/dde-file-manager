/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
    static DFMBASE_NAMESPACE::DMimeDatabase db;
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

void FileInfoAsycWorker::fileThumb(const QUrl &url, ThumbnailProvider::Size size, const QSharedPointer<FileInfoHelperUeserData> data)
{
    if (isStoped())
        return;
    const QString &thumb = ThumbnailProvider::instance()->createThumbnail(url, size);
    data->finish = true;
    data->data = thumb;
    if (thumb.isEmpty()) {
        emit createThumbnailFailed(url);
    } else {
        emit createThumbnailFinished(url, thumb);
    }
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
