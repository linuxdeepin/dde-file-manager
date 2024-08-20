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
    std::atomic_bool queringAttribute { false };
    std::atomic_bool cacheingAttributes { false };
    std::atomic_bool needUpdateMediaInfo;
    DFileInfo::MediaType mediaType { DFileInfo::MediaType::kGeneral};
    char memrySeat[7];
    QSharedPointer<DFileInfo> dfmFileInfo { nullptr };   // dfm文件的信息
    QVariantHash extraProperties;   // 扩展属性列表
    QMap<DFileInfo::AttributeExtendID, QVariant> attributesExtend;   // 缓存的fileinfo 扩展信息
    QList<DFileInfo::AttributeExtendID> extendIDs;
    QMimeType mimeType;
    QReadWriteLock lock;
    QReadWriteLock iconLock;
    QIcon fileIcon;
    QSharedPointer<InfoDataFuture> mediaFuture { nullptr };
    InfoHelperUeserDataPointer fileCountFuture { nullptr };
    InfoHelperUeserDataPointer updateFileCountFuture { nullptr };
    QMap<FileInfo::FileInfoAttributeID, QVariant> cacheAsyncAttributes;
    QReadWriteLock notifyLock;
    QMultiMap<QUrl, QString> notifyUrls;
    quint64 tokenKey{0};
    AsyncFileInfo *const q;
    QReadWriteLock changesLock;
    QList<FileInfo::FileInfoAttributeID> changesAttributes;

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
    QIcon defaultIcon();

public:
    QString fileName() const;
    QString completeBaseName() const;
    QString completeSuffix() const;
    QString iconName() const;
    QString mimeTypeName() const;
    QString fileDisplayName() const;
    QString path() const;
    QString filePath() const;
    QString symLinkTarget() const;
    QUrl redirectedFileUrl() const;
    bool isExecutable() const;
    bool isPrivate() const;
    bool canDelete() const;
    bool canTrash() const;
    bool canRename() const;
    bool canFetch() const;
    QString sizeFormat() const;
    QVariant attribute(DFileInfo::AttributeID key, bool *ok = nullptr) const;
    QVariant asyncAttribute(FileInfo::FileInfoAttributeID key) const;
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> mediaInfo(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids);

    FileInfo::FileType fileType() const;
    int cacheAllAttributes(const QString &attributes = QString());
    bool inserAsyncAttribute(const FileInfo::FileInfoAttributeID id, const QVariant &value);
    void fileMimeTypeAsync(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault);
    QMimeType mimeTypes(const QString &filePath, QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault,
                        const QString &inod = QString(), const bool isGvfs = false);
    void init(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo = nullptr);
    void updateThumbnail(const QUrl &url);
    QIcon updateIcon();
    void updateMediaInfo(const DFileInfo::MediaType type, const QList<DFileInfo::AttributeExtendID> &ids);
    bool hasAsyncAttribute(FileInfo::FileInfoAttributeID key);
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
