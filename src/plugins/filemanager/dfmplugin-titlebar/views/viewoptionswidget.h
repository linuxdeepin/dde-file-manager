// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWOPTIONSWIDGET_H
#define VIEWOPTIONSWIDGET_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <DBlurEffectWidget>
#include <QScopedPointer>

namespace dfmplugin_titlebar {
class ViewOptionsWidgetPrivate;
class ViewOptionsWidget : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
    QScopedPointer<ViewOptionsWidgetPrivate> d;

public:
    explicit ViewOptionsWidget(QWidget *parent = nullptr);
    ~ViewOptionsWidget() override;

    void exec(const QPoint &pos, DFMBASE_NAMESPACE::Global::ViewMode mode, const QUrl &url);

Q_SIGNALS:
    void hidden();
    void displayPreviewVisibleChanged(bool visible);

protected:
    void hideEvent(QHideEvent *event) override;
};

}   // namespace dfmplugin_titlebar

#endif   // VIEWOPTIONSWIDGET_H
