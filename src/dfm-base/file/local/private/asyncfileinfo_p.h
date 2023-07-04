// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ASYNCFILEINFO_P_H
#define ASYNCFILEINFO_P_H

#include "infodatafuture.h"

#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/mimetype/dmimedatabase.h>

#include <dfm-io/dfilefuture.h>

#include <QFileInfo>
#include <QIcon>
#include <QTimer>
#include <QFuture>
#include <QQueue>
#include <QMimeType>
#include <QReadWriteLock>
#include <QReadLocker>

namespace dfmbase {
class AsyncFileInfoPrivate
{
public:
    friend class AsyncFileInfo;
    DMimeDatabase mimeDb;
    QMimeDatabase::MatchMode mimeTypeMode;
    std::atomic_bool notInit { false };
    std::atomic_bool cacheing { false };
    char memrySeat[5];
    QSharedPointer<DFileInfo> dfmFileInfo { nullptr };   // dfm文件的信息
    QVariantHash extraProperties;   // 扩展属性列表
    QMap<DFileInfo::AttributeExtendID, QVariant> attributesExtend;   // 缓存的fileinfo 扩展信息
    QList<DFileInfo::AttributeExtendID> extendIDs;
    QMimeType mimeType;
    QReadWriteLock lock;
    enum IconType {
        kDefaultIcon,
        kThumbIcon,
    };
    QReadWriteLock iconLock;
    QMap<IconType, QIcon> icons;
    QSharedPointer<InfoDataFuture> mediaFuture { nullptr };
    InfoHelperUeserDataPointer fileCountFuture { nullptr };
    InfoHelperUeserDataPointer fileMimeTypeFuture { nullptr };
    QMap<AsyncFileInfo::AsyncAttributeID, QVariant> cacheAsyncAttributes;
    QReadWriteLock notifyLock;
    QMap<QUrl, QString> notifyUrls;
    AsyncFileInfo *const q;

public:
    explicit AsyncFileInfoPrivate(AsyncFileInfo *qq);
    virtual ~AsyncFileInfoPrivate();
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
    }

    QIcon thumbIcon();
    QIcon defaultIcon();

public:
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
    QVector<AsyncFileInfo::AsyncAttributeID> &getAttributeIDVector() const
    {
        static QVector<AsyncFileInfo::AsyncAttributeID> kTimeInfoToDFile {
            AsyncFileInfo::AsyncAttributeID::kTimeCreated,
            AsyncFileInfo::AsyncAttributeID::kTimeCreated,
            AsyncFileInfo::AsyncAttributeID::kTimeChanged,
            AsyncFileInfo::AsyncAttributeID::kTimeModified,
            AsyncFileInfo::AsyncAttributeID::kTimeAccess,
            AsyncFileInfo::AsyncAttributeID::kTimeAccess,
            AsyncFileInfo::AsyncAttributeID::kTimeCreated,
            AsyncFileInfo::AsyncAttributeID::kTimeCreated,
            AsyncFileInfo::AsyncAttributeID::kTimeChanged,
            AsyncFileInfo::AsyncAttributeID::kTimeModified,
            AsyncFileInfo::AsyncAttributeID::kTimeAccess,
            AsyncFileInfo::AsyncAttributeID::kTimeAccess,
            AsyncFileInfo::AsyncAttributeID::kTimeCreatedUsec,
            AsyncFileInfo::AsyncAttributeID::kTimeCreatedUsec,
            AsyncFileInfo::AsyncAttributeID::kTimeChangedUsec,
            AsyncFileInfo::AsyncAttributeID::kTimeModifiedUsec,
            AsyncFileInfo::AsyncAttributeID::kTimeAccessUsec,
        };
        return kTimeInfoToDFile;
    }
    QUrl redirectedFileUrl() const;
    QVector<AsyncFileInfo::AsyncAttributeID> &getAttributeIDIsVector() const
    {
        static QVector<AsyncFileInfo::AsyncAttributeID> kIsToDFile {
            AsyncFileInfo::AsyncAttributeID::kAccessCanRead,
            AsyncFileInfo::AsyncAttributeID::kAccessCanWrite,
            AsyncFileInfo::AsyncAttributeID::kAccessCanExecute,
            AsyncFileInfo::AsyncAttributeID::kStandardIsHidden,
            AsyncFileInfo::AsyncAttributeID::kStandardIsFile,
            AsyncFileInfo::AsyncAttributeID::kStandardIsDir,
            AsyncFileInfo::AsyncAttributeID::kStandardIsSymlink,
        };

        return kIsToDFile;
    }
    bool isExecutable() const;
    bool isPrivate() const;
    bool canDelete() const;
    bool canTrash() const;
    bool canRename() const;
    bool canFetch() const;
    QVector<AsyncFileInfo::AsyncAttributeID> &getAttributeIDExtendVector() const
    {
        static QVector<AsyncFileInfo::AsyncAttributeID> kExtendToDFile = {
            AsyncFileInfo::AsyncAttributeID::kOwnerUser,
            AsyncFileInfo::AsyncAttributeID::kOwnerGroup,
            AsyncFileInfo::AsyncAttributeID::kAttributeIDMax,
            AsyncFileInfo::AsyncAttributeID::kUnixInode,
            AsyncFileInfo::AsyncAttributeID::kUnixUID,
            AsyncFileInfo::AsyncAttributeID::kUnixGID,
            AsyncFileInfo::AsyncAttributeID::kStandardIsHidden,
        };

        return kExtendToDFile;
    }
    QString sizeFormat() const;
    QVariant attribute(DFileInfo::AttributeID key, bool *ok = nullptr) const;
    QVariant asyncAttribute(AsyncFileInfo::AsyncAttributeID key) const;
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> mediaInfo(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids);

    FileInfo::FileType fileType() const;
    void cacheAllAttributes();
    void fileMimeTypeAsync(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault);
    QMimeType mimeTypes(const QString &filePath, QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault,
                        const QString &inod = QString(), const bool isGvfs = false);
    void init(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo = nullptr);
};

AsyncFileInfoPrivate::AsyncFileInfoPrivate(AsyncFileInfo *qq)
    : q(qq)
{
}

AsyncFileInfoPrivate::~AsyncFileInfoPrivate()
{
}

QMimeType AsyncFileInfoPrivate::readMimeType(QMimeDatabase::MatchMode mode) const
{
    QUrl url = q->urlOf(UrlInfoType::kUrl);
    if (dfmbase::FileUtils::isLocalFile(url))
        return mimeDb.mimeTypeForUrl(url);
    else
        return mimeDb.mimeTypeForFile(UrlRoute::urlToPath(url),
                                      mode);
}
}
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::AsyncFileInfoPrivate *)

#endif   // ASYNCFILEINFO_P_H
