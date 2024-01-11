// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERSTATUSBAR_H
#define COMPUTERSTATUSBAR_H

#include <dfm-base/widgets/dfmstatusbar/basicstatusbar.h>

namespace dfmplugin_computer {
class ComputerStatusBar : public DFMBASE_NAMESPACE::BasicStatusBar
{
    Q_OBJECT
public:
    explicit ComputerStatusBar(QWidget *parent)
        : DFMBASE_NAMESPACE::BasicStatusBar(parent) {}

    void showSingleSelectionMessage();

protected:
    void paintEvent(QPaintEvent *);
};
}

#endif   // COMPUTERSTATUSBAR_H
