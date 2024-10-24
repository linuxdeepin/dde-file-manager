// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTBYBUTTON_H
#define SORTBYBUTTON_H

#include "dfmplugin_titlebar_global.h"

#include <DToolButton>

#include <QWidget>

namespace dfmplugin_titlebar {
class SortByButtonPrivate;
class SortByButton : public DTK_WIDGET_NAMESPACE::DToolButton
{
    Q_OBJECT
    friend class SortByButtonPrivate;
    SortByButtonPrivate *const d;

public:
    explicit SortByButton(QWidget *parent = nullptr);
    virtual ~SortByButton();

protected:
    void paintEvent(QPaintEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};
}

#endif   // SORTBYBUTTON_H
