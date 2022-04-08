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

constexpr char kMimetypeAppDesktop[] { "application/x-desktop" };
constexpr char kMimeTypeCdImage[] { "application/x-cd-image" };
constexpr char kMimeTypeISO9660Image[] { "application/x-iso9660-image" };
constexpr char kMimeTypeAppXml[] { "application/xml" };
constexpr char kMimeTypeAppPdf[] { "application/pdf" };
constexpr char kMimeTypeAppMxf[] { "application/mxf" };
constexpr char kMimeTypeAppVMAsf[] { "application/vnd.ms-asf" };
constexpr char kMimeTypeAppCRRMedia[] { "application/cnd.rn-realmedia" };
constexpr char kMimeTypeAppVRRMedia[] { "application/vnd.rn-realmedia" };
constexpr char kMimeTypeTextHtml[] { "text/html" };
constexpr char kMimeTypeAppXhtmlXml[] { "application/xhtml+xml" };
constexpr char kMimeTypeTextXPython[] { "text/x-python" };
constexpr char kMimeTypeTextPlain[] { "text/plain" };
constexpr char kMimeTypeAppXOleStorage[] { "application/x-ole-storage" };
constexpr char kMimeTypeAppZip[] { "application/zip" };
constexpr char kMimeTypeAppXDesktop[] { "application/x-desktop" };
// image
constexpr char kMimeTypeImageIef[] { "image/ief" };
constexpr char kMimeTypeImageTiff[] { "image/tiff" };
constexpr char kMimeTypeImageXTMultipage[] { "image/x-tiff-multipage" };
constexpr char kMimeTypeImageVDMultipage[] { "image/vnd.djvu+multipage" };
constexpr char kMimeTypeImageXADng[] { "image/x-adobe-dng" };
constexpr char kMimeTypeImageJpeg[] { "image/jpeg" };
constexpr char kMimeTypeImagePng[] { "image/png" };
constexpr char kMimeTypeImagePipeg[] { "image/pipeg" };
constexpr char kMimeTypeImageVDjvu[] { "image/vnd.djvu" };
constexpr char kMimeTypeImageSvgXml[] { "image/svg+xml" };

constexpr uint8_t kMaxFileNameCharCount { 255 };

constexpr char kSmb[] { "smb" };
constexpr char kSmbShare[] { "smb-share" };
constexpr char kFtp[] { "ftp" };
constexpr char kSFtp[] { "sftp" };
constexpr char kGPhoto[] { "gphoto" };
constexpr char kGPhoto2[] { "gphoto2" };
constexpr char kFile[] { "file" };
constexpr char kDesktop[] { "desktop" };
constexpr char kMtp[] { "mtp" };
constexpr char kAfc[] { "afc" };
constexpr char kDav[] { "dav" };
constexpr char kEntry[] { "entry" };
constexpr char kBurn[] { "burn" };
constexpr char kComputer[] { "computer" };
constexpr char kDevice[] { "device" };
constexpr char kTrash[] { "trash" };
}   //namespace Global

DFMBASE_END_NAMESPACE

Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::CreateFileType);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::OperatorCallback);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::OperatorHandleCallback);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::ViewMode);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::ItemRoles);

#endif   // DFM_GLOBAL_DEFINES_H
