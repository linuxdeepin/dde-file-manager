/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef CrumbBar_P_H
#define CrumbBar_P_H

#include <QPushButton>
#include <DListView>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class CrumbBar;
class CrumbModel;
class CrumbBarPrivate
{
    friend class CrumbBar;
    CrumbBar *const q;

    QPushButton leftArrow;
    QPushButton rightArrow;
    DListView crumbView;
    CrumbModel *crumbModel = nullptr;
    QHBoxLayout *crumbBarLayout;
    QPoint clickedPos;
    bool clickableAreaEnabled = false;

public:
    explicit CrumbBarPrivate(CrumbBar *qq);
    virtual ~CrumbBarPrivate();

    void clearCrumbs();
    void checkArrowVisiable();
    void updateController(const QUrl &url);
    void setClickableAreaEnabled(bool enabled);

private:
    void initUI();
    void initData();
    void initConnections();
};

#endif   // CrumbBar_P_H
