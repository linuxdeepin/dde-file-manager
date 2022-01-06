/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef ABSTRACTFILEINFO_H
#define ABSTRACTFILEINFO_H

#include "dfm-base/base/urlroute.h"
#include "dfm-base/dfm_base_global.h"
#include "dfm-base/mimetype/mimedatabase.h"

#include <dfm-io/core/dfileinfo.h>

#include <QSharedData>
#include <QFile>
#include <QMimeType>

class QDir;
class QDateTime;

DFMBASE_BEGIN_NAMESPACE

class AbstractFileInfoPrivate;
class AbstractFileInfo : public QSharedData
{

public:
    /*!
     * \enum FileInfoCacheType 文件缓存的key值
     * \brief 文件缓存的key值，不同的key对应不同的文件属性，用于在DAbstractFileInfoPrivate中缓存文件信息的key
     */
    enum FileInfoCacheType {
        kTypeExists,   // 文件存在
        kTypeFilePath,   // 文件路径
        kTypeAbsoluteFilePath,   // 文件绝对路径
        kTypeFileName,   // 文件名称
        kTypeBaseName,   // 文件基础名称
        kTypeCompleteBaseName,   // 文件完整的基础名称
        kTypeSuffix,   // 文件的suffix
        kTypeCompleteSuffix,   // 文件的完成suffix
        kTypePath,   // 路径
        kTypeIsReadable,   // 文件是否可读
        kTypeIsWritable,   // 文件是否可写
        kTypeIsExecutable,   // 文件是否可执行
        kTypeIsHidden,   // 文件是否隐藏
        kTypeIsFile,   // 是否是文件
        kTypeIsDir,   // 是否是目录
        kTypeIsSymLink,   // 是否是链接文件
        kTypeSymLinkTarget,   // 链接文件的目标文件
        kTypeOwner,   // 文件的拥有者
        kTypeOwnerID,   // 文件的拥有者的id
        kTypeGroup,   // 文件所在组
        kTypeGroupID,   // 文件所在组的id
        kTypePermissions,   // 文件的所有权限
        kTypeSize,   // 文件的大小
        kTypeCreateTime,   // 文件的创建时间
        kTypeChangeTime,   // 文件的改变时间
        kTypeLastModifyTime,   // 文件的最后修改时间
        kTypeLastReadTime,   // 文件的最后修改时间
        kTypeUnknow = 255
    };
    enum Type {
        kDirectory = MimeDatabase::kDirectory,
        kCharDevice = MimeDatabase::kCharDevice,
        kBlockDevice = MimeDatabase::kBlockDevice,
        kFIFOFile = MimeDatabase::kFIFOFile,
        kSocketFile = MimeDatabase::kSocketFile,
        kRegularFile = MimeDatabase::kRegularFile,
        kDocuments = MimeDatabase::kDocuments,
        kImages = MimeDatabase::kImages,
        kVideos = MimeDatabase::kVideos,
        kAudios = MimeDatabase::kAudios,
        kArchives = MimeDatabase::kArchives,
        kDesktopApplication = MimeDatabase::kDesktopApplication,
        kExecutable = MimeDatabase::kExecutable,
        kBackups = MimeDatabase::kBackups,
        kUnknown = MimeDatabase::kUnknown,
        kCustomType = MimeDatabase::kCustomType
    };

public:
    explicit AbstractFileInfo() = delete;
    virtual ~AbstractFileInfo();
    virtual AbstractFileInfo &operator=(const AbstractFileInfo &fileinfo);
    virtual bool operator==(const AbstractFileInfo &fileinfo) const;
    virtual bool operator!=(const AbstractFileInfo &fileinfo) const;

    virtual void setFile(const QUrl &url);
    virtual void setFile(const DFMIO::DFileInfo &file);
    virtual bool exists() const;
    virtual void refresh();
    virtual QString filePath() const;
    virtual QString absoluteFilePath() const;
    virtual QString fileName() const;
    virtual QString baseName() const;
    virtual QString completeBaseName() const;
    virtual QString suffix() const;
    virtual QString completeSuffix();
    virtual QString path() const;
    virtual QString absolutePath() const;
    virtual QString canonicalPath() const;
    virtual QDir dir() const;
    virtual QDir absoluteDir() const;
    virtual QUrl url() const;
    virtual bool isReadable() const;
    virtual bool isWritable() const;
    virtual bool isExecutable() const;
    virtual bool isHidden() const;
    virtual bool isFile() const;
    virtual bool isDir() const;
    virtual bool isSymLink() const;
    virtual bool isRoot() const;
    virtual bool isBundle() const;
    virtual QString symLinkTarget() const;
    virtual QString owner() const;
    virtual uint ownerId() const;
    virtual QString group() const;
    virtual uint groupId() const;
    virtual bool permission(QFile::Permissions permissions) const;
    virtual QFile::Permissions permissions() const;
    virtual qint64 size() const;
    virtual QDateTime created() const;
    virtual QDateTime birthTime() const;
    virtual QDateTime metadataChangeTime() const;
    virtual QDateTime lastModified() const;
    virtual QDateTime lastRead() const;
    virtual QDateTime fileTime(QFile::FileTime time) const;
    virtual int countChildFile() const;
    virtual QString sizeFormat() const;
    virtual QIcon fileIcon() const;
    virtual QMimeType fileMimeType() const;
    virtual QVariantHash extraProperties() const;
    virtual Type fileType() const;

protected:
    explicit AbstractFileInfo(const QUrl &url, AbstractFileInfoPrivate *d);
    QScopedPointer<AbstractFileInfoPrivate> dptr;
};
DFMBASE_END_NAMESPACE

typedef QSharedPointer<DFMBASE_NAMESPACE::AbstractFileInfo> AbstractFileInfoPointer;
Q_DECLARE_METATYPE(AbstractFileInfoPointer)

#endif   // ABSTRACTFILEINFO_H
