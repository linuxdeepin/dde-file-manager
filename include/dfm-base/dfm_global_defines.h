// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_GLOBAL_DEFINES_H
#define DFM_GLOBAL_DEFINES_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>

#define DFMGLOBAL_NAMESPACE DFMBASE_NAMESPACE::Global

#define DFMGLOBAL_BEGIN_NAMESPACE namespace DFMGLOBAL_NAMESPACE {
#define DFMGLOBAL_END_NAMESPACE }
#define DFMGLOBAL_USE_NAMESPACE using namespace DFMGLOBAL_NAMESPACE;

namespace dfmbase {
namespace Global {
inline constexpr int kOpenNewWindowMaxCount = 50;

enum class ViewMode {
    kNoneMode = 0x00,
    kIconMode = 0x01,
    kListMode = 0x02,
    kExtendMode = 0x04,
    kTreeMode = 0x08,
    kAllViewMode = kIconMode | kListMode | kExtendMode
};

enum class DirectoryLoadStrategy : uint8_t {
    kCreateNew,   // 默认策略：每次切换目录时立即清空视图
    kPreserve   // 保留策略：保留现有视图内容，直到新目录数据加载完成后再更新视图
};

namespace ViewCustomKeys {
inline constexpr char kSupportIconMode[] { "Custom_Key_SupportIconMode" };
inline constexpr char kSupportListMode[] { "Custom_Key_SupportListMode" };
inline constexpr char kSupportTreeMode[] { "Custom_Key_SupportTreeMode" };
inline constexpr char kDefaultViewMode[] { "Custom_Key_DefaultViewMode" };
inline constexpr char kDefaultListHeight[] { "Custom_Key_DefaultListHeight" };
inline constexpr char kAllowChangeListHeight[] { "Custom_Key_AllowChangeListHeight" };
inline constexpr char kViewModeUrlCallback[] { "Custom_Key_ViewModeUrlCallback" };
}   // namespace ViewCustomKeys

enum class TransparentStatus : uint8_t {
    kDefault,
    kTransparent,
    kUntransparent
};

enum class FileNotifyType : uint8_t {
    kFileNone,
    kFileChanged,
    kFileAdded,
    kFileDeleted
};

enum ThumbnailSize {
    kSmall = 64,
    kNormal = 128,
    kLarge = 256,
};

enum ItemRoles {
    kItemDisplayRole = Qt::DisplayRole,
    kItemIconRole = Qt::DecorationRole,
    kItemEditRole = Qt::EditRole,
    kItemToolTipRole = Qt::ToolTipRole,
    kItemSizeHintRole = Qt::SizeHintRole,
    kItemBackgroundRole = Qt::BackgroundRole,
    kItemForegroundRole = Qt::ForegroundRole,
    kItemCheckStateRole = Qt::CheckStateRole,
    kItemInitialSortOrderRole = Qt::InitialSortOrderRole,
    kItemFontRole = Qt::FontRole,
    kItemTextAlignmentRole = Qt::TextAlignmentRole,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 13, 0))
    kItemColorRole = Qt::TextColorRole,
#else
    kItemColorRole = Qt::ForegroundRole,
#endif

