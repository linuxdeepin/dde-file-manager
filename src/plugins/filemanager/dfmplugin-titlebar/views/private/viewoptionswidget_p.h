// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWOPTIONSWIDGET_P_H
#define VIEWOPTIONSWIDGET_P_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <DFrame>
#include <DSlider>
#include <DLabel>

#include <QCheckBox>
#include <QVBoxLayout>

namespace dfmplugin_titlebar {
using DFMBASE_NAMESPACE::Global::ViewMode;
class ViewOptionsWidget;
class ViewOptionsWidgetPrivate : public QObject
{
    Q_OBJECT
    friend class ViewOptionsWidget;
    ViewOptionsWidget *q { nullptr };

    QUrl fileUrl;

    // title
    DTK_WIDGET_NAMESPACE::DLabel *title { nullptr };

    // icon size
    QFrame *iconSizeFrame { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *iconSizeTitle { nullptr };
    DTK_WIDGET_NAMESPACE::DSlider *iconSizeSlider { nullptr };

    // grid density
    QFrame *gridDensityFrame { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *gridDensityTitle { nullptr };
    DTK_WIDGET_NAMESPACE::DSlider *gridDensitySlider { nullptr };

    // list Height
    QFrame *listHeightFrame { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *listHeightTitle { nullptr };
    DTK_WIDGET_NAMESPACE::DSlider *listHeightSlider { nullptr };

    // display preview
    DTK_WIDGET_NAMESPACE::DFrame *displayPreviewWidget { nullptr };
    QCheckBox *displayPreviewCheckBox { nullptr };

public:
    explicit ViewOptionsWidgetPrivate(ViewOptionsWidget *qq);
    virtual ~ViewOptionsWidgetPrivate();

private:
    void initializeUi();
    void initConnect();
    void setUrl(const QUrl &url);
    void switchMode(ViewMode mode);
};

}   // namespace dfmplugin_titlebar

#endif   // VIEWOPTIONSWIDGET_P_H
