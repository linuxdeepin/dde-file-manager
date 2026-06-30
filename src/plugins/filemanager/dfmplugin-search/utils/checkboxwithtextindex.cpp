// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "checkboxwithtextindex.h"

#include "indexstatuscontroller.h"
#include "searchmanager/searchmanager.h"
#include "textindexclient.h"

namespace dfmplugin_search {

CheckBoxWithTextIndex::CheckBoxWithTextIndex(QWidget *parent)
    : IndexStatusCheckBox(parent)
{
    IndexStatusControllerOptions options;
    options.logTag = QStringLiteral("TextIndex");
    options.inactiveText = tr("Enable to search file contents. Indexing may take a few minutes");
    options.indexingInitialText = tr("Building index");
    options.indexingFilesText = tr("Building index, %1 files indexed");
    options.indexingItemsText = tr("Building index, %1/%2 items indexed");
    options.failedMainText = tr("Index update failed, please");
    options.failedLinkText = tr("try updating again");
    options.completedMainText = tr("Index update completed, last update time: %1");
    options.completedLinkText = tr("Update index now");

    m_controller = new IndexStatusController(this, TextIndexClient::instance(), options, this);
    connect(SearchManager::instance(), &SearchManager::enableFullTextSearchChanged, this, [this](bool enable) {
        m_controller->syncCheckedState(enable);
    });
}

void CheckBoxWithTextIndex::connectToBackend()
{
    m_controller->connectToBackend();
}

void CheckBoxWithTextIndex::initStatusBar()
{
    m_controller->initStatusBar();
}

}   // namespace dfmplugin_search
