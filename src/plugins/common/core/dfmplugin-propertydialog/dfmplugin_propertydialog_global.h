/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef DFMPLUGIN_PROPERTYDIALOG_GLOBAL_H
#define DFMPLUGIN_PROPERTYDIALOG_GLOBAL_H

#define DPPROPERTYDIALOG_BEGIN_NAMESPACE namespace dfmplugin_propertydialog {
#define DPPROPERTYDIALOG_END_NAMESPACE }
#define DPPROPERTYDIALOG_USE_NAMESPACE using namespace dfmplugin_propertydialog;
#define DPPROPERTYDIALOG_NAMESPACE dfmplugin_propertydialog

#include <QWidget>
#include <QVariantHash>

#include <functional>

DPPROPERTYDIALOG_BEGIN_NAMESPACE
namespace PropertyActionId {
inline constexpr char kProperty[] { "property" };
}

Q_NAMESPACE
enum PropertyFilterType {
    kNotFilter = 0,
    kIconTitle = 1,
    kBasisInfo = 1 << 1,
    kPermission = 1 << 2,
    kFileSizeFiled = 1 << 3,
    kFileCountFiled = 1 << 4,
    kFileTypeFiled = 1 << 5,
    kFilePositionFiled = 1 << 6,
    kFileCreateTimeFiled = 1 << 7,
    kFileAccessedTimeFiled = 1 << 8,
    kFileModifiedTimeFiled = 1 << 9
};
Q_ENUM_NS(PropertyFilterType)

enum BasicFieldExpandEnum : int {
    kNotAll,
    kFileSize,
    kFileCount,
    kFileType,
    kFilePosition,
    kFileCreateTime,
    kFileAccessedTime,
    kFileModifiedTime,
};
Q_ENUM_NS(BasicFieldExpandEnum)

enum BasicExpandType : int {
    kFieldInsert,
    kFieldReplace
};
Q_ENUM_NS(BasicExpandType)

enum class ComputerInfoItem : uint8_t {
    kName,
    kVersion,
    kEdition,
    kOSBuild,
    kType,
    kCpu,
    kMemory
};

using BasicExpandMap = QMultiMap<BasicFieldExpandEnum, QPair<QString, QString>>;
using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
using BasicViewFieldFunc = std::function<QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(const QUrl &url)>;
using ViewIntiCallback = std::function<void(QWidget *w, const QVariantHash &opt)>;

static constexpr char kOption_Key_Name[] { "Option_Key_Name" };
static constexpr char kOption_Key_BasicInfoExpand[] { "Option_Key_BasicInfoExpand" };
static constexpr char kOption_Key_ExtendViewExpand[] { "Option_Key_ExtendViewExpand" };
static constexpr char kOption_Key_ViewIndex[] { "Option_Key_ViewIndex" };
static constexpr char kOption_Key_ViewInitCalback[] { "Option_Key_ViewInitCalback" };
static constexpr char kOption_Key_CreatorCalback[] { "Option_Key_CreatorCalback" };

DPPROPERTYDIALOG_END_NAMESPACE

Q_DECLARE_METATYPE(DPPROPERTYDIALOG_NAMESPACE::CustomViewExtensionView);
Q_DECLARE_METATYPE(DPPROPERTYDIALOG_NAMESPACE::BasicViewFieldFunc);
Q_DECLARE_METATYPE(DPPROPERTYDIALOG_NAMESPACE::ViewIntiCallback);

#endif   // DFMPLUGIN_PROPERTYDIALOG_GLOBAL_H
