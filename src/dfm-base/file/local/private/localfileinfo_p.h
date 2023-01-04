/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#include "interfaces/private/abstractfileinfo_p.h"
#include "file/local/localfileinfo.h"
#include "mimetype/mimedatabase.h"
#include "infodatafuture.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/utils/thumbnailprovider.h"
#include "dfm-base/utils/fileinfohelper.h"

#include <dfm-io/core/dfilefuture.h>

#include <QFileInfo>
#include <QIcon>
#include <QTimer>
#include <QFuture>
#include <QQueue>
#include <QMimeType>
#include <QReadWriteLock>
#include <QReadLocker>

namespace dfmbase {
class LocalFileInfoPrivate : public AbstractFileInfoPrivate
{
    friend class LocalFileInfo;
    std::atomic_bool loadingThumbnail = { false };
    MimeDatabase::FileType fileType { MimeDatabase::FileType::kUnknown };   // 缓存文件的FileType
    QMimeDatabase::MatchMode mimeTypeMode;
    std::atomic_int enableThumbnail = { -1 };   // 小于0时表示此值未初始化，0表示不支持，1表示支持
    QSharedPointer<DFileInfo> dfmFileInfo { nullptr };   // dfm文件的信息
    QVariantHash extraProperties;   // 扩展属性列表
    QMap<DFileInfo::AttributeExtendID, QVariant> attributesExtend;   // 缓存的fileinfo 扩展信息
    QList<DFileInfo::AttributeExtendID> extendIDs;
    QMimeType mimeType;
    QReadWriteLock lock;
    QMutex mutex;
    enum IconType {
        kDefaultIcon,
        kThumbIcon,
    };
    QReadWriteLock iconLock;
    QMap<IconType, QIcon> icons;
    QVariant isLocalDevice;
    QVariant isCdRomDevice;
    QSharedPointer<InfoDataFuture> mediaFuture { nullptr };
    InfoHelperUeserDataPointer fileCountFuture { nullptr };
    InfoHelperUeserDataPointer fileMimeTypeFuture { nullptr };
    InfoHelperUeserDataPointer iconFuture { nullptr };

public:
    explicit LocalFileInfoPrivate(const QUrl &url, LocalFileInfo *qq);
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

    void clearIcon()
    {
        icons.clear();
        loadingThumbnail = false;
        enableThumbnail = -1;
        iconFuture.reset(nullptr);
    }

    QIcon thumbIcon();
    QIcon defaultIcon();

private:
    QString fileName() const;
    QString baseName() const;
    QString completeBaseName() const;
    QString suffix() const;
    QString completeSuffix() const;
    QString iconName() const;
    QString mimeTypeName() const;
    QString fileDisplayName() const;
    QString path() const;
    QString filePath() const;
    QString symLinkTarget() const;
    QVector<DFileInfo::AttributeID> getAttributeIDVector() const
    {
        static QVector<DFileInfo::AttributeID> kTimeInfoToDFile = {
            DFileInfo::AttributeID::kTimeCreated,
            DFileInfo::AttributeID::kTimeCreated,
            DFileInfo::AttributeID::kTimeChanged,
            DFileInfo::AttributeID::kTimeModified,
            DFileInfo::AttributeID::kTimeAccess,
            DFileInfo::AttributeID::kTimeAccess,
            DFileInfo::AttributeID::kTimeCreated,
            DFileInfo::AttributeID::kTimeCreated,
            DFileInfo::AttributeID::kTimeChanged,
            DFileInfo::AttributeID::kTimeModified,
            DFileInfo::AttributeID::kTimeAccess,
            DFileInfo::AttributeID::kTimeAccess,
            DFileInfo::AttributeID::kTimeCreatedUsec,
            DFileInfo::AttributeID::kTimeCreatedUsec,
            DFileInfo::AttributeID::kTimeChangedUsec,
            DFileInfo::AttributeID::kTimeModifiedUsec,
            DFileInfo::AttributeID::kTimeAccessUsec,
        };
        return kTimeInfoToDFile;
    }
    QUrl redirectedFileUrl() const;
    QVector<DFileInfo::AttributeID> getAttributeIDIsVector() const
    {
        static QVector<DFileInfo::AttributeID> kIsToDFile = {
            DFileInfo::AttributeID::kAccessCanRead,
            DFileInfo::AttributeID::kAccessCanWrite,
            DFileInfo::AttributeID::kAccessCanExecute,
            DFileInfo::AttributeID::kStandardIsHidden,
            DFileInfo::AttributeID::kStandardIsFile,
            DFileInfo::AttributeID::kStandardIsDir,
            DFileInfo::AttributeID::kStandardIsSymlink,
        };

        return kIsToDFile;
    }
    bool isExecutable() const;
    bool isPrivate() const;
    bool canDelete() const;
    bool canTrash() const;
    bool canRename() const;
    bool canFetch() const;
    QVector<DFileInfo::AttributeID> getAttributeIDExtendVector() const
    {
        static QVector<DFileInfo::AttributeID> kExtendToDFile = {
            DFileInfo::AttributeID::kOwnerUser,
            DFileInfo::AttributeID::kOwnerGroup,
            DFileInfo::AttributeID::kAttributeIDMax,
            DFileInfo::AttributeID::kUnixInode,
            DFileInfo::AttributeID::kUnixUID,
            DFileInfo::AttributeID::kUnixGID,
            DFileInfo::AttributeID::kStandardIsHidden,
        };

        return kExtendToDFile;
    }
    QString sizeFormat() const;
    QVariant attribute(DFileInfo::AttributeID key, bool *ok = nullptr) const;
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> mediaInfo(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids);
    bool canThumb() const;
};

LocalFileInfoPrivate::LocalFileInfoPrivate(const QUrl &url, LocalFileInfo *qq)
    : AbstractFileInfoPrivate(url, qq)
{
}

LocalFileInfoPrivate::~LocalFileInfoPrivate()
{
}

QMimeType LocalFileInfoPrivate::readMimeType(QMimeDatabase::MatchMode mode) const
{
    QUrl url = q->urlOf(UrlInfoType::kUrl);
    if (url.isLocalFile())
        return MimeDatabase::mimeTypeForUrl(url);
    else
        return MimeDatabase::mimeTypeForFile(UrlRoute::urlToPath(url),
                                             mode);
}
}
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::LocalFileInfoPrivate *)

#endif   // LOCALFILEINFO_P_H
