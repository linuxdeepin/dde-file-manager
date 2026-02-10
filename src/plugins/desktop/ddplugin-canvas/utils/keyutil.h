// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYUTILS_H
#define KEYUTILS_H

#include "ddplugin_canvas_global.h"

#include <dfm-base/utils/windowutils.h>

#include <QWidget>
#include <QApplication>

namespace ddplugin_canvas {

inline bool isCtrlPressed()
{
    return DFMBASE_NAMESPACE::WindowUtils::keyCtrlIsPressed();
}

inline bool isShiftPressed()
{
    return DFMBASE_NAMESPACE::WindowUtils::keyShiftIsPressed();
}

inline bool isCtrlOrShiftPressed()
{
    return isCtrlPressed() || isShiftPressed();
}

inline bool isAltPressed()
{
    return DFMBASE_NAMESPACE::WindowUtils::keyAltIsPressed();
}

}

#endif   // KEYUTILS_H
