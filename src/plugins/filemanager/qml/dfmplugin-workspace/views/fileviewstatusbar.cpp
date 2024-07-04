// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileviewstatusbar.h"
#include <dfm-base/utils/fileutils.h>

#include <DAnchors>

#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

FileViewStatusBar::FileViewStatusBar(QWidget *parent)
    : BasicStatusBar(parent)
{
    initScalingSlider();
    initLoadingIndicator();
    setCustomLayout();
}

void FileViewStatusBar::resetScalingSlider(const int stepCount)
{
    if (scaleSlider)
        scaleSlider->setMaximum(stepCount);
}

void FileViewStatusBar::setScalingVisible(const bool visible)
{
    if (!scaleSlider)
        return;

    if (visible) {
        stretchWidget->show();
        scaleSlider->show();
    } else {
        stretchWidget->hide();
        scaleSlider->hide();
    }
}

int FileViewStatusBar::scalingValue()
{
    if (!scaleSlider)
        return 1;

    return scaleSlider->value();
}

DSlider *FileViewStatusBar::scalingSlider() const
{
    return scaleSlider;
}

void FileViewStatusBar::showLoadingIncator(const QString &tip)
{
    loadingIndicator->setVisible(true);
    loadingIndicator->play();

    setTipText(tip.isEmpty() ? tr("Loading...") : tip);
}

void FileViewStatusBar::hideLoadingIncator()
{
    loadingIndicator->stop();
    loadingIndicator->setVisible(false);

    setTipText(QString());
}

void FileViewStatusBar::initScalingSlider()
{
    scaleSlider = new DSlider(Qt::Horizontal, this);
    scaleSlider->adjustSize();
    scaleSlider->setFixedWidth(120);
    scaleSlider->setMaximum(1);
    scaleSlider->setMinimum(0);
    scaleSlider->slider()->setTickInterval(1);
    scaleSlider->setPageStep(1);
    scaleSlider->hide();
    scaleSlider->setMouseWheelEnabled(true);
}

void FileViewStatusBar::initLoadingIndicator()
{
    QStringList seq;

    for (int i = 1; i != 91; ++i)
        seq.append(QString(":/images/images/Spinner/Spinner%1.png").arg(i, 2, 10, QChar('0')));

    loadingIndicator = new DPictureSequenceView(this);
    loadingIndicator->setFixedSize(18, 18);
    loadingIndicator->setPictureSequence(seq, true);
    loadingIndicator->setSpeed(20);
    loadingIndicator->hide();
}

void FileViewStatusBar::setCustomLayout()
{
    insertWidget(0, loadingIndicator);

    stretchWidget = new QWidget(this);
    stretchWidget->setMinimumWidth(0);
    stretchWidget->setMaximumWidth(120);
    stretchWidget->setFixedHeight(30);
    stretchWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    stretchWidget->hide();
    insertWidget(1, stretchWidget);

    addWidget(scaleSlider, 0, Qt::AlignRight);
}

void FileViewStatusBar::clearLayoutAndAnchors()
{
    BasicStatusBar::clearLayoutAndAnchors();

    DAnchorsBase::clearAnchors(scaleSlider);
}
