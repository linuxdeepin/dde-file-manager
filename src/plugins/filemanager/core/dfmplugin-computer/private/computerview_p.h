// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERVIEW_P_H
#define COMPUTERVIEW_P_H
#include "dfmplugin_computer_global.h"
#include "views/computerview.h"

namespace dfmplugin_computer {
class ComputerStatusBar;
class ComputerModel;
class ComputerViewPrivate
{
    friend class ComputerView;

public:
    explicit ComputerViewPrivate(ComputerView *qq = nullptr);
    int visibleItemCount();

private:
    ComputerView *q { nullptr };
    ComputerStatusBar *statusBar { nullptr };
};
}
#endif   // COMPUTERVIEW_P_H
