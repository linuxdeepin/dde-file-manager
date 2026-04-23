// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "checkboxwidthocrindex.h"

#include "indexstatuscontroller.h"
#include "ocrindexclient.h"
#include "searchmanager/searchmanager.h"

namespace dfmplugin_search {

CheckBoxWidthOcrIndex::CheckBoxWidthOcrIndex(QWidget *parent)
    : IndexStatusCheckBox(parent)
{
    IndexStatusControllerOptions options;
    options.logTag = QStringLiteral("OcrIndex");
    options.inactiveText = tr("Enable to search text in images");
    options.indexingInitialText = tr("Building index");
    options.indexingFilesText = tr("Building index, %1 files indexed");
    options.indexingItemsText = tr("Building index, %1/%2 items indexed");
    options.failedMainText = tr("Index update failed, please");
    options.failedLinkText = tr("try updating again");
    options.completedMainText = tr("Index update completed, last update time: %1");
    options.completedLinkText = tr("Update index now");

    m_controller = new IndexStatusController(this, OcrIndexClient::instance(), options, this);
    connect(SearchManager::instance(), &SearchManager::enableOcrTextSearchChanged, this, [this](bool enable) {
        m_controller->syncCheckedState(enable);
    });
}

void CheckBoxWidthOcrIndex::connectToBackend()
{
    m_controller->connectToBackend();
}

void CheckBoxWidthOcrIndex::initStatusBar()
{
    m_controller->initStatusBar();
}

}   // namespace dfmplugin_search
