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
#include <QVariant>

class QDir;
class QDateTime;

namespace dfmbase {
class AbstractFileInfo;
}

typedef QSharedPointer<DFMBASE_NAMESPACE::AbstractFileInfo> AbstractFileInfoPointer;

namespace dfmbase {
class AbstractFileInfoPrivate;
class AbstractFileInfo : public QObject, public QSharedData, public QEnableSharedFromThis<AbstractFileInfo>
{
    Q_OBJECT
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
    /*!
     * \enum FileNameInfoType 文件名称信息
     * \brief 文件的名称信息的key
     */
    enum class FileNameInfoType : uint8_t {
        // url = file://temp/archive.tar.gz
        kFileName = 0,   // 文件的全名: archive.tar.gz
        kBaseName = 1,   // 文件的基本名称: archive
        kCompleteBaseName = 2,   // 文件的完整名称: archive.tar
        kSuffix = 3,   // 文件的suffix: gz
        kCompleteSuffix = 4,   // 文件完整suffix: tar.gz
        kFileNameOfRename = 5,   // 没有特殊情况返回: archive.tar.gz
        kBaseNameOfRename = 6,   // 没有特殊情况返回: archive
        kSuffixOfRename = 7,   // 没有特殊情况返回: tar.gz
        kFileCopyName = 8,   // 文件的拷贝名称
        kIconName = 9,   // 文件的icom名称
        kGenericIconName = 10,   // 文件的
        kMimeTypeName = 11,   // 文件的mimetype名称
        kCustomerStartName = 50,   // 其他用户使用
        kUnknowNameInfo = 255,
    };
    /*!
     * \enum FilePathInfoType 文件路径信息
     * \brief 文件的名称信息的key
     */
    enum class FilePathInfoType {
        // 文件url = file://temp/archive.tar.gz
        // 目录url = file://temp/var/
        kFilePath = 0,   // 文件路径: /temp/archive.tar.gz 、/temp/var
        kAbsoluteFilePath = 2,   // 文件绝对路径: /temp/archive.tar.gz 、/temp/var
        kPath = 3,   // 路径:/temp 、/temp
        kAbsolutePath = 4,   // 路径:/temp 、/temp
        kCanonicalPath = 5,   // 返回没有符号链接或者冗余“.”或“..”元素的绝对路径:/temp/archive.tar.gz 、/temp/var
        kSymLinkTarget = 6,   // 链接文件的目标的绝对路径
        kCustomerStartPath = 50,   // 其他用户使用
        kUnknowPathInfo = 255,
    };
    /*!
     * \enum FileUrlInfoType 文件url信息
     * \brief 文件的QUrl信息的key
     */
    enum DisplayInfoType {
        // 文件url = file://temp/archive.tar.gz
        kFileDisplayName = 0,   // 文件的显示名称: archive (dde-file-manager.desktop的显示名称是: 文件管理器)
        kFileDisplayPinyinName = 1,   // 文件的显示拼音名称
        kSizeDisplayName = 2,   // 文件的大小显示名称
        kFileDisplayPath = 3,   // 文件路径显示名称
        kMimeTypeDisplayName = 4,   // 文件的mimeType显示名称
        kFileTypeDisplayName = 5,   // 文件的文件类型显示名称
        kCustomerStartDisplay = 50,   // 其他用户使用
        kUnknowDisplayInfo = 255,
    };
    /*!
     * \enum FileUrlInfoType 文件url信息
     * \brief 文件的QUrl信息的key
     */
    enum class FileUrlInfoType : uint8_t {
        // 文件url = file://temp/archive.tar.gz
        kUrl = 0,   // 文件的url: file://temp/archive.tar.gz
        kGetUrlByChildFileName = 1,   // 根据子文件名称获取url
        kGetUrlByNewFileName = 2,   // 获取文件的新名称的url
        kRedirectedFileUrl = 3,   // 文件的重定向url
        kOriginalUrl = 4,   // 文件的原始url
        kParentUrl = 5,   // 文件父url
        kCustomerStartUrl = 50,   // 其他用户使用
        kUnknowUrlInfo = 255,
    };
    /*!
     * \enum FileExtendedInfoType 文件扩展信息
     * \brief 文件的扩展信息
     */
    enum class FileExtendedInfoType : uint8_t {
        // 文件url = file://temp/archive.tar.gz
        kOwner = 0,   // 文件的所有者string
        kGroup = 2,   // 文件的所属组的string
        kSizeFormat = 3,   // 文件大小的固定格式
        kInode = 4,   // 文件的inode
        kOwnerId = 5,   // 文件的拥有者的id
        kGroupId = 6,   // 文件的组id
        kFileIsHid = 7,   // 是否是隐藏文件
        kFileLocalDevice = 8,   // 文件是本地文件
        kFileCdRomDevice = 9,   // 文件是光驱
        kCustomerStartExtended = 50,   // 其他用户使用
        kUnknowExtendedInfo = 255,
    };
    /*!
     * \enum ViewType 文件在试图中显示的tip文字
     * \brief 文件的提示信息
     */
    enum class ViewType : uint8_t {
        kEmptyDir = 0,   // 空目录view的提示信息
        kLoading = 1,   // 加载中view的提示信息
        kCustomerStartView = 50,   // 其他用户使用
        kUnknowViewTInfo = 255,
    };
    /*!
     * \enum SupportType 文件支持哪些操作
     * \brief 文件的drop和drag支持属性
     */
    enum class SupportType : uint8_t {
        kDrag = 0,   // Drag type
        kDrop = 1,   // Drop type
        kCustomerSupport = 50,   // 其他用户使用
        kUnknowSupportTInfo = 255,
    };
    /*!
     * \enum FileTimeType 文件时间信息
     * \brief 文件的时间信息，Second的返回秒qint64, MSecond的返回毫秒
     */
    enum class FileTimeType : uint8_t {
        kCreateTime = 0,   // 文件的创建时间
        kBirthTime = 1,   // 文件出生时间
        kMetadataChangeTime = 2,   // 文件的metadata改变时间
        kLastModified = 3,   // 文件的最后修改时间
        kLastRead = 4,   // 文件的最后读取时间
        kDeletionTime = 5,   // 文件的删除时间
        kCreateTimeSecond = 6,   // 文件的创建时间s为单位
        kBirthTimeSecond = 7,   // 文件出生时间
        kMetadataChangeTimeSecond = 8,   // 文件的metadata改变时间
        kLastModifiedSecond = 9,   // 文件的最后修改时间
        kLastReadSecond = 10,   // 文件的最后读取时间
        kDeletionTimeSecond = 11,   // 文件的删除时间
        kCreateTimeMSecond = 12,   // 文件的创建时间s为单位
        kBirthTimeMSecond = 13,   // 文件出生时间
        kMetadataChangeTimeMSecond = 14,   // 文件的metadata改变时间
        kLastModifiedMSecond = 15,   // 文件的最后修改时间
        kLastReadMSecond = 16,   // 文件的最后读取时间
        kDeletionTimeMSecond = 17,   // 文件的删除时间
        kCustomerSupport = 50,   // 其他用户使用
        kUnknowSupportTInfo = 255,
    };
    /*!
     * \enum FileIsType
     * \brief 文件
     */
    enum class FileIsType : uint8_t {
        kIsReadable = 0,   // 文件是否可读
        kIsWritable = 1,   // 文件是否可写
        kIsExecutable = 2,   // 文件是否可执行
        kIsHidden = 3,   // 文件是否可隐藏
        kIsFile = 4,   // 是否是文件
        kIsDir = 5,   // 是否是目录
        kIsSymLink = 6,   // 是否是链接文件
        kIsRoot = 7,   // 是否是根文件
        kIsBundle = 8,   // 是否是二进制文件
        kIsDragCompressFileFormat = 9,   // 是否可以追加压缩
        kIsPrivate = 10,   // 是否是私有文件
        kCustomerFileIs = 50,   // 其他用户使用
        kUnknowFileIsInfo = 255,
    };
    /*!
     * \enum FileIsType
     * \brief 文件
     */
    enum class FileCanType : uint8_t {
        kCanDelete,   // 可以删除
        kCanTrash,   // 可以移动到回收站
        kCanRename,   // 可以重命名
        kCanRedirectionFileUrl,   // 可以重定向
        kCanMoveOrCopy,   // 可以移动或者拷贝
        kCanDrop,   // 可以Drop
        kCanDrag,   // 可以drag
        kCanDragCompress,   // 可以压缩
        kCanFetch,   // 可以遍历
        kCanHidden,   // 可以隐藏
        kCustomerFileCan = 50,   // 其他用户使用
        kUnknowFileCanInfo = 255,
    };
    enum class FileType : uint16_t {
        kDirectory = 0,   // 目录
        kCharDevice = 1,   // 字符设备
        kBlockDevice = 2,   // 块设备
        kFIFOFile = 3,   // FIFO文件
        kSocketFile = 4,   // socket文件
        kRegularFile = 5,   // Regular文件
        kDocuments = 6,   // 文档
        kImages = 7,   // 镜像文件
        kVideos = 8,   // 视频文件
        kAudios = 9,   // 音乐文件
        kArchives = 10,   // 归档文件
        kDesktopApplication = 11,   // 应用
        kExecutable = 12,   // 可执行
        kBackups = 13,   // 回退
        kUnknown = 14,
        kCustomType = 0x100,
    };

