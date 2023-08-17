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
public slots:
    void refresh();
    void onSystemMaskShow(bool showEnable, QPoint pos);
protected slots:
    void onConfigChanged(const QString &cfg, const QString &key);
protected:
    void loadConfig();
    void update();
    void setPosition();
    QPixmap maskPixmap(const QString &uri, const QSize &size, qreal pixelRatio);
private:
    QLabel *logoLabel = nullptr;

    bool maskOpen = false;
    QString maskLogoUri;
    QSize maskSize = QSize(0, 0);
    QPoint maskOffset = QPoint(0, 0);

    bool systemMaskEnable = false;
    QPoint systemMaskPosition = QPoint(0, 0);
};
}

#endif // CUSTOMWATERMASKFRAME_H
