/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: yanghao<yanghao@uniontech.com>
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

#include "windowutils.h"

#include <QApplication>

bool DFMBASE_NAMESPACE::WindowUtils::isWayLand()
{
    //! This function can only be called after QApplication to return a valid value, before it will return a null value
    Q_ASSERT(qApp);
    return QApplication::platformName() == "wayland";
}

bool DFMBASE_NAMESPACE::WindowUtils::keyShiftIsPressed()
{
    return qApp->keyboardModifiers() == Qt::ShiftModifier;
}

bool DFMBASE_NAMESPACE::WindowUtils::keyCtrlIsPressed()
{
    return qApp->queryKeyboardModifiers() == Qt::ControlModifier;
}

bool DFMBASE_NAMESPACE::WindowUtils::keyAltIsPressed()
{
    return qApp->keyboardModifiers() == Qt::AltModifier;
}
