// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COVER_H
#define COVER_H

#include <DLabel>

DWIDGET_USE_NAMESPACE

class Cover : public DLabel
{
    Q_OBJECT
public:
    explicit Cover(QWidget *parent = Q_NULLPTR);

    void setCoverPixmap(const QPixmap &pixmap);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    QPixmap         background;
};

#endif // COVER_H
