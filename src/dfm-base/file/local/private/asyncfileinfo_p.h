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
#include <QMutex>
#include <QMutexLocker>

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
    DFileInfo::MediaType mediaType { DFileInfo::MediaType::kGeneral };
    char memrySeat[7];
    QSharedPointer<DFileInfo> dfmFileInfo { nullptr };   // dfm文件的信息
    QVariantHash extraProperties;   // 扩展属性列表
    QMap<DFileInfo::AttributeExtendID, QVariant> attributesExtend;   // 缓存的fileinfo 扩展信息
    QList<DFileInfo::AttributeExtendID> extendIDs;
    QMimeType mimeType;
    mutable QMutex lock;
    QReadWriteLock iconLock;
    QIcon fileIcon;
    QSharedPointer<InfoDataFuture> mediaFuture { nullptr };
    InfoHelperUeserDataPointer fileCountFuture { nullptr };
    InfoHelperUeserDataPointer updateFileCountFuture { nullptr };
    QMap<FileInfo::FileInfoAttributeID, QVariant> cacheAsyncAttributes;
    mutable QMutex notifyLock;
    QMultiMap<QUrl, QString> notifyUrls;
    quint64 tokenKey { 0 };
    AsyncFileInfo *const q;
    mutable QMutex changesLock;
    QList<FileInfo::FileInfoAttributeID> changesAttributes;

public:
    explicit AsyncFileInfoPrivate(AsyncFileInfo *qq);
    virtual ~AsyncFileInfoPrivate();

    QIcon defaultIcon();

public:
    QString fileName() const;
    QString baseName() const;
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
    bool insertAsyncAttribute(const FileInfo::FileInfoAttributeID id, const QVariant &value);
    void fileMimeTypeAsync(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault);
    QMimeType mimeTypes(const QString &filePath, QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault,
                        const QString &inod = QString(), const bool isGvfs = false);
    void init(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo = nullptr);
    void updateThumbnail(const QUrl &url);
    QIcon updateIcon();
    void updateMediaInfo(const DFileInfo::MediaType type, const QList<DFileInfo::AttributeExtendID> &ids);
    bool hasAsyncAttribute(FileInfo::FileInfoAttributeID key);
};

}
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::AsyncFileInfoPrivate *)

#endif   // ASYNCFILEINFO_P_H
