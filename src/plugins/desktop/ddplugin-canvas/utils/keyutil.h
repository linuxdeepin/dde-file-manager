/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef KEYUTILS_H
#define KEYUTILS_H

#include "ddplugin_canvas_global.h"

#include <QWidget>
#include <QApplication>

namespace ddplugin_canvas {

inline bool isCtrlPressed() {
    return qApp->keyboardModifiers() == Qt::ControlModifier;
}

inline bool isShiftPressed() {
    return qApp->keyboardModifiers() == Qt::ShiftModifier;
}

inline bool isCtrlOrShiftPressed() {
    return isCtrlPressed() || isShiftPressed();
}

inline bool isAltPressed() {
    return qApp->keyboardModifiers() == Qt::AltModifier;
}

}

#endif // KEYUTILS_H
