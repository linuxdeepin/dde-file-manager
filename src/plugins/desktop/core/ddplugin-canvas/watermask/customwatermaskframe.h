// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CUSTOMWATERMASKFRAME_H
#define CUSTOMWATERMASKFRAME_H

#include "ddplugin_canvas_global.h"
#include "watermaskframe.h"

#include <QFrame>
#include <QMap>

class QHBoxLayout;
class QLabel;
class QJsonObject;
namespace ddplugin_canvas {

class CustomWaterMaskFrame : public QFrame
{
    Q_OBJECT
public:
    explicit CustomWaterMaskFrame(QWidget *parent = nullptr);
    ~CustomWaterMaskFrame();

    void loadConfig();
    void refresh();
    void setPosition();

public slots:
    void onSystemMaskShow(bool showEnable, QPoint pos, int height);
protected slots:
    void onConfigChanged(const QString &cfg, const QString &key);
    void update();

protected:
    QPixmap maskPixmap(const QString &uri, const QSize &size, qreal pixelRatio);
    //void getSystemMaskPosision();
    //void paintEvent(QPaintEvent *) override;

private:
    bool maskOpen = false;
    bool systemMaskEnable = false;
    int systemMaskHeight;

    QLabel *logoLabel = nullptr;

    QString maskLogoUri;

    QSize maskSize = QSize(0, 0);
    QPoint maskOffset = QPoint(0, 0);
    QPoint systemMaskPosition = QPoint(0, 0);
    //QSize systemMaskSize = QSize(0, 0);
};
}

#endif // CUSTOMWATERMASKFRAME_H
