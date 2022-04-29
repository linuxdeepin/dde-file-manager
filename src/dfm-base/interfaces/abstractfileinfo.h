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

// todo lanxs deal private
#define private public
#include <QSharedPointer>
#undef private

#include "dfm-base/base/urlroute.h"
#include "dfm-base/dfm_base_global.h"
#include "dfm-base/mimetype/mimedatabase.h"

#include <dfm-io/core/dfileinfo.h>

#include <QSharedData>
#include <QFile>
#include <QMimeType>
#include <QVariant>

class QDir;
class QDateTime;

// todo zhangyu deal define
#    define COMPARE_FUN_DEFINE(Value, Name, Type)                                                                           \
        bool compareFileListBy##Name(const QSharedPointer<DFMBASE_NAMESPACE::AbstractFileInfo> &info1,                      \
                                     const QSharedPointer<DFMBASE_NAMESPACE::AbstractFileInfo> &info2, Qt::SortOrder order) \
        {                                                                                                                   \
            bool isDir1 = info1->isDir();                                                                                   \
            bool isDir2 = info2->isDir();                                                                                   \
                                                                                                                            \
            bool isFile1 = info1->isFile();                                                                                 \
            bool isFile2 = info2->isFile();                                                                                 \
                                                                                                                            \
            if (!static_cast<const Type *>(info1.data()))                                                                   \
                return false;                                                                                               \
            if (!static_cast<const Type *>(info2.data()))                                                                   \
                return false;                                                                                               \
            auto value1 = static_cast<const Type *>(info1.data())->Value();                                                 \
            auto value2 = static_cast<const Type *>(info2.data())->Value();                                                 \
                                                                                                                            \
            if (isDir1) {                                                                                                   \
                if (!isDir2) return true;                                                                                   \
            } else {                                                                                                        \
                if (isDir2) return false;                                                                                   \
            }                                                                                                               \
                                                                                                                            \
            if ((isDir1 && isDir2 && (value1 == value2)) || (isFile1 && isFile2 && (value1 == value2))) {                   \
                return compareByString(info1->fileName(), info2->fileName());                                               \
            }                                                                                                               \
                                                                                                                            \
            bool isStrType = typeid(value1) == typeid(QString);                                                             \
            if (isStrType)                                                                                                  \
                return compareByString(value1, value2, order);                                                              \
                                                                                                                            \
            return ((order == Qt::DescendingOrder) ^ (value1 < value2)) == 0x01;                                            \
        }

namespace FileSortFunction {
bool compareByString(const QString &str1, const QString &str2, Qt::SortOrder order = Qt::AscendingOrder);
template<typename T>
bool compareByString(T, T, Qt::SortOrder order = Qt::AscendingOrder)
{
    Q_UNUSED(order)

    return false;
}
}

DFMBASE_BEGIN_NAMESPACE
class AbstractFileInfo;
DFMBASE_END_NAMESPACE

typedef QSharedPointer<DFMBASE_NAMESPACE::AbstractFileInfo> AbstractFileInfoPointer;

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
    enum FileType {
        kDirectory,   // 目录
        kCharDevice,   // 字符设备
        kBlockDevice,   // 块设备
        kFIFOFile,   // FIFO文件
        kSocketFile,   // socket文件
        kRegularFile,   // Regular文件

        kDocuments,   // 文档
        kImages,   // 镜像文件
        kVideos,   // 视频文件
        kAudios,   // 音乐文件
        kArchives,   // 归档文件
        kDesktopApplication,   // 应用
        kExecutable,   // 可执行
        kBackups,   // 回退
        kUnknown,
        kCustomType = 0x100
    };
    enum SortKey {
        kSortByFileName,
        kSortByModified,
        kSortByFileSize,
        kSortByFileCreated,
        kSortByFileLastRead,
        kSortByFileMimeType,
    };

