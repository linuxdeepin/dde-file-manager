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
#ifndef DFMOPTIONBUTTONBOX_H
#define DFMOPTIONBUTTONBOX_H

#include "dfm_filemanager_service_global.h"

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QToolButton>

DSB_FM_BEGIN_NAMESPACE

class DFMActionButton : public QToolButton
{
    Q_OBJECT

public:
    explicit DFMActionButton(QWidget *parent = nullptr);

    void setAction(QAction *action);
    QAction *action() const;
};

class DFMOptionButtonBox : public QWidget
{
    Q_OBJECT

    QToolButton* m_iconViewButton = nullptr;
    QToolButton* m_listViewButton = nullptr;
    QToolButton* m_detailButton = nullptr;

    QHBoxLayout* m_hBoxLayout = nullptr;

public:
    DFMOptionButtonBox(QWidget * parent = nullptr);

    QToolButton *iconViewButton() const;
    void setIconViewButton(QToolButton *iconViewButton);

    QToolButton *listViewButton() const;
    void setListViewButton(QToolButton *listViewButton);

    QToolButton *detailButton() const;
    void setDetailButton(QToolButton *detailButton);
};

DSB_FM_END_NAMESPACE

#endif // DFMOPTIONBUTTONBOX_H
