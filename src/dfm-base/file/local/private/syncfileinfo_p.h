// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYNCFILEINFO_P_H
#define SYNCFILEINFO_P_H

#include "infodatafuture.h"

#include <dfm-base/interfaces/private/fileinfo_p.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/fileinfohelper.h>

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
class SyncFileInfoPrivate
{
public:
    SyncFileInfo *const q;
    FileInfo::FileType fileType { FileInfo::FileType::kUnknown };   // 缓存文件的FileType
    DFileInfo::MediaType mediaType { DFileInfo::MediaType::kGeneral};
    DMimeDatabase mimeDb;
    QMimeDatabase::MatchMode mimeTypeMode;
    QSharedPointer<DFileInfo> dfmFileInfo { nullptr };   // dfm文件的信息
    QVariantHash extraProperties;   // 扩展属性列表
    QMap<DFileInfo::AttributeExtendID, QVariant> attributesExtend;   // 缓存的fileinfo 扩展信息
    QList<DFileInfo::AttributeExtendID> extendIDs;
    QMimeType mimeType;
    QReadWriteLock lock;
    QMutex mutex;
    QReadWriteLock iconLock;
    QIcon fileIcon;
    QVariant isLocalDevice = true;
    QVariant isCdRomDevice;
    QSharedPointer<InfoDataFuture> mediaFuture { nullptr };
    InfoHelperUeserDataPointer fileMimeTypeFuture { nullptr };
    QMap<DFMIO::DFileInfo::AttributeID, QVariant> cacheAttributes;

public:
    explicit SyncFileInfoPrivate(SyncFileInfo *qq);
    virtual ~SyncFileInfoPrivate();
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
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> mediaInfo(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids);
    void init(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo = nullptr);
    QMimeType mimeTypes(const QString &filePath, QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault,
                        const QString &inod = QString(), const bool isGvfs = false);
    FileInfo::FileType updateFileType();
    QIcon updateIcon();
    void updateMediaInfo(const DFileInfo::MediaType type, const QList<DFileInfo::AttributeExtendID> &ids);
};
}
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::SyncFileInfoPrivate *)

#endif   // SYNCFILEINFO_P_H
