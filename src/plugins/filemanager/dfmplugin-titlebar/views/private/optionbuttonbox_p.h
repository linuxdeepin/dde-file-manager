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
#ifndef OPTIONBUTTONBOX_P_H
#define OPTIONBUTTONBOX_P_H

#include "dfmplugin_titlebar_global.h"

#include "services/filemanager/titlebar/titlebar_defines.h"
#include "dfm-base/dfm_global_defines.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QButtonGroup>

class QToolButton;
class QHBoxLayout;

DSB_FM_USE_NAMESPACE
DPTITLEBAR_BEGIN_NAMESPACE

using DFMBASE_NAMESPACE::Global::ViewMode;
class OptionButtonBox;
class OptionButtonBoxPrivate : public QObject
{
    Q_OBJECT
    friend class OptionButtonBox;

public:
    explicit OptionButtonBoxPrivate(OptionButtonBox *parent);

public slots:
    void setViewMode(ViewMode mode);
    void onViewModeChanged(int mode);

private:
    void loadViewMode(const QUrl &url);
    void switchMode(ViewMode mode);

private:
    OptionButtonBox *const q;

    QButtonGroup *buttonGroup { nullptr };
    QToolButton *iconViewButton { nullptr };
    QToolButton *listViewButton { nullptr };
    QToolButton *detailButton { nullptr };
    QHBoxLayout *hBoxLayout { nullptr };

    ViewMode currentMode { ViewMode::kIconMode };
};
DPTITLEBAR_END_NAMESPACE

#endif   // OPTIONBUTTONBOX_P_H
