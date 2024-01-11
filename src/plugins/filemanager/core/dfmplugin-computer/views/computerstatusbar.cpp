// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computerstatusbar.h"

#include <QApplication>
#include <QStyleOption>
#include <QPainter>

namespace dfmplugin_computer {

void ComputerStatusBar::showSingleSelectionMessage()
{
    setTipText(qApp->translate("dfmbase::BasicStatusBarPrivate", "%1 item selected").arg(1));
}

void ComputerStatusBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_PanelStatusBar, &opt, &p, this);
}

}
