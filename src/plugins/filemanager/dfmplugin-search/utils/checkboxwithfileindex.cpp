// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkboxwithfileindex.h"

#include "indexstatuscontroller.h"
#include "filenameindexclient.h"
#include "searchmanager/searchmanager.h"

#include <DDialog>

#include <QApplication>

namespace dfmplugin_search {

CheckBoxWithFileIndex::CheckBoxWithFileIndex(QWidget *parent)
    : IndexStatusCheckBox(parent)
{
    IndexStatusControllerOptions options;
    options.logTag = QStringLiteral("FileIndex");
    options.inactiveText = tr("Enable to build the file index immediately for faster file name searches");
    options.indexingInitialText = tr("Building index");
    options.indexingFilesText = tr("Building index, %1 files indexed");
    options.indexingItemsText = tr("Building index, %1/%2 items indexed");
    options.failedMainText = tr("Index update failed, please");
    options.failedLinkText = tr("try updating again");
    options.completedMainText = tr("Index update completed, last update time: %1");
    options.completedLinkText = tr("Update index now");
    options.waitingPowerMainText = tr("Currently using battery, index update has been paused");
    options.waitingPowerSaveMainText = tr("Power saving mode is enabled, index update has been paused");
    options.waitingIdleMainText = tr("Waiting for the device to become idle to continue updating");
    options.waitingUpgradeMainText = tr("Waiting for index service upgrade");
    options.waitingUpdateLinkText = tr("Continue updating");
    options.waitingUpgradeLinkText = tr("Update index now");

    m_controller = new IndexStatusController(this, FileNameIndexClient::instance(), options, this);
    connect(SearchManager::instance(), &SearchManager::enableFileIndexSearchChanged, this, [this](bool enable) {
        m_controller->syncCheckedState(enable);
    });
}

void CheckBoxWithFileIndex::connectToBackend()
{
    m_controller->connectToBackend();
}

void CheckBoxWithFileIndex::initStatusBar()
{
    m_controller->initStatusBar();
}

bool CheckBoxWithFileIndex::acceptCheckStateChange(Qt::CheckState oldState, Qt::CheckState newState)
{
    if (oldState == Qt::CheckState::Checked && newState == Qt::CheckState::Unchecked)
        return confirmDisableFileIndex();

    return true;
}

bool CheckBoxWithFileIndex::confirmDisableFileIndex()
{
    Dtk::Widget::DDialog dialog(qApp->activeWindow());
    dialog.setTitle(tr("Confirm turning off file index?"));
    dialog.setMessage(tr("If turned off, file searches will traverse the file system and severely reduce search speed, and the smart search feature will be unavailable."));
    dialog.addButton(QObject::tr("Cancel"), false, Dtk::Widget::DDialog::ButtonNormal);
    dialog.addButton(QObject::tr("Confirm"), true, Dtk::Widget::DDialog::ButtonRecommend);

    return dialog.exec() == Dtk::Widget::DDialog::Accepted;
}

}   // namespace dfmplugin_search
