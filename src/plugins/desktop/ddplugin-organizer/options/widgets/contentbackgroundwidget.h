/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef CONTENTBACKGROUNDWIDGET_H
#define CONTENTBACKGROUNDWIDGET_H

#include <QWidget>

namespace ddplugin_organizer {

class ContentBackgroundWidget : public QWidget
{
    Q_OBJECT
public:
    enum RoundEdge {
        kNone = 0x0,
        kTop = 0x01,
        kBottom = 0x2,
        kBoth = RoundEdge::kTop | RoundEdge::kBottom
    };
public:
    explicit ContentBackgroundWidget(QWidget *parent = nullptr);

    inline void setRadius(int r) {rectRadius = r;}
    inline int radius() const {return rectRadius;}

    inline void setRoundEdge(RoundEdge flag) {edge = flag;}
    inline RoundEdge roundEdge() const {return edge;}
signals:

protected:
    void paintEvent(QPaintEvent *event) override;
public slots:
protected:
    int rectRadius = 0;
    RoundEdge edge = RoundEdge::kNone;
};

}

#endif // CONTENTBACKGROUNDWIDGET_H
