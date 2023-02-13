// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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

private:
    void mousePressEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void editLabelClicked();
};

}
#endif   // EDITLABEL_H
