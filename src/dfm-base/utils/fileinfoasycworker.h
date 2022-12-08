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

#ifndef FILEINFOASYCWORKER_H
#define FILEINFOASYCWORKER_H

#include "dfm_base_global.h"
#include "dfm_global_defines.h"
#include "thumbnailprovider.h"

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

Q_SIGNALS:
    void fileConutAsyncFinish(const QUrl &url, int files);
    void fileMimeTypeFinished(const QUrl &url, const QMimeType &type);
    void createThumbnailFinished(const QUrl &sourceFile, const QString &thumbnailPath);
    void createThumbnailFailed(const QUrl &sourceFile);
private Q_SLOTS:
    void fileConutAsync(const QUrl &url, const QSharedPointer<FileInfoHelperUeserData> data);
    void fileMimeType(const QUrl &url, const QMimeDatabase::MatchMode mode, const QString &inod, const bool isGvfs, const QSharedPointer<FileInfoHelperUeserData> data);
    void fileThumb(const QUrl &url, ThumbnailProvider::Size size, const QSharedPointer<FileInfoHelperUeserData> data);

private:
    std::atomic_bool stoped { false };
};

}

using InfoHelperUeserDataPointer = QSharedPointer<dfmbase::FileInfoHelperUeserData>;
Q_DECLARE_METATYPE(InfoHelperUeserDataPointer)

#endif   // FILEINFOASYCWORKER_H