    kItemFilePathRole = Qt::UserRole + 1,
    kItemNameRole = Qt::UserRole + 2,
    kItemFileSizeRole = Qt::UserRole + 3,
    kItemFileMimeTypeRole = Qt::UserRole + 4,
    kItemUrlRole = Qt::UserRole + 5,
    kItemFileLastModifiedRole = Qt::UserRole + 6,
    kItemFileLastReadRole = Qt::UserRole + 7,
    kItemFileCreatedRole = Qt::UserRole + 8,
    kItemFileDisplayNameRole = Qt::UserRole + 9,
    kItemFilePinyinNameRole = Qt::UserRole + 10,
    kItemExtraProperties = Qt::UserRole + 11,
    kItemFileBaseNameRole = Qt::UserRole + 12,
    kItemFileSuffixRole = Qt::UserRole + 13,
    kItemFileNameOfRenameRole = Qt::UserRole + 14,
    kItemFileBaseNameOfRenameRole = Qt::UserRole + 15,
    kItemFileSuffixOfRenameRole = Qt::UserRole + 16,
    kItemFileIconModelToolTipRole = Qt::UserRole + 19,
    kItemFileOriginalPath = Qt::UserRole + 23,
    kItemFileDeletionDate = Qt::UserRole + 24,
    kItemFileRefreshIcon = Qt::UserRole + 25,
    kItemFileIsAvailableRole = Qt::UserRole + 26,   // the item gray display and can not select
    kItemFileIsDirRole = Qt::UserRole + 27,
    kItemFileIsWritableRole = Qt::UserRole + 28,
    kItemFileCanRenameRole = Qt::UserRole + 29,
    kItemFileCanDropRole = Qt::UserRole + 30,
    kItemFileCanDragRole = Qt::UserRole + 31,
    kItemFileSizeIntRole = Qt::UserRole + 32,
    kItemCreateFileInfoRole = Qt::UserRole + 33,
    kItemTreeViewDepthRole = Qt::UserRole + 34,
    kItemTreeViewExpandedRole = Qt::UserRole + 35,
    kItemTreeViewCanExpandRole = Qt::UserRole + 36,   // item can expand
    kItemUpdateAndTransFileInfoRole = Qt::UserRole + 37,
    kItemFileContentPreviewRole = Qt::UserRole + 38,   // item file content
    kItemIsGroupHeaderType = Qt::UserRole + 39,
    kItemGroupHeaderKey = Qt::UserRole + 40,
    kItemGroupDisplayIndex = Qt::UserRole + 41,
    kItemGroupExpandedRole = Qt::UserRole + 42,
    kItemUnknowRole = Qt::UserRole + 999
};

enum CreateFileType : uint8_t {
    kCreateFileTypeUnknow = 0,
    kCreateFileTypeFolder,
    kCreateFileTypeText,
    kCreateFileTypeExcel,
    kCreateFileTypeWord,
    kCreateFileTypePowerpoint,
    kCreateFileTypeDefault = 0xff,
};

enum CreateFileInfoType : uint8_t {
    kCreateFileInfoAuto = 0,   // auto can cache file info, virtual schema will synchronize create file info
    kCreateFileInfoSync = 1,
    kCreateFileInfoAsync = 2,
    kCreateFileInfoSyncAndCache = 3,   // create file info Synchronize and cache file info
    kCreateFileInfoAsyncAndCache = 4,   // create file info Asynchronous and cache file info
    kCreateFileInfoAutoNoCache = 5,   // can not cache file info, virtual schema will synchronize create file info
};

