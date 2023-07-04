// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computerstatusbar.h"

#include <QApplication>

namespace dfmplugin_computer {

void ComputerStatusBar::showSingleSelectionMessage()
{
    setTipText(qApp->translate("dfmbase::BasicStatusBarPrivate", "%1 item selected").arg(1));
}

}
