// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINFO_H
#define FILEINFO_H

#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/abstractfileinfo.h>

#include <dfm-io/dfileinfo.h>

#include <QSharedData>
#include <QFile>
#include <QMimeType>
#include <QVariant>
#include <QMetaType>

class QDir;
class QDateTime;

namespace dfmbase {
class FileInfo;
}

typedef QSharedPointer<DFMBASE_NAMESPACE::FileInfo> FileInfoPointer;

namespace dfmbase {
class FileInfoPrivate;
class FileInfo : public AbstractFileInfo, public QEnableSharedFromThis<FileInfo>
{
public:
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
        kBaseNameOfRename = 6,   // 没有特殊情况返回: archive.tar
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
        kAbsoluteFilePath = 1,   // 文件绝对路径: /temp/archive.tar.gz 、/temp/var
        kPath = 2,   // 路径:/temp 、/temp
        kAbsolutePath = 3,   // 路径:/temp 、/temp
        kCanonicalPath = 4,   // 返回没有符号链接或者冗余“.”或“..”元素的绝对路径:/temp/archive.tar.gz 、/temp/var
        kSymLinkTarget = 5,   // 链接文件的目标的绝对路径
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
        kGroup = 1,   // 文件的所属组的string
        kSizeFormat = 2,   // 文件大小的固定格式
        kInode = 3,   // 文件的inode
        kOwnerId = 4,   // 文件的拥有者的id
        kGroupId = 5,   // 文件的组id
        kFileIsHid = 6,   // 是否是隐藏文件
        kFileLocalDevice = 7,   // 文件是本地文件
        kFileCdRomDevice = 8,   // 文件是光驱
        kFileThumbnail = 9,   // 文件缩略图
        kCustomerStartExtended = 50,   // 其他用户使用
        kFileNeedUpdate = 51,   // 文件信息需要在显示更新
        kFileNeedTransInfo = 52,   // 文件信息需要转换为desktopfileinfo
        kFileHighlightContent = 53,   // 文件需要高亮内容
        kExpectedSize = 54,   // 文件预期大小（用于正在拷贝/移动的文件）
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
        kCustomerFileIs = 50,   // 其他用户使用
        kUnknowFileIsInfo = 255,
    };
    /*!
     * \enum FileIsType
     * \brief 文件
     */
    enum class FileCanType : uint8_t {
        kCanDelete = 0,   // 可以删除
        kCanTrash = 1,   // 可以移动到回收站
        kCanRename = 2,   // 可以重命名
        kCanRedirectionFileUrl = 3,   // 可以重定向
        kCanMoveOrCopy = 4,   // 可以拷贝(若需要可以移动使用,则使用kCanRename去判断)
        kCanDrop = 5,   // 可以Drop
        kCanDrag = 6,   // 可以drag
        kCanDragCompress = 7,   // 可以压缩
        kCanFetch = 8,   // 可以遍历
        kCanHidden = 9,   // 可以隐藏
        kCustomerFileCan = 50,   // 其他用户使用
        kUnknowFileCanInfo = 255,
    };

    // TODO(liyigang): This is a bad design,
    // fileinfo cannot return mime tpye!(image, video ...)
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

    enum class FileInfoAttributeID : uint16_t {

        kStandardType = 0,   // uint32
        kStandardIsHidden = 1,   // boolean
        kStandardIsBackup = 2,   // boolean
        kStandardIsSymlink = 3,   // boolean
        kStandardIsVirtual = 4,   // boolean
        kStandardIsVolatile = 5,   // boolean
        kStandardName = 6,   // byte string
        kStandardDisplayName = 7,   // string
        kStandardEditName = 8,   // string
        kStandardCopyName = 9,   // string
        kStandardIcon = 10,   // QList<QString>
        kStandardSymbolicIcon = 11,   // QList<QString>
        kStandardContentType = 12,   // string
        kStandardFastContentType = 13,   // string
        kStandardSize = 14,   // uint64
        kStandardAllocatedSize = 15,   // uint64
        kStandardSymlinkTarget = 16,   // byte string
        kStandardTargetUri = 17,   // string
        kStandardSortOrder = 18,   // int32
        kStandardDescription = 19,   // string
        kStandardFileExists = 20,   // bool
        kStandardIsLocalDevice = 21,   // bool
        kStandardIsCdRomDevice = 22,   // bool
        kStandardFileType = 23,   // FileInfo::FileType
        kOriginalUri = 24,   // original uri

