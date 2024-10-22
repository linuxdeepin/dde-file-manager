// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URLPUSHBUTTON_H
#define URLPUSHBUTTON_H

#include "dfmplugin_titlebar_global.h"

#include <DPushButton>

#include <QColor>
#include <QScopedPointer>

class QUrl;
class QEvent;
class QListView;
class QMenu;

namespace dfmplugin_titlebar {
class UrlPushButtonPrivate;
class UrlPushButton : public Dtk::Widget::DPushButton
{
    Q_OBJECT
    QScopedPointer<UrlPushButtonPrivate> d;

public:
    explicit UrlPushButton(QWidget *parent = nullptr);
    ~UrlPushButton() override;

    void setCrumbDatas(const QList<CrumbData> &datas, bool stacked = false);
    QList<CrumbData> crumbDatas() const;

    void setActive(bool active);
    bool isActive() const;

    void setActiveSubDirectory(const QString &subDir);
    QString activeSubDirectory() const;

Q_SIGNALS:
    void urlButtonActivated(const QUrl &url);
    void selectSubDirs();

protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
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

#endif   // URLPUSHBUTTON_H
