// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKGROUNDDEFAULT_H
#define BACKGROUNDDEFAULT_H

#include "ddplugin_background_global.h"

#include "interfaces/background/abstractbackground.h"

DDP_BACKGROUND_BEGIN_NAMESPACE

class BackgroundDefault : public DFMBASE_NAMESPACE::AbstractBackground
{
    Q_OBJECT
public:
    explicit BackgroundDefault(const QString &screenName, QWidget *parent = nullptr);
    virtual void setMode(int mode) override;
    virtual void setDisplay(const QString &path) override;
    virtual void updateDisplay() override;
protected:
    void paintEvent(QPaintEvent *event) override;
    QPixmap getPixmap(const QString &path, const QPixmap &defalutPixmap);

private:
    int painted = 3;
    QPixmap pixmap;
    QPixmap noScalePixmap;
};

DDP_BACKGROUND_END_NAMESPACE

#endif // BACKGROUNDDEFAULT_H
