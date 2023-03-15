// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINFOHELPER_H
#define FILEINFOHELPER_H

#include "dfm-base/dfm_base_global.h"
#include "fileinfoasycworker.h"

#include "dfm-base/utils/thumbnailprovider.h"

#include <dfm-io/dfileinfo.h>

#include <QObject>
#include <QThread>
#include <QVariant>
#include <QMimeDatabase>

namespace dfmbase {
class FileInfoHelper : public QObject
{
    Q_OBJECT
public:
    ~FileInfoHelper() override;
    static FileInfoHelper &instance();
    QSharedPointer<FileInfoHelperUeserData> fileCountAsync(QUrl &url);
    QSharedPointer<FileInfoHelperUeserData> fileMimeTypeAsync(const QUrl &url, const QMimeDatabase::MatchMode mode,
                                                              const QString &inod, const bool isGvfs);
    QSharedPointer<FileInfoHelperUeserData> fileThumbAsync(const QUrl &url, ThumbnailProvider::Size size);
    void fileRefreshAsync(const QUrl &url, const QSharedPointer<dfmio::DFileInfo> dfileInfo);

private:
    explicit FileInfoHelper(QObject *parent = nullptr);
    void init();

private:
    // send for other
Q_SIGNALS:
    void createThumbnailFinished(const QUrl &sourceFile, const QString &thumbnailPath);
    void createThumbnailFailed(const QUrl &sourceFile);
    void mediaDataFinished(const QUrl &sourceFile, QMap<dfmio::DFileInfo::AttributeExtendID, QVariant> properties);
    void fileCountFinished(const QUrl &url, const int fileCount);
    void fileMimeTypeFinished(const QUrl &url, const QMimeType &type);
    // shend to fileinfoasyncworker for async get attribute
    void fileCount(const QUrl &url, const QSharedPointer<FileInfoHelperUeserData> data);
    void fileMimeType(const QUrl &url, const QMimeDatabase::MatchMode mode, const QString &inod,
                      const bool isGvfs, const QSharedPointer<FileInfoHelperUeserData> data);
    void fileThumb(const QUrl &url, ThumbnailProvider::Size size, const QSharedPointer<FileInfoHelperUeserData> data);
    void fileInfoRefresh(const QUrl &url, QSharedPointer<dfmio::DFileInfo> dfileInfo);
private Q_SLOTS:
    void aboutToQuit();

private:
    QSharedPointer<QThread> thread { nullptr };
    QSharedPointer<FileInfoAsycWorker> worker { nullptr };
    std::atomic_bool stoped { false };
};
}

#endif   // FILEINFOHELPER_H
