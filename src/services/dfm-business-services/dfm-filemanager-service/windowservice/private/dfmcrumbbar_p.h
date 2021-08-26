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

//#include <dfmglobal.h>
#include <QPushButton>
#include <DListView>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class DFMCrumbBar;
class DFMCrumbModel;
class DFMCrumbBarPrivate
{
    Q_DECLARE_PUBLIC(DFMCrumbBar)
    DFMCrumbBar *const q_ptr;

    QPushButton m_leftArrow;
    QPushButton m_rightArrow;
    DListView m_crumbView;
    DFMCrumbModel *m_crumbModel = nullptr;
    QHBoxLayout *m_crumbBarLayout;
    QPoint m_clickedPos;
    bool m_clickableAreaEnabled = false;

public:

    explicit DFMCrumbBarPrivate(DFMCrumbBar *qq);
    ~DFMCrumbBarPrivate();

    void clearCrumbs();
    void checkArrowVisiable();
    void updateController(const QUrl &url);
    void setClickableAreaEnabled(bool enabled);

private:
    void initUI();
    void initData();
    void initConnections();
};

#endif // DFMCRUMBBAR_P_H
