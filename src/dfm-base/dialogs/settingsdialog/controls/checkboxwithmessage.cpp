// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkboxwithmessage.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>

CheckBoxWithMessage::CheckBoxWithMessage(QWidget *parent)
    : QWidget(parent)
{
    auto widget = new QWidget(this);
    widget->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    setLayout(layout);

    checkBox = new QCheckBox(widget);
    layout->addWidget(checkBox);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(30, 0, 0, 0);
    layout->addLayout(hLayout);

    QPalette paletteText;
    QColor color("#526A7F");
    paletteText.setColor(QPalette::Text, color);
    msgLabel = new QLabel(widget);
    msgLabel->setPalette(paletteText);
    msgLabel->setWordWrap(true);
    hLayout->addWidget(msgLabel);

    connect(checkBox, &QCheckBox::stateChanged, this, &CheckBoxWithMessage::stateChanged);
}

void CheckBoxWithMessage::setDisplayText(const QString &checkText, const QString &msg)
{
    checkBox->setText(checkText);
    msgLabel->setText(msg);
}

void CheckBoxWithMessage::setChecked(bool checked)
{
    checkBox->setChecked(checked);
}