    using initQuerierAsyncCallback = std::function<void(bool, void *)>;

public:
    explicit AbstractFileInfo() = delete;
    virtual ~AbstractFileInfo();
    virtual AbstractFileInfo &operator=(const AbstractFileInfo &fileinfo);
    virtual bool operator==(const AbstractFileInfo &fileinfo) const;
    virtual bool operator!=(const AbstractFileInfo &fileinfo) const;

    virtual bool initQuerier();
    virtual void initQuerierAsync(int ioPriority = 0, initQuerierAsyncCallback func = nullptr, void *userData = nullptr);
    virtual bool exists() const;
    virtual void refresh();
    virtual void refresh(DFMIO::DFileInfo::AttributeID id, const QVariant &value = QVariant());
    virtual QString nameInfo(const FileNameInfoType type = FileNameInfoType::kFileName) const;

    virtual QString filePath() const;
    virtual QString absoluteFilePath() const;
    virtual QString path() const;
    virtual QString absolutePath() const;
    virtual QString canonicalPath() const;
    virtual QString symLinkTarget() const;
    virtual QString owner() const;
    virtual QString group() const;
    virtual QString sizeFormat() const;

    virtual QString displayInfo(const DisplayInfoType type = DisplayInfoType::kFileDisplayName) const;
    virtual QUrl url() const;
    virtual QUrl getUrlByChildFileName(const QString &fileName) const;
    virtual QUrl getUrlByNewFileName(const QString &fileName) const;
    virtual QUrl redirectedFileUrl() const;
    virtual QUrl originalUrl() const;
    virtual QUrl parentUrl() const;
    virtual bool isAncestorsUrl(const QUrl &url, QList<QUrl> *ancestors = nullptr) const;
    virtual bool isReadable() const;
    virtual bool isWritable() const;
    virtual bool isExecutable() const;
    virtual bool isHidden() const;
    virtual bool isFile() const;
    virtual bool isDir() const;
    virtual bool isSymLink() const;
    virtual bool isRoot() const;
    virtual bool isBundle() const;
    virtual bool isDragCompressFileFormat() const;
    virtual bool isPrivate() const;
    virtual bool canDelete() const;
    virtual bool canTrash() const;
    virtual bool canRename() const;
    virtual bool canRedirectionFileUrl() const;
    virtual bool canMoveOrCopy() const;
    virtual bool canDrop();
    virtual bool canDrag();
    virtual bool canDragCompress() const;
    virtual bool canFetch() const;
    virtual bool canHidden() const;
    virtual bool permission(QFile::Permissions permissions) const;
    virtual QFile::Permissions permissions() const;
    virtual int countChildFile() const;
    virtual quint64 inode() const;
    virtual uint ownerId() const;
    virtual uint groupId() const;
    virtual qint64 size() const;

