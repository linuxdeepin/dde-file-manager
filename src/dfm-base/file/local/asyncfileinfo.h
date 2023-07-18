// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ASYNCFILEINFO_H
#define ASYNCFILEINFO_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QIcon>
#include <QPointF>

namespace dfmbase {

class AsyncFileInfoPrivate;
class AsyncFileInfo : public FileInfo
{
    QSharedPointer<AsyncFileInfoPrivate> d { nullptr };

public:
    enum class AsyncAttributeID : uint16_t {

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
        kStandardFileType = 22,   // FileInfo::FileType

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

        kAttributeIDMax = 999,
    };

public:
    enum FlagIcon {
        kWritable,   // 是否可写
        kSymLink,   // 是否是链接文件
        kReadable,   // 是否可读
    };
    Q_ENUMS(FlagIcon)

    explicit AsyncFileInfo(const QUrl &url);
    AsyncFileInfo(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo);
    virtual ~AsyncFileInfo() override;
    virtual bool exists() const override;
    virtual void refresh() override;
    virtual void cacheAttribute(DFMIO::DFileInfo::AttributeID id, const QVariant &value = QVariant()) override;
    virtual QString nameOf(const FileNameInfoType type) const override;
    virtual QString pathOf(const FilePathInfoType type) const override;
    virtual QUrl urlOf(const FileUrlInfoType type) const override;
    virtual bool isAttributes(const FileIsType type) const override;
    virtual bool canAttributes(const FileCanType type) const override;
    virtual QVariant extendAttributes(const FileExtendedInfoType type) const override;
    virtual bool permission(QFile::Permissions permissions) const override;
    virtual QFile::Permissions permissions() const override;
    virtual qint64 size() const override;
    virtual QVariant timeOf(const FileTimeType type) const override;
    virtual QVariantHash extraProperties() const override;
    virtual QIcon fileIcon() override;
    virtual FileInfo::FileType fileType() const override;
    virtual int countChildFile() const override;
    virtual int countChildFileAsync() const override;
    virtual QString displayOf(const DisplayInfoType type) const override;
    virtual QMimeType fileMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) override;
    virtual QMimeType fileMimeTypeAsync(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) override;
    virtual QString viewOfTip(const ViewType type) const override;
    // emblems
    virtual QVariant customAttribute(const char *key, const DFMIO::DFileInfo::DFileAttributeType type) override;
    QVariant customData(int role) const override;
    // media info
    virtual QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> mediaInfoAttributes(DFMIO::DFileInfo::MediaType type, QList<DFMIO::DFileInfo::AttributeExtendID> ids) const override;
    // cache attribute
    virtual void setExtendedAttributes(const FileExtendedInfoType &key, const QVariant &value) override;
    QMultiMap<QUrl, QString> notifyUrls() const;
    void setNotifyUrl(const QUrl &url, const QString &infoPtr);
    void removeNotifyUrl(const QUrl &url, const QString &infoPtr);
    void cacheAsyncAttributes();
    bool asyncQueryDfmFileInfo(int ioPriority = 0, initQuerierAsyncCallback func = nullptr, void *userData = nullptr);
};
}
typedef QSharedPointer<DFMBASE_NAMESPACE::AsyncFileInfo> DFMAsyncFileInfoPointer;
Q_DECLARE_METATYPE(DFMAsyncFileInfoPointer)

#endif   // ASYNCFILEINFO_H
