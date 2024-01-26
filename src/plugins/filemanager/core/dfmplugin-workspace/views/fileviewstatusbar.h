// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEVIEWSTATUSBAR_H
#define FILEVIEWSTATUSBAR_H

#include "dfmplugin_workspace_global.h"
#include <dfm-base/dfm_base_global.h>
#include <dfm-base/widgets/dfmstatusbar/basicstatusbar.h>

#include <dpicturesequenceview.h>
#include <dslider.h>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
namespace dfmplugin_workspace {

class FileViewStatusBar : public BasicStatusBar
{
    Q_OBJECT
public:
    explicit FileViewStatusBar(QWidget *parent = nullptr);

    void resetScalingSlider(const int stepCount);
    void setScalingVisible(const bool visible);
    int scalingValue();
    DSlider *scalingSlider() const;
    void showLoadingIncator(const QString &tip);
    void hideLoadingIncator();

protected:
    void clearLayoutAndAnchors() override;

private:
    void initScalingSlider();
    void initLoadingIndicator();
    void setCustomLayout();

    DPictureSequenceView *loadingIndicator { nullptr };
    DSlider *scaleSlider { nullptr };
    QWidget *stretchWidget { nullptr };
};

}

#endif   // STATUSBAR_H
