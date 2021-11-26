/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef LOCALFILEINFO_P_H
#define LOCALFILEINFO_P_H

#include "base/private/abstractfileinfo_p.h"
#include "localfile/localfileinfo.h"
#include "mimetype/mimedatabase.h"

#include <QFileInfo>
#include <QIcon>
#include <QTimer>
#include <QFuture>
#include <QQueue>
#include <QMimeType>
#include <QReadWriteLock>

DFMBASE_BEGIN_NAMESPACE
class LocalFileInfoPrivate : public AbstractFileInfoPrivate
{
    friend class LocalFileInfo;
    LocalFileInfo *const q;
    // 文件的inode，唯一表示符
    QAtomicInteger<quint64> inode { 0 };
    // 扩展属性列表
    QVariantHash extraProperties;
    // 缓存文件的FileType
    MimeDatabase::FileType fileType { MimeDatabase::FileType::Unknown };
    // 文件缓存
    DThreadMap<AbstractFileInfo::FileInfoCacheType, QVariant> caches;
    // dfm文件的信息
    QSharedPointer<DFileInfo> dfmFileInfo { nullptr };

public:
    explicit LocalFileInfoPrivate(LocalFileInfo *qq);
    virtual ~LocalFileInfoPrivate();
    QString sizeString(const QString &str) const
    {
        int begin_pos = str.indexOf('.');

        if (begin_pos < 0)
            return str;

        QString size = str;

        while (size.count() - 1 > begin_pos) {
            if (!size.endsWith('0'))
                return size;

            size = size.left(size.count() - 1);
        }

        return size.left(size.count() - 1);
    }
    virtual QMimeType readMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const;
};

LocalFileInfoPrivate::LocalFileInfoPrivate(LocalFileInfo *qq)
    : AbstractFileInfoPrivate(qq), q(qq)
{
}

LocalFileInfoPrivate::~LocalFileInfoPrivate()
{
}

QMimeType LocalFileInfoPrivate::readMimeType(QMimeDatabase::MatchMode mode) const
{
    QUrl url = q->url();
    if (url.isLocalFile())
        return MimeDatabase::mimeTypeForUrl(url);
    else
        return MimeDatabase::mimeTypeForFile(UrlRoute::urlToPath(url),
                                             mode);
}

DFMBASE_END_NAMESPACE
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::LocalFileInfoPrivate *)

#endif   // LOCALFILEINFO_P_H
