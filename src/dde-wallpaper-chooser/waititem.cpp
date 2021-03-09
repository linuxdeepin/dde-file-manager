/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xinglinkun<xinglinkun@uniontech.com>
 *
 * Maintainer: xinglinkun<xinglinkun@uniontech.com>
 *
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
#include "waititem.h"
#include <QDebug>
#include <DSpinner>
#include <DAnchors>
#include <QLabel>
#define WAITITEM_ICON_CONTANT_SPACE 10  //gap
#define MARGIN_OF_WAITICON 6
WaitItem::WaitItem(QWidget *parent)
    : QFrame(parent)
{
    //set window and transparency
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
}

WaitItem::~WaitItem()
{

}

void WaitItem::initSize(const QSize &size)
{
    //set window size and the son size
    this->setFixedSize(size);
    qDebug() << "WaitItem::initSize " << size;
    m_movedistance = size.width() * m_proportion;
    qDebug() << "m_movedistance " << m_movedistance;

    //judge the child window will overflow ？

    //ｃalculates the total width of the son widget
    int sumwidth = m_iconsize.width() + m_contantsize.width() + WAITITEM_ICON_CONTANT_SPACE;

    //adjust the width of m_movedistance
    if ((size.width() - static_cast<int>(m_movedistance)) < sumwidth) {
        int temp = sumwidth - (size.width() - static_cast<int>(m_movedistance));
        m_movedistance -= temp;
    }

    //move the son widget by m_movedistance
    if (sumwidth <= size.width()) {
        if (m_icon == nullptr) {
            m_icon = new QLabel(this);
            m_icon->move(static_cast<int>(m_movedistance), size.height() / 3);
            m_icon->setFixedSize(m_iconsize);
        }

        if (m_contant == nullptr) {
            m_contant = new QLabel(this);
            m_contant->move(static_cast<int>(m_movedistance) + WAITITEM_ICON_CONTANT_SPACE + m_icon->width(), size.height() / 3 + MARGIN_OF_WAITICON);
            m_contant->setFixedSize(m_contantsize);
        }
    } else {
        qDebug() << "the parent widget is too small that can not to display the son widget";
        m_icon = new QLabel(this);
        m_icon->setFixedSize(QSize(0, 0));
        m_contant = new QLabel(this);
        m_contant->setFixedSize(QSize(0, 0));
    }

    qDebug() << "icon_size " << m_iconsize << "contant_size " << m_contantsize;
    setAnimation();
}

void WaitItem::setContantText(const QString &temp)
{
    m_contant->clear();
    m_contant->setText(temp);
}

void WaitItem::setAnimation()
{
    //set animation
    if (m_animationSpinner != nullptr)
        return;
    m_animationSpinner = new DSpinner(m_icon);
    m_animationSpinner->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_animationSpinner->setFocusPolicy(Qt::NoFocus);
    m_animationSpinner->setFixedSize(m_spinnersize);

    DAnchorsBase::setAnchor(m_animationSpinner, Qt::AnchorVerticalCenter, m_icon, Qt::AnchorVerticalCenter);
    DAnchorsBase::setAnchor(m_animationSpinner, Qt::AnchorRight, m_icon, Qt::AnchorRight);
    DAnchorsBase::getAnchorBaseByWidget(m_animationSpinner)->setRightMargin(MARGIN_OF_WAITICON);

    m_animationSpinner->show();
    m_animationSpinner->start();
}
