// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENTERDIRANIMATIONWIDGET_H
#define ENTERDIRANIMATIONWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QPropertyAnimation;
QT_END_NAMESPACE

namespace dfmplugin_workspace {

class EnterDirAnimationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EnterDirAnimationWidget(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pix);
    void setScaleAnimParam(const QRect &start, const QRect &end);
    void setTrasparentAnimParam(qreal start, qreal end);
    void setBlankBackgroundVisiable(bool visible = false);
    void resetWidgetSize(const QRect &rect);

    void play();
    void playAppear();
    void playDisappear();

private:
    void init();

    QLabel *blankBackground { nullptr };
    QLabel *freezePixmapContainer { nullptr };
    QPropertyAnimation *scaleAnim { nullptr };
    QPropertyAnimation *transparentAnim { nullptr };
};

}   // namespace dfmplugin_workspace

#endif   // ENTERDIRANIMATIONWIDGET_H
