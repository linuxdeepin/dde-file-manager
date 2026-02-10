// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WATERMASKCONTAINER_H
#define WATERMASKCONTAINER_H

#include <QObject>

namespace ddplugin_canvas {

class CustomWaterMaskLabel;
class WatermaskSystem;
class WaterMaskFrame;
class WatermaskContainer : public QObject
{
    Q_OBJECT
public:
    explicit WatermaskContainer(QWidget *parent = nullptr);
    static bool isEnable();
    void refresh();
    void updatePosition();
signals:

public slots:
private:
    //old
    WaterMaskFrame *frame = nullptr;
    // new
    WatermaskSystem *control = nullptr;
    // custom
    CustomWaterMaskLabel *custom = nullptr;
};

}

#endif // WATERMASKCONTAINER_H
