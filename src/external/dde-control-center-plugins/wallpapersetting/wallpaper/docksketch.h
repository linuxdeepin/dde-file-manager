// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCKSKETCH_H
#define DOCKSKETCH_H

#include <DBlurEffectWidget>

namespace dfm_wallpapersetting {

class DockSketch : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit DockSketch(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
};

}

#endif // DOCKSKETCH_H
