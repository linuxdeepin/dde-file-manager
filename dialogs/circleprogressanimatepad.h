/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CIRCLEPROGRESSANIMATEPAD_H
#define CIRCLEPROGRESSANIMATEPAD_H

#include <QLabel>
#include <QTimer>
#include <QColor>

class CircleProgressAnimatePad : public QWidget
{
    Q_OBJECT
public:
    explicit CircleProgressAnimatePad(QWidget *parent = 0);
    void initConnections();

    int lineWidth() const;
    void setLineWidth(int lineWidth);

    int currentValue() const;
    void setCurrentValue(int currentValue);

    int minmumValue() const;
    void setMinmumValue(int minmumValue);

    int maximumValue() const;
    void setMaximumValue(int maximumValue);

    int angle() const;
    void setAngle(int angle);

    QColor chunkColor() const;
    void setChunkColor(const QColor &chunkColor);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &backgroundColor);

    int fontSize() const;
    void setFontSize(int fontSize);

    bool animationRunning() const;

    void setPauseState(bool on);
    void setCanPause(bool on);

signals:
    void clicked();

public slots:
    void startAnimation();
    void stopAnimation();
    void stepAngle();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int m_lineWidth = 3;
    int m_currentValue = 10;
    int m_minmumValue = 0;
    int m_maximumValue = 100;
    int m_angle = 90 * 16;
    int m_fontSize = 12;

    QColor m_chunkColor = Qt::cyan;
    QColor m_backgroundColor = Qt::darkCyan;

    QTimer* m_timer;
    bool isAnimateStarted = false;
    bool isPauseState = false;
    bool canPause = false;
};

#endif // CIRCLEPROGRESSANIMATEPAD_H
