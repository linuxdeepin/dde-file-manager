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
#ifndef FILEINFO_H
#define FILEINFO_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QIcon>
#include <QPointF>

namespace dfmbase {

class LocalFileInfoPrivate;
class LocalFileInfo : public AbstractFileInfo
{
    Q_OBJECT
    LocalFileInfoPrivate *d = nullptr;

public:
    enum FlagIcon {
        kWritable,   // 是否可写
        kSymLink,   // 是否是链接文件
        kReadable,   // 是否可读
    };
    Q_ENUMS(FlagIcon)

    explicit LocalFileInfo(const QUrl &url);
    LocalFileInfo(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo);
    virtual ~LocalFileInfo() override;

    LocalFileInfo &operator=(const LocalFileInfo &info);
    virtual bool operator==(const LocalFileInfo &fileinfo) const;
    virtual bool operator!=(const LocalFileInfo &fileinfo) const;

    virtual bool initQuerier() override;
    virtual void initQuerierAsync(int ioPriority = 0, initQuerierAsyncCallback func = nullptr, void *userData = nullptr) override;
    virtual void setFile(const QUrl &url) override;
    virtual bool exists() const override;
    virtual void refresh() override;
    virtual void refresh(DFMIO::DFileInfo::AttributeID id, const QVariant &value = QVariant()) override;
    virtual QString filePath() const override;
    virtual QString absoluteFilePath() const override;
    virtual QString fileName() const override;
    virtual QString baseName() const override;
    virtual QString completeBaseName() const override;
    virtual QString suffix() const override;
    virtual QString completeSuffix() override;
    virtual QString path() const override;
    virtual QString absolutePath() const override;
    virtual QString canonicalPath() const override;
    virtual QDir dir() const override;
    virtual QDir absoluteDir() const override;
    virtual QUrl url() const override;
    virtual bool canDelete() const override;
    virtual bool canTrash() const override;
    virtual bool canRename() const override;
    virtual bool canHidden() const override;
    virtual bool isReadable() const override;
    virtual bool isWritable() const override;
    virtual bool isExecutable() const override;
    virtual bool isHidden() const override;
    virtual bool isFile() const override;
    virtual bool isDir() const override;
    virtual bool isSymLink() const override;
    virtual bool isRoot() const override;
    virtual bool isBundle() const override;
    virtual bool isPrivate() const override;
    virtual bool canFetch() const override;
    virtual quint64 inode() const override;
    virtual QString symLinkTarget() const override;
    virtual QString owner() const override;
    virtual uint ownerId() const override;
    virtual QString group() const override;
    virtual uint groupId() const override;
    virtual bool permission(QFile::Permissions permissions) const override;
    virtual QFile::Permissions permissions() const override;
    virtual qint64 size() const override;
    virtual QDateTime created() const override;
    virtual QDateTime birthTime() const override;
    virtual QDateTime metadataChangeTime() const override;
    virtual QDateTime lastModified() const override;
    virtual QDateTime lastRead() const override;
    virtual QDateTime fileTime(QFile::FileTime time) const override;
    virtual QVariantHash extraProperties() const override;
    virtual QIcon fileIcon() override;
    virtual QString iconName() override;
    virtual QString genericIconName() override;
    virtual QUrl redirectedFileUrl() const override;

    virtual bool isBlockDev() const;
    virtual QString mountPath() const;
    virtual bool isCharDev() const;
    virtual bool isFifo() const;
    virtual bool isSocket() const;
    virtual bool isRegular() const;
    virtual AbstractFileInfo::FileType fileType() const override;
    virtual int countChildFile() const override;
    virtual QString sizeFormat() const override;
    virtual QString fileDisplayName() const override;
    virtual QString fileCopyName() const override;
    virtual QFileInfo toQFileInfo() const;
    virtual QMimeType fileMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) override;
    virtual QString mimeTypeName() override;

    virtual QString emptyDirectoryTip() const override;

    virtual bool canDragCompress() const override;
    virtual bool isDragCompressFileFormat() const override;

    // emblems
    virtual void setEmblems(const QMap<int, QIcon> &maps) override;
    virtual QMap<int, QIcon> emblems() const override;
    virtual bool emblemsInited() const override;
    virtual QVariant customAttribute(const char *key, const DFMIO::DFileInfo::DFileAttributeType type) override;

    // media info
    virtual void mediaInfoAttributes(DFMIO::DFileInfo::MediaType type, QList<DFMIO::DFileInfo::AttributeExtendID> ids) const override;

    virtual bool notifyAttributeChanged() override;

    // cache attribute
    virtual void cacheAttribute(const DFMIO::DFileInfo::AttributeID id, const QVariant &value) override;
    virtual QVariant attribute(const DFMIO::DFileInfo::AttributeID id) override;

    virtual void setIsLocalDevice(const bool isLocalDevice) override;
    virtual void setIsCdRomDevice(const bool isCdRomDevice) override;

private:
    void init(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo = nullptr);
    QMimeType mimeType(const QString &filePath, QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault, const QString inod = QString(), const bool isGvfs = false);
};
}
typedef QSharedPointer<DFMBASE_NAMESPACE::LocalFileInfo> DFMLocalFileInfoPointer;

#endif   // FILEINFO_H