namespace Mime {
inline constexpr char kTypeAppDesktop[] { "application/x-desktop" };
inline constexpr char kTypeCdImage[] { "application/x-cd-image" };
inline constexpr char kTypeISO9660Image[] { "application/x-iso9660-image" };
inline constexpr char kTypeAppXml[] { "application/xml" };
inline constexpr char kTypeAppPdf[] { "application/pdf" };
inline constexpr char kTypeAppPptx[] { "application/vnd.openxmlformats-officedocument.presentationml.presentation" };
inline constexpr char kTypeAppMxf[] { "application/mxf" };
inline constexpr char kTypeAppVMAsf[] { "application/vnd.ms-asf" };
inline constexpr char kTypeAppCRRMedia[] { "application/cnd.rn-realmedia" };
inline constexpr char kTypeAppVRRMedia[] { "application/vnd.rn-realmedia" };
inline constexpr char kTypeAppAppimage[] { "application/vnd.appimage" };
inline constexpr char kTypeTextHtml[] { "text/html" };
inline constexpr char kTypeAppXhtmlXml[] { "application/xhtml+xml" };
inline constexpr char kTypeTextXPython[] { "text/x-python" };
inline constexpr char kTypeTextPlain[] { "text/plain" };
inline constexpr char kTypeAppXOleStorage[] { "application/x-ole-storage" };
inline constexpr char kTypeAppZip[] { "application/zip" };
inline constexpr char kTypeArchiveRAR[] { "application/vnd.rar" };
inline constexpr char kTypeAppRAR[] { "application/rar" };
inline constexpr char kTypeAppXDesktop[] { "application/x-desktop" };
// image
inline constexpr char kTypeImageIef[] { "image/ief" };
inline constexpr char kTypeImageTiff[] { "image/tiff" };
inline constexpr char kTypeImageXTMultipage[] { "image/x-tiff-multipage" };
inline constexpr char kTypeImageVDMultipage[] { "image/vnd.djvu+multipage" };
inline constexpr char kTypeImageXADng[] { "image/x-adobe-dng" };
inline constexpr char kTypeImageJpeg[] { "image/jpeg" };
inline constexpr char kTypeImagePng[] { "image/png" };
inline constexpr char kTypeImagePipeg[] { "image/pipeg" };
inline constexpr char kTypeImageVDjvu[] { "image/vnd.djvu" };
inline constexpr char kTypeImageSvgXml[] { "image/svg+xml" };
inline constexpr char kTypeAudioFlac[] { "audio/flac" };

// custom
inline constexpr char kDataUserIDKey[] { "userid_for_drag" };
inline constexpr char kDFMMimeDataKey[] { "dfm_mimedata_for_drag" };
inline constexpr char kDFMTreeUrlsKey[] { "dfm_tree_urls_for_drag" };
inline constexpr char kDFMAppTypeKey[] { "dfm_app_type_for_drag" };
}   // namespace Mime

namespace Scheme {
inline constexpr char kSmb[] { "smb" };
inline constexpr char kSmbShare[] { "smb-share" };
inline constexpr char kFtp[] { "ftp" };
inline constexpr char kSFtp[] { "sftp" };
inline constexpr char kGPhoto[] { "gphoto" };
inline constexpr char kGPhoto2[] { "gphoto2" };
inline constexpr char kFile[] { "file" };
inline constexpr char kAsyncFile[] { "asyncfile" };
inline constexpr char kDesktop[] { "desktop" };
inline constexpr char kMtp[] { "mtp" };
inline constexpr char kAfc[] { "afc" };
inline constexpr char kDav[] { "dav" };
inline constexpr char kDavs[] { "davs" };
inline constexpr char kNfs[] { "nfs" };
inline constexpr char kEntry[] { "entry" };
inline constexpr char kBurn[] { "burn" };
inline constexpr char kComputer[] { "computer" };
inline constexpr char kTrash[] { "trash" };
inline constexpr char kRecent[] { "recent" };
inline constexpr char kUserShare[] { "usershare" };
inline constexpr char kNetwork[] { "network" };
}   // namespace Scheme

namespace Regex {
inline constexpr char kGvfsRoot[] { R"(^/run/user/.*/gvfs/|^/root/.gvfs/)" };
}   // namespace GlobalRegex

namespace DataBase {
inline constexpr char kDfmDBName[] { "dfmruntime.db" };
}   // namespace DataBase

namespace DataPersistence {
// groups
inline constexpr char kReportGroup[] { "Report" };
// keys
inline constexpr char kDesktopStartUpReportKey[] { "DesktopStartUp" };
inline constexpr char kDesktopLaunchTime[] { "DesktopLaunchTime" };
inline constexpr char kDesktopDrawWallpaperTime[] { "DrawWallPaperTime" };
inline constexpr char kDesktopLoadFilesTime[] { "LoadFilesTime" };
inline constexpr char kDesktopLoadFilesCount[] { "LoadFilesCount" };
}   // namespace DataPersistence

}   // namespace Global
}   // namespace dfmbase

Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::CreateFileType);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::ViewMode);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::ItemRoles);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::ThumbnailSize);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::TransparentStatus);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::TransparentStatus *)
Q_DECLARE_METATYPE(QList<DFMBASE_NAMESPACE::Global::ItemRoles> *)

#endif   // DFM_GLOBAL_DEFINES_H
