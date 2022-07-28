/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "fileviewstatusbar.h"
#include "dfm-base/utils/fileutils.h"

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

    if (visible)
        scaleSlider->show();
    else
        scaleSlider->hide();
}

int FileViewStatusBar::scalingValue()
{
    if (!scaleSlider)
        return 1;

    return scaleSlider->value();
}

QSlider *FileViewStatusBar::scalingSlider() const
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
    scaleSlider = new QSlider(this);
    scaleSlider->setOrientation(Qt::Horizontal);
    scaleSlider->adjustSize();
    scaleSlider->setFixedWidth(120);
    scaleSlider->setMaximum(1);
    scaleSlider->setMinimum(0);
    scaleSlider->setTickInterval(1);
    scaleSlider->setPageStep(1);
    scaleSlider->hide();
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
    addWidget(scaleSlider, 0, Qt::AlignRight);
}

void FileViewStatusBar::clearLayoutAndAnchors()
{
    BasicStatusBar::clearLayoutAndAnchors();

    DAnchorsBase::clearAnchors(scaleSlider);
}