        kEtagValue = 40,   // string

        kIdFile = 60,   // string
        kIdFilesystem = 61,   // string

        kAccessCanRead = 100,   // boolean
        kAccessCanWrite = 101,   // boolean
        kAccessCanExecute = 102,   // boolean
        kAccessCanDelete = 103,   // boolean
        kAccessCanTrash = 104,   // boolean
        kAccessCanRename = 105,   // boolean
        kAccessPermissions = 106,   // DFile::Permissions

        kMountableCanMount = 130,   // boolean
        kMountableCanUnmount = 131,   // boolean
        kMountableCanEject = 132,   // boolean
        kMountableUnixDevice = 133,   // uint32
        kMountableUnixDeviceFile = 134,   // string
        kMountableHalUdi = 135,   // string
        kMountableCanPoll = 136,   // boolean
        kMountableIsMediaCheckAutomatic = 137,   // boolean
        kMountableCanStart = 138,   // boolean
        kMountableCanStartDegraded = 139,   // boolean
        kMountableCanStop = 140,   // boolean
        kMountableStartStopType = 141,   // uint32

        kTimeModified = 200,   // uint64
        kTimeModifiedUsec = 201,   // uint32
        kTimeAccess = 202,   // uint64
        kTimeAccessUsec = 203,   // uint32
        kTimeChanged = 204,   // uint64
        kTimeChangedUsec = 205,   // uint32
        kTimeCreated = 206,   // uint64
        kTimeCreatedUsec = 207,   // uint32

        kUnixDevice = 330,   // uint32
        kUnixInode = 331,   // uint64
        kUnixMode = 332,   // uint32
        kUnixNlink = 333,   // uint32
        kUnixUID = 334,   // uint32
        kUnixGID = 335,   // uint32
        kUnixRdev = 336,   // uint32
        kUnixBlockSize = 337,   // uint32
        kUnixBlocks = 338,   // uint64
        kUnixIsMountPoint = 339,   // boolean

        kDosIsArchive = 360,   // boolean
        kDosIsSystem = 361,   // boolean

        kOwnerUser = 300,   // string
        kOwnerUserReal = 301,   // string
        kOwnerGroup = 302,   // string

        kThumbnailPath = 390,   // byte string
        kThumbnailFailed = 391,   // boolean
        kThumbnailIsValid = 392,   // boolean
        kThumbnailIcon = 393,   // QIcon

        kPreviewIcon = 420,   // object

        kFileSystemSize = 440,   // uint64
        kFileSystemFree = 441,   // uint64
        kFileSystemUsed = 442,   // uint64
        kFileSystemType = 443,   // string
        kFileSystemReadOnly = 444,   // boolean
        kFileSystemUsePreview = 445,   // uint32
        kFileSystemRemote = 446,   // boolean

        kGvfsBackend = 470,   // string

        kSelinuxContext = 490,   // string

        kTrashItemCount = 510,   // uint32
        kTrashDeletionDate = 511,   // string
        kTrashOrigPath = 512,   // byte string

        kRecentModified = 540,   // int64

        kCustomStart = 600,

        kStandardIsFile = 610,
        kStandardIsDir = 611,
        kStandardIsRoot = 612,
        kStandardSuffix = 613,
        kStandardCompleteSuffix = 614,
        kStandardFilePath = 615,
        kStandardParentPath = 616,
        kStandardBaseName = 617,
        kStandardFileName = 618,
        kStandardCompleteBaseName = 619,

