// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINFOASYCWORKER_H
#define FILEINFOASYCWORKER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include "thumbnailprovider.h"

#include <dfm-io/dfileinfo.h>

#include <QObject>
#include <QMimeDatabase>

namespace dfmbase {
struct FileInfoHelperUeserData
{
    std::atomic_bool finish { false };
    QVariant data;
};
class FileInfoAsycWorker : public QObject
{
    Q_OBJECT
    friend class FileInfoHelper;

public:
    ~FileInfoAsycWorker() override;

private:
    explicit FileInfoAsycWorker(QObject *parent = nullptr);
    inline void stopWorker()
    {
        stoped = true;
    }

    bool isStoped() const
    {
        return stoped;
    }

    bool checkThumbEnable(FileInfoPointer fileInfo) const;

Q_SIGNALS:
    void fileConutAsyncFinish(const QUrl &url, int files);
    void fileMimeTypeFinished(const QUrl &url, const QMimeType &type);
    void createThumbnailFinished(const QUrl &sourceFile, const QString &thumbnailPath);
    void createThumbnailFailed(const QUrl &sourceFile);
private Q_SLOTS:
    void fileConutAsync(const QUrl &url, const QSharedPointer<FileInfoHelperUeserData> data);
    void fileMimeType(const QUrl &url, const QMimeDatabase::MatchMode mode, const QString &inod, const bool isGvfs, const QSharedPointer<FileInfoHelperUeserData> data);
    void fileThumb(const QUrl &url, ThumbnailProvider::Size size, const QSharedPointer<FileInfoHelperUeserData> data);
    void fileRefresh(const QUrl &url, const QSharedPointer<dfmio::DFileInfo> dfileInfo);

private:
    std::atomic_bool stoped { false };
};

}

using InfoHelperUeserDataPointer = QSharedPointer<dfmbase::FileInfoHelperUeserData>;
Q_DECLARE_METATYPE(InfoHelperUeserDataPointer)

#endif   // FILEINFOASYCWORKER_H
