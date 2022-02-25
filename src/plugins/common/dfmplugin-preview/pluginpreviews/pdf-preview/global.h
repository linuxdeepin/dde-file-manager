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

#ifndef GLOBAL_H
#define GLOBAL_H
#include "preview_plugin_global.h"

#include <QString>
#include <QWidget>

PREVIEW_BEGIN_NAMESPACE
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

extern "C" void setMainWidget(QWidget *pMainWidget);
extern "C" QWidget *getMainDialog();
PREVIEW_END_NAMESPACE
#endif   // GLOBAL_H
