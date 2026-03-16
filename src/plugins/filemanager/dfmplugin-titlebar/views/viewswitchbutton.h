// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWSWITCHBUTTON_H
#define VIEWSWITCHBUTTON_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <DToolButton>

#include <QWidget>

namespace dfmplugin_titlebar {

class ViewSwitchButtonPrivate;

class ViewSwitchButton : public DTK_WIDGET_NAMESPACE::DToolButton
{
    Q_OBJECT
    friend class ViewSwitchButtonPrivate;
    ViewSwitchButtonPrivate *const d;

public:
    explicit ViewSwitchButton(QWidget *parent = nullptr);
    ~ViewSwitchButton() override;

    void setViewModeIcon(DFMBASE_NAMESPACE::Global::ViewMode mode);
    void setViewModeActionEnabled(DFMBASE_NAMESPACE::Global::ViewMode mode, bool enabled);

signals:
    void viewModeChangeRequested(DFMBASE_NAMESPACE::Global::ViewMode mode);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

}   // namespace dfmplugin_titlebar

#endif   // VIEWSWITCHBUTTON_H
