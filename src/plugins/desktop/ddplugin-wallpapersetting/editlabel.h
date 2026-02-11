// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITLABEL_H
#define EDITLABEL_H

#include "ddplugin_wallpapersetting_global.h"

#include <QLabel>

namespace ddplugin_wallpapersetting {

class EditLabel : public QLabel
{
    Q_OBJECT
public:
    explicit EditLabel(QWidget *parent = nullptr);
    void setHotZoom(const QRect &rect);
protected:
    void mousePressEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void editLabelClicked();
private:
    QRect hotZoom;
};

}
#endif   // EDITLABEL_H
