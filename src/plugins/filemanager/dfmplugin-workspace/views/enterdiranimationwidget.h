// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
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
    Q_PROPERTY(double appearProcess READ getAppearProcess WRITE setAppearProcess)
    Q_PROPERTY(double disappearProcess READ getDisappearProcess WRITE setDisappearProcess)
public:
    explicit EnterDirAnimationWidget(QWidget *parent = nullptr);

    void setAppearPixmap(const QPixmap &pm);
    void setDisappearPixmap(const QPixmap &pm);
    void resetWidgetSize(const QSize &size);

    void playAppear();
    void playDisappear();
    void stopAndHide();

    void setAppearProcess(double value);
    inline double getAppearProcess() const { return appearProcess; };

    void setDisappearProcess(double value);
    inline double getDisappearProcess() const { return disappearProcess; };

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void onProcessChanged();

private:
    void init();
    void paintPix(QPainter *painter, const QPixmap &pix, double process);

private:
    QPixmap appearPix;
    QPixmap disappearPix;

    QPropertyAnimation *appearAnim { nullptr };
    QPropertyAnimation *disappearAnim { nullptr };

    double appearProcess { 0.0 };
    double disappearProcess { 0.0 };

    double configScale { 0.8 };
    double configOpacity { 0.0 };
};

}   // namespace dfmplugin_workspace

#endif   // ENTERDIRANIMATIONWIDGET_H
