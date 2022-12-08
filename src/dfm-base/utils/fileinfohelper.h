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

#ifndef FILEINFOHELPER_H
#define FILEINFOHELPER_H

#include "dfm_base_global.h"
#include "utils/thumbnailprovider.h"
#include "fileinfoasycworker.h"

#include <dfm-io/core/dfileinfo.h>

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
private Q_SLOTS:
    void aboutToQuit();

private:
    QSharedPointer<QThread> thread { nullptr };
    QSharedPointer<FileInfoAsycWorker> worker { nullptr };
    std::atomic_bool stoped { false };
};
}

#endif   // FILEINFOHELPER_H
