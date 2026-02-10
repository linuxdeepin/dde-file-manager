// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMDTOOLBUTTON_H
#define CUSTOMDTOOLBUTTON_H

#include <DToolButton>
#include <DStyle>

namespace dfmbase {

class CustomDToolButton : public DTK_NAMESPACE::Widget::DToolButton
{
    Q_OBJECT
public:
    explicit CustomDToolButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void initStyleOption(QStyleOptionToolButton *option) const;
};

}

#endif   // CUSTOMDTOOLBUTTON_H
