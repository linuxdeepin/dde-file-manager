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

#define DFMGLOBAL_BEGIN_NAMESPACE namespace DFMBASE_NAMESPACE::Global {
#define DFMGLOBAL_END_NAMESPACE }
#define DFMGLOBAL_NAMESPACE DFMBASE_NAMESPACE::Global
#define DFMGLOBAL_USE_NAMESPACE using namespace DFMBASE_NAMESPACE::Global;

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
using CallbackArgus = QSharedPointer<QMap<CallbackKey, QVariant>>;
using OperaterCallback = void (*)(const CallbackArgus args);

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
constexpr uint8_t kMaxFileNameCharCount { 255 };
}   //namespace Global

DFMBASE_END_NAMESPACE

Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::CreateFileType);
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::OperaterCallback);

Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::ViewMode);

#endif   // DFM_GLOBAL_DEFINES_H
