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

bool FileInfoAsycWorker::checkThumbEnable(FileInfoPointer fileInfo) const
{
#ifdef DFM_MINIMUM
    return false;
#endif
    if (!fileInfo)
        return false;

    const QUrl &fileUrl = fileInfo->urlOf(UrlInfoType::kUrl);

    bool isLocalDevice = FileUtils::isLocalDevice(fileUrl);
    bool isCdRomDevice = FileUtils::isCdRomDevice(fileUrl);

    bool thumbEnabled = isLocalDevice && !isCdRomDevice;
    if (!thumbEnabled && ThumbnailProvider::instance()->thumbnailEnable(fileUrl))
        thumbEnabled = true;

    bool hasThumbnail = ThumbnailProvider::instance()->hasThumbnail(fileInfo->fileMimeType());

    return thumbEnabled && hasThumbnail;
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

void FileInfoAsycWorker::fileThumb(const QUrl &url, ThumbnailProvider::Size size, const QSharedPointer<FileInfoHelperUeserData> data)
{
    if (isStoped())
        return;

    const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(url);

    if (!checkThumbEnable(fileInfo)) {
        data->finish = true;
        data->data = false;
        emit createThumbnailFailed(url);
        return;
    }

    const QString &thumb = ThumbnailProvider::instance()->createThumbnail(fileInfo, size);
    data->finish = true;

    if (thumb.isEmpty()) {
        emit createThumbnailFailed(url);
    } else {
        data->data = true;
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
