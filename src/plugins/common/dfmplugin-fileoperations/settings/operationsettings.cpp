// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "operationsettings.h"

#include <DSettingsOption>
#include <dsettingsbackend.h>
#include <DRadioButton>
#include <DTipLabel>

#include <QCheckBox>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QLabel>

using namespace dfmplugin_fileoperations;
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

QWidget *OperationSettings::createSyncModeItem(QObject *opt)
{
    auto option = qobject_cast<DSettingsOption *>(opt);
    if (!option)
        return {};

    // Create main widget and layout
    QWidget *widget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create button group for mutual exclusion
    QButtonGroup *buttonGroup = new QButtonGroup(widget);

    // Create sync mode radio button with message
    QWidget *syncWidget = new QWidget(widget);
    QVBoxLayout *syncLayout = new QVBoxLayout(syncWidget);
    syncLayout->setContentsMargins(0, 0, 0, 0);

    DRadioButton *syncBtn = new DRadioButton(syncWidget);
    syncBtn->setText(tr("Real-time data synchronization (Recommended)"));
    syncLayout->addWidget(syncBtn);

    QHBoxLayout *syncMsgLayout = new QHBoxLayout();
    syncMsgLayout->setContentsMargins(30, 0, 0, 0);
    syncLayout->addLayout(syncMsgLayout);

    DTipLabel *syncMsgLabel = new Dtk::Widget::DTipLabel(tr("Once the files are copied, you can unplug the external drive without needing to \"Safely Remove\" it."), syncWidget);
    syncMsgLabel->setAlignment(Qt::AlignLeft);
    syncMsgLabel->setWordWrap(true);
    syncMsgLayout->addWidget(syncMsgLabel);

    // Create performance mode radio button with message
    QWidget *performWidget = new QWidget(widget);
    QVBoxLayout *performLayout = new QVBoxLayout(performWidget);
    performLayout->setContentsMargins(0, 0, 0, 0);

    DRadioButton *performBtn = new DRadioButton(performWidget);
    performBtn->setText(tr("Performance First (Requires Safe Removal)"));
    performLayout->addWidget(performBtn);

    QHBoxLayout *performMsgLayout = new QHBoxLayout();
    performMsgLayout->setContentsMargins(30, 0, 0, 0);
    performLayout->addLayout(performMsgLayout);

    DTipLabel *performMsgLabel = new Dtk::Widget::DTipLabel(tr("File copy data is not synchronized in real-time. After the copy is complete, you must use \"Safe Removal\" to "
                                                               "disconnect the external storage device. Otherwise, file corruption or data loss may occur."),
                                                            performWidget);
    performMsgLabel->setAlignment(Qt::AlignLeft);
    performMsgLabel->setWordWrap(true);
    performMsgLayout->addWidget(performMsgLabel);

    // Add buttons to button group with IDs
    buttonGroup->addButton(syncBtn, 0);   // ID 0 for sync mode (true)
    buttonGroup->addButton(performBtn, 1);   // ID 1 for performance mode (false)

    // Set initial state based on option value
    bool syncMode = option->value().toBool();
    syncBtn->setChecked(syncMode);
    performBtn->setChecked(!syncMode);

    // Connect button group signal to update option
    connect(buttonGroup, &QButtonGroup::idClicked, option, [=](int id) {
        bool syncMode = (id == 0);
        option->setValue(syncMode);
    });

    // Connect option value change to update buttons
    connect(option, &DSettingsOption::valueChanged, buttonGroup, [=](QVariant value) {
        bool syncMode = value.toBool();
        syncBtn->setChecked(syncMode);
        performBtn->setChecked(!syncMode);
    });

    // Add widgets to main layout
    mainLayout->addWidget(syncWidget);
    mainLayout->addWidget(performWidget);

    return widget;
}
