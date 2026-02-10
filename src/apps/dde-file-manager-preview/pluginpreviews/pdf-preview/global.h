// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBAL_H
#define GLOBAL_H
#include "preview_plugin_global.h"

#include <QString>
#include <QWidget>

namespace plugin_filepreview {
/**
 * @brief The FileType enum
 * 支持文档类型
 */
enum FileType {
    kUnknown = 0,
    kPDF = 1,
    kDJVU = 2,
    kDOCX = 3,
    kPS = 4,
    kDOC = 5,
    kPPTX = 6
};

extern "C" FileType fileType(const QString &filePath);

/**
 * @brief The Rotation enum
 * 文档旋转角度
 */
enum Rotation {
    kRotateBy0 = 0,
    kRotateBy90 = 1,
    kRotateBy180 = 2,
    kRotateBy270 = 3,
    kNumberOfRotations = 4
};

/**
 * @brief The LayoutMode enum
 * 布局模式
 */
enum LayoutMode {
    SinglePageMode = 0,
    TwoPagesMode = 1,
    NumberOfLayoutModes = 2
};

/**
 * @brief The ScaleMode enum
 * 缩放模式
 */
enum ScaleMode {
    kScaleFactorMode = 0,
    kFitToPageWidthMode = 1,
    kFitToPageHeightMode = 2,
    kNumberOfScaleModes = 3,
    kFitToPageDefaultMode = 4,
    kFitToPageWorHMode = 5
};

inline constexpr double kImageBrowserWidth = 780.0;
inline constexpr int kPdfWidgetWidth = 830;
inline constexpr int kPdfWidgetHeight = 500;

extern "C" void setMainWidget(QWidget *pMainWidget);
extern "C" QWidget *getMainDialog();
}
#endif   // GLOBAL_H
