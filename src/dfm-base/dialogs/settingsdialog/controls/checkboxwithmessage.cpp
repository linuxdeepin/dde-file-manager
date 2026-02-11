// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkboxwithmessage.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <DTipLabel>

CheckBoxWithMessage::CheckBoxWithMessage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    checkBox = new QCheckBox(this);
    layout->addWidget(checkBox);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(30, 0, 0, 0);
    layout->addLayout(hLayout);

    msgLabel = new Dtk::Widget::DTipLabel("", this);
    msgLabel->setAlignment(Qt::AlignLeft);
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
