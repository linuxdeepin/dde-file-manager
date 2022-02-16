/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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
#ifndef FILEVIEWSTATUSBAR_H
#define FILEVIEWSTATUSBAR_H

#include "dfmplugin_workspace_global.h"
#include "dfm-base/dfm_base_global.h"
#include "dfm-base/widgets/dfmstatusbar/basicstatusbar.h"

#include <dpicturesequenceview.h>
#include <dslider.h>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPWORKSPACE_BEGIN_NAMESPACE

class FileViewStatusBar : public BasicStatusBar
{
    Q_OBJECT
public:
    explicit FileViewStatusBar(QWidget *parent = nullptr);

    void resetScalingSlider(const int stepCount);
    void setScalingVisible(const bool visible);
    int scalingValue();
    QSlider *scalingSlider() const;
    void showLoadingIncator(const QString &tip);
    void hideLoadingIncator();

protected:
    void clearLayoutAndAnchors() override;

private:
    void initScalingSlider();
    void initLoadingIndicator();
    void setCustomLayout();

    DPictureSequenceView *loadingIndicator = nullptr;
    QSlider *scaleSlider = nullptr;
};

DPWORKSPACE_END_NAMESPACE

#endif   // STATUSBAR_H
