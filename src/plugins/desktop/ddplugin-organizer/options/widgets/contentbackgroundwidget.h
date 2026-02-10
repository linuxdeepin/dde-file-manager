// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