    virtual QVariant timeInfo(const FileTimeType type = FileTimeType::kCreateTime) const;

    virtual QIcon fileIcon();
    virtual QMimeType fileMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault);
    virtual QVariantHash extraProperties() const;
    virtual QVariant customData(int role) const;
    virtual FileType fileType() const;
    virtual Qt::DropActions supportedAttributes(const SupportType type = SupportType::kDrag) const;

    // property for view
    virtual QString viewTip(const ViewType type = ViewType::kEmptyDir) const;

    // emblems
    virtual QVariant customAttribute(const char *key, const DFMIO::DFileInfo::DFileAttributeType type);

    // media info
    virtual void mediaInfoAttributes(DFMIO::DFileInfo::MediaType type, QList<DFMIO::DFileInfo::AttributeExtendID> ids) const;

    virtual bool notifyAttributeChanged();

    virtual void setExtendedAttributes(const FileExtendedInfoType &key, const QVariant &value);

Q_SIGNALS:
    void mediaDataFinished(bool, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant>);

protected:
    explicit AbstractFileInfo(const QUrl &url);
    void setProxy(const AbstractFileInfoPointer &proxy);
    QSharedPointer<AbstractFileInfoPrivate> dptr;

private:
    QString fileName() const;
    QString baseName() const;

private:
    bool hasProxy();
    friend class InfoCache;
};
}

typedef std::function<const AbstractFileInfoPointer(int)> getFileInfoFun;
Q_DECLARE_METATYPE(AbstractFileInfoPointer)

#endif   // ABSTRACTFILEINFO_H
