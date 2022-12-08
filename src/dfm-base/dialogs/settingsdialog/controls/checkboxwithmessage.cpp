// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkboxwithmessage.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>

CheckBoxWithMessage::CheckBoxWithMessage(QWidget *parent)
    : QWidget(parent)
{
    widget = new QWidget(this);
    widget->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    checkBox = new QCheckBox(widget);
    layout->addWidget(checkBox);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(30, 0, 0, 0);
    QPalette paletteText;
    QColor color("#526A7F");
    paletteText.setColor(QPalette::Text, color);
    message = new QLabel(widget);
    message->setPalette(paletteText);
    QFontMetrics metricsLabel(widget->font());
    if (metricsLabel.width(message->text()) > 380) {
        message->setToolTip(message->text());
        QString text = metricsLabel.elidedText(text, Qt::ElideRight, 380);
        message->setText(text);
    } else {
        message->setToolTip(QString());
    }
    hLayout->addWidget(message);
    layout->addLayout(hLayout);
    setLayout(layout);
}

void CheckBoxWithMessage::setText(const QString &text)
{
    checkBox->setText(text);
}

void CheckBoxWithMessage::setMessage(const QString &msg)
{
    message->setText(msg);
}

QCheckBox *CheckBoxWithMessage::getCheckBox()
{
    return checkBox;
}