        kFileMimeType = 620,
        kFileCount = 621,
        kFileMediaInfo = 621,

        kAttributeIDMax = 999,
    };

    using initQuerierAsyncCallback = std::function<void(bool, void *)>;

public:
    explicit FileInfo() = delete;
    virtual ~FileInfo();
    virtual FileInfo &operator=(const FileInfo &fileinfo);
    virtual bool operator==(const FileInfo &fileinfo) const;
    virtual bool operator!=(const FileInfo &fileinfo) const;

    virtual bool initQuerier();
    virtual void initQuerierAsync(int ioPriority = 0, initQuerierAsyncCallback func = nullptr, void *userData = nullptr);
    virtual bool exists() const;
    virtual void refresh();
    virtual void cacheAttribute(DFMIO::DFileInfo::AttributeID id, const QVariant &value = QVariant());
    virtual QString nameOf(const FileNameInfoType type) const;
    virtual QString pathOf(const FilePathInfoType type) const;
    virtual QString displayOf(const DisplayInfoType type) const;
    virtual QUrl urlOf(const FileUrlInfoType type) const;
    virtual QUrl getUrlByType(const FileUrlInfoType type, const QString &fileName) const;
    virtual bool isAttributes(const FileIsType type) const;
    virtual bool canAttributes(const FileCanType type) const;
    virtual QVariant extendAttributes(const FileExtendedInfoType type) const;
    virtual bool permission(QFile::Permissions permissions) const;
    virtual QFile::Permissions permissions() const;
    virtual int countChildFile() const;
    virtual int countChildFileAsync() const;
    virtual qint64 size() const;
    virtual QVariant timeOf(const FileTimeType type) const;
    virtual QIcon fileIcon();
    virtual QMimeType fileMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault);
    virtual QMimeType fileMimeTypeAsync(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault);
    virtual QVariantHash extraProperties() const;
    virtual QVariant customData(int role) const;
    virtual FileType fileType() const;
    virtual Qt::DropActions supportedOfAttributes(const SupportType type) const;
    virtual QString viewOfTip(const ViewType type) const;
    virtual QVariant customAttribute(const char *key, const DFMIO::DFileInfo::DFileAttributeType type);
    virtual QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> mediaInfoAttributes(DFMIO::DFileInfo::MediaType type, QList<DFMIO::DFileInfo::AttributeExtendID> ids) const;
    virtual void setExtendedAttributes(const FileExtendedInfoType &key, const QVariant &value);
    // 只是对相应的属性进行更新，不是清空，refresh是清空所有属性再去获取，默认是更新所有文件属性
    virtual void updateAttributes(const QList<FileInfoAttributeID> &types = {});

protected:
    explicit FileInfo(const QUrl &url);
    mutable QReadWriteLock extendOtherCacheLock;
    mutable QMap<FileInfo::FileExtendedInfoType, QVariant> extendOtherCache;
    QString pinyinName;

private:
    QSharedPointer<FileInfoPrivate> dptr;
};

using NameInfoType = FileInfo::FileNameInfoType;
using PathInfoType = FileInfo::FilePathInfoType;
using DisPlayInfoType = FileInfo::DisplayInfoType;
using UrlInfoType = FileInfo::FileUrlInfoType;
using ExtInfoType = FileInfo::FileExtendedInfoType;
using ViewInfoType = FileInfo::ViewType;
using SupportedType = FileInfo::SupportType;
using TimeInfoType = FileInfo::FileTimeType;
using OptInfoType = FileInfo::FileIsType;
using CanableInfoType = FileInfo::FileCanType;
}
Q_DECLARE_METATYPE(FileInfoPointer);
Q_DECLARE_METATYPE(QList<FileInfoPointer>);
Q_DECLARE_METATYPE(dfmbase::FileInfo::FileType);

#endif   // FILEINFO_H
