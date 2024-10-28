// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWOPTIONSBUTTON_H
#define VIEWOPTIONSBUTTON_H

#include "dfmplugin_titlebar_global.h"

#include <DToolButton>

#include <QScopedPointer>

namespace dfmplugin_titlebar {
class ViewOptionsButtonPrivate;
class ViewOptionsButton : public Dtk::Widget::DToolButton
{
    Q_OBJECT
    QScopedPointer<ViewOptionsButtonPrivate> d;

public:
    explicit ViewOptionsButton(QWidget *parent = nullptr);
    ~ViewOptionsButton() override;
    void switchMode(DFMBASE_NAMESPACE::Global::ViewMode mode, const QUrl &url);

    void setVisible(bool visible) override;

Q_SIGNALS:
    void displayPreviewVisibleChanged(bool visible);

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

}   // namespace dfmplugin_titlebar

#endif   // VIEWOPTIONSBUTTON_H