public:
    explicit AbstractFileInfo() = delete;
    virtual ~AbstractFileInfo();
    virtual AbstractFileInfo &operator=(const AbstractFileInfo &fileinfo);
    virtual bool operator==(const AbstractFileInfo &fileinfo) const;
    virtual bool operator!=(const AbstractFileInfo &fileinfo) const;

    virtual void setFile(const QUrl &url);
    virtual bool exists() const;
    virtual void refresh();
    virtual void refresh(DFMIO::DFileInfo::AttributeID id, const QVariant &value = QVariant());
    virtual QString filePath() const;
    virtual QString absoluteFilePath() const;
    virtual QString fileName() const;
    virtual QString baseName() const;
    virtual QString completeBaseName() const;
    virtual QString fileNameOfRename() const;
    virtual QString baseNameOfRename() const;
    virtual QString suffixOfRename() const;
    virtual QString suffix() const;
    virtual QString completeSuffix();
    virtual QString path() const;
    virtual QString absolutePath() const;
    virtual QString canonicalPath() const;
    virtual QDir dir() const;
    virtual QDir absoluteDir() const;
    virtual QUrl url() const;
    virtual bool canRename() const;
    virtual bool isReadable() const;
    virtual bool isWritable() const;
    virtual bool isExecutable() const;
    virtual bool isHidden() const;
    virtual bool isFile() const;
    virtual bool isDir() const;
    virtual bool isSymLink() const;
    virtual bool isRoot() const;
    virtual bool isBundle() const;
    virtual bool isShared() const;
    virtual quint64 inode() const;
    virtual QString symLinkTarget() const;
    virtual QString owner() const;
    virtual uint ownerId() const;
    virtual QString group() const;
    virtual uint groupId() const;
    virtual bool permission(QFile::Permissions permissions) const;
    virtual QFile::Permissions permissions() const;
    virtual qint64 size() const;
    virtual QString sizeFormat() const;
    virtual QString fileDisplayName() const;
    virtual QString fileDisplayPinyinName() const;
    virtual QString sizeDisplayName() const;
    virtual QDateTime created() const;
    virtual QDateTime birthTime() const;
    virtual QDateTime metadataChangeTime() const;
    virtual QDateTime lastModified() const;
    virtual QDateTime lastRead() const;
    virtual QDateTime fileTime(QFile::FileTime time) const;
    virtual int countChildFile() const;
    virtual QIcon fileIcon() const;
    virtual QList<QIcon> additionalIcon() const;
    virtual QString iconName() const;
    virtual QString genericIconName() const;
    virtual QMimeType fileMimeType() const;
    virtual QVariantHash extraProperties() const;
    virtual FileType fileType() const;

    typedef std::function<bool(const QSharedPointer<DFMBASE_NAMESPACE::AbstractFileInfo> &,
                               const QSharedPointer<DFMBASE_NAMESPACE::AbstractFileInfo> &,
                               Qt::SortOrder)>
            CompareFunction;
    virtual CompareFunction compareFunByKey(const SortKey &sortKey) const;
    virtual QUrl getUrlByChildFileName(const QString &fileName) const;
    virtual QUrl getUrlByNewFileName(const QString &fileName) const;
    virtual QString mimeTypeDisplayName() const;
    virtual QString fileTypeDisplayName() const;
    virtual bool canRedirectionFileUrl() const;
    virtual QUrl redirectedFileUrl() const;
    virtual bool canMoveOrCopy() const;
    virtual bool canDrop() const;
    virtual bool canTag() const;
    virtual QUrl parentUrl() const;
    virtual Qt::DropActions supportedDragActions() const;
    virtual Qt::DropActions supportedDropActions() const;
    virtual bool canDragCompress() const;
    virtual bool isDragCompressFileFormat() const;
    virtual QString mimeTypeName() const;

    // property for view
    virtual QString emptyDirectoryTip() const;
    virtual QString loadingTip() const;

    // media info
    virtual void mediaInfoAttributes(DFMIO::DFileInfo::MediaType type,
                                     QList<DFMIO::DFileInfo::AttributeExtendID> ids,
                                     DFMIO::DFileInfo::AttributeExtendFuncCallback callback = nullptr) const;

    virtual bool notifyAttributeChanged();

protected:
    explicit AbstractFileInfo(const QUrl &url, AbstractFileInfoPrivate *d);
    void setProxy(const AbstractFileInfoPointer &proxy);
    QScopedPointer<AbstractFileInfoPrivate> dptr;
};
DFMBASE_END_NAMESPACE

typedef std::function<const AbstractFileInfoPointer(int)> getFileInfoFun;
Q_DECLARE_METATYPE(AbstractFileInfoPointer)

#endif   // ABSTRACTFILEINFO_H
