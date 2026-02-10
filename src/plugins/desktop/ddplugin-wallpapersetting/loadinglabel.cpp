// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadinglabel.h"

#include <DSpinner>
#include <DAnchors>

#include <QLabel>

DWIDGET_USE_NAMESPACE
using namespace ddplugin_wallpapersetting;

#define WAITITEM_ICON_CONTANT_SPACE 10  //gap
#define MARGIN_OF_WAITICON 6

LoadingLabel::LoadingLabel(QWidget *parent)
    : QFrame(parent)
{
    //set window and transparency
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    init();
}

LoadingLabel::~LoadingLabel()
{

}

void LoadingLabel::start()
{
    animationSpinner->setAttribute(Qt::WA_TransparentForMouseEvents);
    animationSpinner->setFocusPolicy(Qt::NoFocus);
    animationSpinner->setFixedSize(spinnerSize);

    DAnchorsBase::setAnchor(animationSpinner, Qt::AnchorVerticalCenter, icon, Qt::AnchorVerticalCenter);
    DAnchorsBase::setAnchor(animationSpinner, Qt::AnchorRight, icon, Qt::AnchorRight);
    DAnchorsBase::getAnchorBaseByWidget(animationSpinner)->setRightMargin(MARGIN_OF_WAITICON);

    animationSpinner->show();
    animationSpinner->start();
}

void LoadingLabel::init()
{
    icon = new QLabel(this);
    contant = new QLabel(this);
    animationSpinner = new DSpinner(icon);
}

void LoadingLabel::resize(const QSize &size)
{
    //set window size and the son size
    this->setFixedSize(size);
    moveDistance = size.width() * proportion;

    //calculates the total width of the son widget
    int sumwidth = iconSize.width() + contantSize.width() + WAITITEM_ICON_CONTANT_SPACE;

    //adjust the width of m_movedistance
    if ((size.width() - static_cast<int>(moveDistance)) < sumwidth) {
        int temp = sumwidth - (size.width() - static_cast<int>(moveDistance));
        moveDistance -= temp;
    }

    //move the son widget by m_movedistance
    if (sumwidth <= size.width()) {
        icon->move(static_cast<int>(moveDistance), size.height() / 3);
        icon->setFixedSize(iconSize);
        contant->move(static_cast<int>(moveDistance) + WAITITEM_ICON_CONTANT_SPACE + icon->width(), size.height() / 3 + MARGIN_OF_WAITICON);
        contant->setFixedSize(contantSize);
    } else {
        fmDebug() << "the parent widget is too small that can not to display the son widget";
        icon->setFixedSize(QSize(0, 0));
        contant->setFixedSize(QSize(0, 0));
    }
}

void LoadingLabel::setText(const QString &text)
{
    contant->setText(text);
}

