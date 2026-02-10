// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileviewstatusbar.h"
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/dfm_global_defines.h>

#include <DAnchors>

#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

FileViewStatusBar::FileViewStatusBar(QWidget *parent)
    : BasicStatusBar(parent),
      loadingSpinner(nullptr),
      scaleSlider(nullptr),
      stretchWidget(nullptr),
      centerContainer(nullptr)
{
    fmInfo() << "Creating FileViewStatusBar";

    initScalingSlider();
    initLoadingSpinner();
    setCustomLayout();

    fmDebug() << "FileViewStatusBar initialization completed";
}

void FileViewStatusBar::resetScalingSlider(const int stepCount)
{
    if (scaleSlider) {
        scaleSlider->setMaximum(stepCount);
        fmDebug() << "Scaling slider maximum set to:" << stepCount;
    } else {
        fmWarning() << "Cannot reset scaling slider: scaleSlider is null";
    }
}

void FileViewStatusBar::setScalingVisible(const bool visible)
{
    fmDebug() << "Setting scaling slider visibility to:" << visible;

    if (!scaleSlider) {
        fmWarning() << "Cannot set scaling visibility: scaleSlider is null";
        return;
    }

    if (visible) {
        fmDebug() << "Showing scaling slider and stretch widget";
        stretchWidget->show();
        scaleSlider->show();
    } else {
        fmDebug() << "Hiding scaling slider and stretch widget";
        stretchWidget->hide();
        scaleSlider->hide();
    }
}

int FileViewStatusBar::scalingValue()
{
    if (!scaleSlider) {
        fmWarning() << "Cannot get scaling value: scaleSlider is null";
        return 1;
    }

    return scaleSlider->value();
}

DSlider *FileViewStatusBar::scalingSlider() const
{
    return scaleSlider;
}

void FileViewStatusBar::showLoadingIncator(const QString &tip)
{
    fmInfo() << "Showing loading indicator with tip:" << (tip.isEmpty() ? "Loading..." : tip);

    if (loadingSpinner) {
        loadingSpinner->setVisible(true);
        loadingSpinner->start();
        fmDebug() << "Loading indicator started playing";
    } else {
        fmWarning() << "Cannot show loading indicator: loadingIndicator is null";
    }

    setTipText(tip.isEmpty() ? tr("Loading...") : tip);
}

void FileViewStatusBar::hideLoadingIncator()
{
    fmInfo() << "Hiding loading indicator";

    if (loadingSpinner) {
        loadingSpinner->stop();
        loadingSpinner->setVisible(false);
        fmDebug() << "Loading indicator stopped and hidden";
    } else {
        fmWarning() << "Cannot hide loading indicator: loadingIndicator is null";
    }

    setTipText(QString());
}

void FileViewStatusBar::initScalingSlider()
{
    fmDebug() << "Initializing scaling slider";

    scaleSlider = new DSlider(Qt::Horizontal, this);
    scaleSlider->adjustSize();
    scaleSlider->setFixedWidth(120);
    scaleSlider->setMaximum(1);
    scaleSlider->setMinimum(0);
    scaleSlider->slider()->setTickInterval(1);
    scaleSlider->setPageStep(1);
    scaleSlider->hide();
    scaleSlider->setMouseWheelEnabled(true);

    fmDebug() << "Scaling slider initialized with width: 120, range: 0-1";
}

void FileViewStatusBar::initLoadingSpinner()
{
    fmDebug() << "Initializing loading indicator";

    loadingSpinner = new DSpinner(this);
    loadingSpinner->setFixedSize(16, 16);
    loadingSpinner->hide();
}

DTipLabel *FileViewStatusBar::findTipLabel() const
{
    fmDebug() << "Searching for DTipLabel in widget hierarchy";

    // Find the DTipLabel in the widget hierarchy
    for (QObject *child : children()) {
        if (DTipLabel *tipLabel = qobject_cast<DTipLabel *>(child)) {
            fmDebug() << "Found DTipLabel as direct child";
            return tipLabel;
        }

        // Look one level deeper if needed
        for (QObject *grandchild : child->children()) {
            if (DTipLabel *tipLabel = qobject_cast<DTipLabel *>(grandchild))
                return tipLabel;
        }
    }
    return nullptr;
}

void FileViewStatusBar::setCustomLayout()
{
    fmInfo() << "Setting up custom layout for FileViewStatusBar";

    clearLayoutAndAnchors();

    if (!centerContainer) {
        // Create a container widget for center alignment
        centerContainer = new QWidget(this);
        fmDebug() << "Created new center container widget";
    }

    QHBoxLayout *centerLayout = new QHBoxLayout(centerContainer);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(5);
    fmDebug() << "Created center layout with spacing: 5";

    // Add loadingSpinner to the center container
    centerLayout->addWidget(loadingSpinner);
    fmDebug() << "Added loading indicator to center layout";

    // Find the tip label from base class
    DTipLabel *tipLabel = findTipLabel();
    if (tipLabel) {
        // Move the tip from its original parent to our center container
        tipLabel->setParent(centerContainer);
        centerLayout->addWidget(tipLabel);
        fmDebug() << "Found and added tip label to center layout";
    } else {
        fmWarning() << "Tip label not found, layout may be incomplete";
    }

    // Set alignment for the center container
    centerContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Add the center container to the main layout
    insertWidget(0, centerContainer, 1, Qt::AlignCenter);
    fmDebug() << "Inserted center container into main layout";

    // Add scaling slider to the right
    stretchWidget = new QWidget(this);
    stretchWidget->setMinimumWidth(0);
    stretchWidget->setMaximumWidth(120);
    stretchWidget->setFixedHeight(30);
    stretchWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    stretchWidget->hide();
    insertWidget(1, stretchWidget);
    fmDebug() << "Created and inserted stretch widget (width: 0-120, height: 30)";

    addWidget(scaleSlider, 0, Qt::AlignRight);
}

void FileViewStatusBar::clearLayoutAndAnchors()
{
    fmDebug() << "Clearing layout and anchors for FileViewStatusBar components";

    BasicStatusBar::clearLayoutAndAnchors();

    DAnchorsBase::clearAnchors(scaleSlider);
    if (centerContainer) {
        DAnchorsBase::clearAnchors(centerContainer);
        fmDebug() << "Cleared anchors for center container";
    }

    fmDebug() << "Layout and anchors clearing completed";
}
