// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEVIEWSTATUSBAR_H
#define FILEVIEWSTATUSBAR_H

#include "dfmplugin_workspace_global.h"
#include <dfm-base/dfm_base_global.h>
#include <dfm-base/widgets/dfmstatusbar/basicstatusbar.h>

#include <DSpinner>
#include <dslider.h>
#include <DTipLabel>

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
    DTipLabel *findTipLabel() const;

private:
    void initScalingSlider();
    void initLoadingSpinner();
    void setCustomLayout();

    DSpinner *loadingSpinner { nullptr };
    DSlider *scaleSlider { nullptr };
    QWidget *stretchWidget { nullptr };
    QWidget *centerContainer { nullptr };
};

}

#endif   // STATUSBAR_H
