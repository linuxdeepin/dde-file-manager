/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef DFMPLUGIN_DETAILSPACE_GLOBAL_H
#define DFMPLUGIN_DETAILSPACE_GLOBAL_H

#define DPDETAILSPACE_NAMESPACE dfmplugin_detailspace

#define DPDETAILSPACE_BEGIN_NAMESPACE namespace DPDETAILSPACE_NAMESPACE {
#define DPDETAILSPACE_END_NAMESPACE }
#define DPDETAILSPACE_USE_NAMESPACE using namespace DPDETAILSPACE_NAMESPACE;

#include <functional>
#include <QWidget>

DPDETAILSPACE_BEGIN_NAMESPACE

Q_NAMESPACE
enum DetailFilterType {
    kNotFilter = 0,
    kBasicView = 1,
    kIconView = 1 << 1,
    kFileNameField = 1 << 2,
    kFileSizeField = 1 << 3,
    kFileViewSizeField = 1 << 4,
    kFileDurationField = 1 << 5,
    kFileTypeField = 1 << 6,
    kFileInterviewTimeField = 1 << 7,
    kFileChangeTimeField = 1 << 8
};
Q_ENUM_NS(DetailFilterType)

enum BasicFieldExpandEnum : int {
    kNotAll,
    kFileName,
    kFileSize,
    kFileViewSize,
    kFileDuration,
    kFileType,
    kFileInterviewTime,
    kFileChangeTIme
};
Q_ENUM_NS(BasicFieldExpandEnum)

enum BasicExpandType : int {
    kFieldInsert,
    kFieldReplace
};
Q_ENUM_NS(BasicExpandType)

using BasicExpandMap = QMultiMap<BasicFieldExpandEnum, QPair<QString, QString>>;
using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
using BasicViewFieldFunc = std::function<QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(const QUrl &url)>;

DPDETAILSPACE_END_NAMESPACE

Q_DECLARE_METATYPE(DPDETAILSPACE_NAMESPACE::CustomViewExtensionView);
Q_DECLARE_METATYPE(DPDETAILSPACE_NAMESPACE::BasicViewFieldFunc);

#endif   // DFMPLUGIN_DETAILSPACE_GLOBAL_H
