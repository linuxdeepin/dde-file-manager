// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMBUTTON_H
#define CUSTOMBUTTON_H

#include <DIconButton>
#include <DToolButton>
#include <DStyle>

DWIDGET_USE_NAMESPACE

class CustomDIconButton : public DIconButton
{
    Q_OBJECT
public:
    explicit CustomDIconButton(QWidget *parent = nullptr);
    explicit CustomDIconButton(DStyle::StandardPixmap iconType, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class CustomDToolButton : public DToolButton
{
    Q_OBJECT
public:
    explicit CustomDToolButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void initStyleOption(QStyleOptionToolButton *option) const;
};

#endif   // CUSTOMBUTTON_H
