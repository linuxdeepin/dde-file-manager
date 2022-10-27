// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkboxwithmessage.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>

CheckBoxWithMessage::CheckBoxWithMessage(QWidget *parent) : QWidget(parent)
{
    m_widget = new QWidget(this);
    m_widget->setContentsMargins(0,0,0,0);
    QVBoxLayout *layout = new QVBoxLayout(m_widget);
    layout->setMargin(0);
    m_checkBox = new QCheckBox(m_widget);
    layout->addWidget(m_checkBox);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(30,0,0,0);
    QPalette paletteText;
    QColor color("#526A7F");
    paletteText.setColor(QPalette::Text, color);
    m_message = new QLabel(m_widget);
    m_message->setPalette(paletteText);
    QFontMetrics metricsLabel(m_widget->font());
    if (metricsLabel.width(m_message->text()) > 380 ){
        m_message->setToolTip(m_message->text());
        QString text = metricsLabel.elidedText(text, Qt::ElideRight, 380);
        m_message->setText(text);
    } else {
        m_message->setToolTip(QString());
    }
    hLayout->addWidget(m_message);
    layout->addLayout(hLayout);
    setLayout(layout);
}

void CheckBoxWithMessage::setText(const QString &text)
{
    m_checkBox->setText(text);
}

void CheckBoxWithMessage::setMessage(const QString &message)
{
    m_message->setText(message);
}

QCheckBox *CheckBoxWithMessage::checkBox()
{
    return m_checkBox;
}
