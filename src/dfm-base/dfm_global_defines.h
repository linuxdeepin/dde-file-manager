/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DFM_GLOBAL_DEFINES_H
#define DFM_GLOBAL_DEFINES_H

#include "dfm_base_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <QObject>

#define DFMGLOBAL_NAMESPACE DFMBASE_NAMESPACE::Global

#define DFMGLOBAL_BEGIN_NAMESPACE namespace DFMGLOBAL_NAMESPACE {
#define DFMGLOBAL_END_NAMESPACE }
#define DFMGLOBAL_USE_NAMESPACE using namespace DFMGLOBAL_NAMESPACE;

DFMBASE_BEGIN_NAMESPACE
namespace Global {
enum class ViewMode {
    kNoneMode = 0x00,
    kIconMode = 0x01,
    kListMode = 0x02,
    kExtendMode = 0x04,
    kAllViewMode = kIconMode | kListMode | kExtendMode
};

enum class CallbackKey : uint8_t {
    kWindowId,   // quint64 windowId
    kSuccessed,   // bool
    kSourceUrls,   // QList<QUrl>
    kTargets,   // QList<QUrl>
    kJobHandle,   // JobHandlePointer
    kCustom,   // QVariant
};

enum ItemRoles {
    kItemFileDisplayNameRole = Qt::DisplayRole,
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
    kItemColorRole = Qt::TextColorRole,
    kItemUrlRole = Qt::UserRole + 1,
    kItemFileLastModifiedRole = Qt::UserRole + 2,
    kItemFileSizeRole = Qt::UserRole + 3,
    kItemFileMimeTypeRole = Qt::UserRole + 4,
    kItemFilePathRole = Qt::UserRole + 5,
    kItemColumListRole = Qt::UserRole + 6,
    kItemColumWidthScaleListRole = Qt::UserRole + 7,
    kItemCornerMarkTLRole = Qt::UserRole + 8,
    kItemCornerMarkTRRole = Qt::UserRole + 9,
    kItemCornerMarkBLRole = Qt::UserRole + 10,
    kItemCornerMarkBRRole = Qt::UserRole + 11,
    kItemIconLayersRole = Qt::UserRole + 12,
    kItemNameRole = Qt::UserRole + 13,
    kItemFilePinyinNameRole = Qt::UserRole + 14,
    kItemFileBaseNameRole = Qt::UserRole + 15,
    kItemFileSuffixRole = Qt::UserRole + 16,
    kItemFileNameOfRenameRole = Qt::UserRole + 17,
    kItemFileBaseNameOfRenameRole = Qt::UserRole + 18,
    kItemFileSuffixOfRenameRole = Qt::UserRole + 19,
    kItemExtraProperties = Qt::UserRole + 20,
    kItemFileIconModelToolTipRole = Qt::UserRole + 21,   // 用于返回图标视图下的tooltip
    kItemFileLastReadRole = Qt::UserRole + 22,

    kItemUnknowRole = Qt::UserRole + 999
};

using CallbackArgus = QSharedPointer<QMap<CallbackKey, QVariant>>;
using OperatorCallback = std::function<void(const CallbackArgus args)>;

using OperatorHandleCallback = std::function<void(QSharedPointer<AbstractJobHandler>)>;

enum CreateFileType : uint8_t {
    kCreateFileTypeUnknow = 0,
    kCreateFileTypeFolder,
    kCreateFileTypeText,
    kCreateFileTypeExcel,
    kCreateFileTypeWord,
    kCreateFileTypePowerpoint,
    kCreateFileTypeDefault = 0xff,
};

inline constexpr char kMimetypeAppDesktop[] { "application/x-desktop" };
inline constexpr char kMimeTypeCdImage[] { "application/x-cd-image" };
inline constexpr char kMimeTypeISO9660Image[] { "application/x-iso9660-image" };
inline constexpr char kMimeTypeAppXml[] { "application/xml" };
inline constexpr char kMimeTypeAppPdf[] { "application/pdf" };
inline constexpr char kMimeTypeAppMxf[] { "application/mxf" };
inline constexpr char kMimeTypeAppVMAsf[] { "application/vnd.ms-asf" };
inline constexpr char kMimeTypeAppCRRMedia[] { "application/cnd.rn-realmedia" };
inline constexpr char kMimeTypeAppVRRMedia[] { "application/vnd.rn-realmedia" };
inline constexpr char kMimeTypeTextHtml[] { "text/html" };
inline constexpr char kMimeTypeAppXhtmlXml[] { "application/xhtml+xml" };
inline constexpr char kMimeTypeTextXPython[] { "text/x-python" };
inline constexpr char kMimeTypeTextPlain[] { "text/plain" };
inline constexpr char kMimeTypeAppXOleStorage[] { "application/x-ole-storage" };
inline constexpr char kMimeTypeAppZip[] { "application/zip" };
inline constexpr char kMimeTypeAppXDesktop[] { "application/x-desktop" };
// image
inline constexpr char kMimeTypeImageIef[] { "image/ief" };
inline constexpr char kMimeTypeImageTiff[] { "image/tiff" };
inline constexpr char kMimeTypeImageXTMultipage[] { "image/x-tiff-multipage" };
inline constexpr char kMimeTypeImageVDMultipage[] { "image/vnd.djvu+multipage" };
inline constexpr char kMimeTypeImageXADng[] { "image/x-adobe-dng" };
inline constexpr char kMimeTypeImageJpeg[] { "image/jpeg" };
inline constexpr char kMimeTypeImagePng[] { "image/png" };
inline constexpr char kMimeTypeImagePipeg[] { "image/pipeg" };
inline constexpr char kMimeTypeImageVDjvu[] { "image/vnd.djvu" };
inline constexpr char kMimeTypeImageSvgXml[] { "image/svg+xml" };

inline constexpr char kMimeDataUserIDKey[] { "userid_for_drag" };

inline constexpr uint8_t kMaxFileNameCharCount { 255 };

inline constexpr char kSmb[] { "smb" };
inline constexpr char kSmbShare[] { "smb-share" };
inline constexpr char kFtp[] { "ftp" };
inline constexpr char kSFtp[] { "sftp" };
inline constexpr char kGPhoto[] { "gphoto" };
inline constexpr char kGPhoto2[] { "gphoto2" };
inline constexpr char kFile[] { "file" };
inline constexpr char kDesktop[] { "desktop" };
inline constexpr char kMtp[] { "mtp" };
inline constexpr char kAfc[] { "afc" };
inline constexpr char kDav[] { "dav" };
inline constexpr char kEntry[] { "entry" };
inline constexpr char kBurn[] { "burn" };
inline constexpr char kComputer[] { "computer" };
inline constexpr char kDevice[] { "device" };
inline constexpr char kTrash[] { "trash" };
inline constexpr char kRecent[] { "recent" };
inline constexpr char kUserShare[] { "usershare" };
}   //namespace Global

DFMBASE_END_NAMESPACE

Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::CreateFileType);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::OperatorCallback);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::OperatorHandleCallback);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::ViewMode);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::ItemRoles);

#endif   // DFM_GLOBAL_DEFINES_H
