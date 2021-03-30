/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#ifndef DFMCRUMBBAR_P_H
#define DFMCRUMBBAR_P_H

#include <dfmglobal.h>
#include <QPushButton>
#include <DListView>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE
DFM_BEGIN_NAMESPACE

class DFMCrumbBar;
class DFMCrumbListviewModel;
class DFMAddressBar;
class DFMCrumbInterface;
class DFMCrumbBarPrivate
{
    Q_DECLARE_PUBLIC(DFMCrumbBar)

public:
    explicit DFMCrumbBarPrivate(DFMCrumbBar *qq);
    ~DFMCrumbBarPrivate();

    // UI
    QPushButton leftArrow;
    QPushButton rightArrow;
    DListView crumbListView;
    DFMCrumbListviewModel *crumbListviewModel = nullptr;
    QHBoxLayout *crumbBarLayout;
    QPoint clickedPos;
    DFMAddressBar *addressBar = nullptr;

    // Scheme support
    DFMCrumbInterface *crumbController = nullptr;

    // Misc
    bool clickableAreaEnabled = false;

    DFMCrumbBar *q_ptr = nullptr;

    void clearCrumbs();
    void checkArrowVisiable();
    void updateController(const DUrl &url);
    void setClickableAreaEnabled(bool enabled);

private:
    void initUI();
    void initData();
    void initConnections();
};

DFM_END_NAMESPACE

#endif // DFMCRUMBBAR_P_H
