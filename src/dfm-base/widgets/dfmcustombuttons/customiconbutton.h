// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMICONBUTTON_H
#define CUSTOMICONBUTTON_H

#include <DIconButton>
#include <DToolButton>
#include <DStyle>

namespace dfmbase {

class CustomDIconButton : public DTK_NAMESPACE::Widget::DIconButton
{
    Q_OBJECT
public:
    explicit CustomDIconButton(QWidget *parent = nullptr);
    explicit CustomDIconButton(DTK_NAMESPACE::Widget::DStyle::StandardPixmap iconType, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

}

#endif   // CUSTOMICONBUTTON_H
