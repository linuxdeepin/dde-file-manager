// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMBUTTON_H
#define CUSTOMBUTTON_H

#include <DIconButton>
#include <DToolButton>
#include <DStyle>

class CustomDIconButton : public DTK_NAMESPACE::Widget::DIconButton
{
    Q_OBJECT
public:
    explicit CustomDIconButton(QWidget *parent = nullptr);
    explicit CustomDIconButton(DTK_NAMESPACE::Widget::DStyle::StandardPixmap iconType, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class CustomDToolButton : public DTK_NAMESPACE::Widget::DToolButton
{
    Q_OBJECT
public:
    explicit CustomDToolButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void initStyleOption(QStyleOptionToolButton *option) const override;
};

#endif   // CUSTOMBUTTON_H
