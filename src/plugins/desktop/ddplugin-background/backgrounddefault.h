// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDDEFAULT_H
#define BACKGROUNDDEFAULT_H

#include "ddplugin_background_global.h"

#include <QWidget>
#include <QSharedPointer>

DDP_BACKGROUND_BEGIN_NAMESPACE

class BackgroundDefault : public QWidget
{
    Q_OBJECT
public:
    explicit BackgroundDefault(const QString &screenName, QWidget *parent = nullptr);
    void setPixmap(const QPixmap &pix);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void sendPaintReport();

    int painted = 3;
    QString screen;
    QPixmap pixmap;
};

DDP_BACKGROUND_END_NAMESPACE

typedef QSharedPointer<ddplugin_background::BackgroundDefault> BackgroundWidgetPointer;

#endif   // BACKGROUNDDEFAULT_H
