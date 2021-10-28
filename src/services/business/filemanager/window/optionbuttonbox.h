/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef OptionButtonBox_H
#define OptionButtonBox_H

#include "dfm_filemanager_service_global.h"

#include <QWidget>
#include <QToolButton>

DSB_FM_BEGIN_NAMESPACE

class ActionButton : public QToolButton
{
    Q_OBJECT

public:
    explicit ActionButton(QWidget *parent = nullptr);
    void setAction(QAction *action);
    QAction *action() const;
};

class OptionButtonBoxPrivate;
class OptionButtonBox : public QWidget
{
    Q_OBJECT
    friend class OptionButtonBoxPrivate;
    OptionButtonBoxPrivate *const d;
public:
    explicit OptionButtonBox(QWidget *parent = nullptr);
    QToolButton *iconViewButton() const;
    QToolButton *listViewButton() const;
    QToolButton *detailButton() const;
    void setIconViewButton(QToolButton *iconViewButton);
    void setListViewButton(QToolButton *listViewButton);
    void setDetailButton(QToolButton *detailButton);
};

DSB_FM_END_NAMESPACE

#endif // OptionButtonBox_H
