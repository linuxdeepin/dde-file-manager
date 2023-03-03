// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
