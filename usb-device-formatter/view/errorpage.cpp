/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "errorpage.h"
#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

ErrorPage::ErrorPage(const QString &errorStr, QWidget *parent):
    QFrame(parent)
{
    m_errorText = errorStr;
    initUI();
}

void ErrorPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QLabel* warnIconLabel = new QLabel(this);
    warnIconLabel->setPixmap(QIcon::fromTheme("dialog-error").pixmap(128, 128));
    m_errorLabel = new QLabel(this);
    m_errorLabel->setObjectName("StatusLabel");
    m_errorLabel->setText(m_errorText);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setAlignment(Qt::AlignHCenter);

    mainLayout->addSpacing(30);
    mainLayout->addWidget(warnIconLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(m_errorLabel, 0, Qt::AlignHCenter);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
    setContentsMargins(20, 0, 20, 0);
}

void ErrorPage::setErrorMsg(const QString &errorMsg)
{
    m_errorLabel->setText(errorMsg);
}

void ErrorPage::resizeEvent(QResizeEvent *event)
{
    m_errorLabel->setFixedWidth(event->size().width() - 80);
    m_errorLabel->setFixedHeight(80);
}
