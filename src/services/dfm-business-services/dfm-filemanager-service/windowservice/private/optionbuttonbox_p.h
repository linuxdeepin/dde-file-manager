/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#ifndef OPTIONBUTTONBOX_P_H
#define OPTIONBUTTONBOX_P_H

#include "dfm_filemanager_service_global.h"

#include <QPushButton>
#include <QHBoxLayout>

class QToolButton;
class QHBoxLayout;
DSB_FM_BEGIN_NAMESPACE
class OptionButtonBox;
class OptionButtonBoxPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(OptionButtonBox)
    OptionButtonBox * const q_ptr;

    QToolButton* iconViewButton = nullptr;
    QToolButton* listViewButton = nullptr;
    QToolButton* detailButton = nullptr;
    QHBoxLayout* hBoxLayout = nullptr;

    explicit OptionButtonBoxPrivate(OptionButtonBox *parent);
};

DSB_FM_END_NAMESPACE

#endif // OPTIONBUTTONBOX_P_H
