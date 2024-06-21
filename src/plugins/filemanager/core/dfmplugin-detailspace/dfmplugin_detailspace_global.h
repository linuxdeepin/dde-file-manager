// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_DETAILSPACE_GLOBAL_H
#define DFMPLUGIN_DETAILSPACE_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPDETAILSPACE_NAMESPACE dfmplugin_detailspace

#define DPDETAILSPACE_BEGIN_NAMESPACE namespace DPDETAILSPACE_NAMESPACE {
#define DPDETAILSPACE_END_NAMESPACE }
#define DPDETAILSPACE_USE_NAMESPACE using namespace DPDETAILSPACE_NAMESPACE;

#include <functional>
#include <QWidget>

DPDETAILSPACE_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPDETAILSPACE_NAMESPACE)

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
Q_FLAG_NS(DetailFilterType)

// TODO: try remove it
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
