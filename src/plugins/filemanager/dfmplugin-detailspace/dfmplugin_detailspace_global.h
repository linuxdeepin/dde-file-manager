// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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

inline constexpr int kViewAnimationDuration { 366 };

using BasicExpandMap = QMultiMap<BasicFieldExpandEnum, QPair<QString, QString>>;
using BasicViewFieldFunc = std::function<QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(const QUrl &url)>;

// Extension view function types for reusable widget pattern
using ViewExtensionCreateFunc = std::function<QWidget *(const QUrl &url)>;
using ViewExtensionUpdateFunc = std::function<void(QWidget *widget, const QUrl &url)>;
using ViewExtensionShouldShowFunc = std::function<bool(const QUrl &url)>;

// Legacy alias for compatibility
using CustomViewExtensionView = ViewExtensionCreateFunc;

DPDETAILSPACE_END_NAMESPACE

Q_DECLARE_METATYPE(DPDETAILSPACE_NAMESPACE::ViewExtensionCreateFunc);
Q_DECLARE_METATYPE(DPDETAILSPACE_NAMESPACE::ViewExtensionUpdateFunc);
Q_DECLARE_METATYPE(DPDETAILSPACE_NAMESPACE::ViewExtensionShouldShowFunc);
Q_DECLARE_METATYPE(DPDETAILSPACE_NAMESPACE::BasicViewFieldFunc);

#endif   // DFMPLUGIN_DETAILSPACE_GLOBAL_H
