// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMWATERMASKLABEL_H
#define CUSTOMWATERMASKLABEL_H

#include "ddplugin_canvas_global.h"

#include <QLabel>

namespace ddplugin_canvas {

class CustomWaterMaskLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CustomWaterMaskLabel(QWidget *parent = nullptr);
    ~CustomWaterMaskLabel();
public slots:
    void refresh();
    void onSystemMaskShow(const QPoint &pos);
protected slots:
    void onConfigChanged(const QString &cfg, const QString &key);
protected:
    void loadConfig();
    void update();
    void setPosition();
    QPixmap maskPixmap(const QString &uri, const QSize &size, qreal pixelRatio);
private:
    bool maskEnabled = false;
    QString maskLogoUri;
    QSize maskSize = QSize(0, 0);
    QPoint maskOffset = QPoint(0, 0);
    QPoint systemMaskPosition = QPoint(0, 0);
};
}

#endif // CUSTOMWATERMASKLABEL_H
