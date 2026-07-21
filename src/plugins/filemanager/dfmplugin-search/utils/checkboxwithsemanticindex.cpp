// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkboxwithsemanticindex.h"

#include "searchmanager/searchmanager.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_search {

CheckBoxWithSemanticIndex::CheckBoxWithSemanticIndex(QWidget *parent)
    : IndexStatusCheckBox(parent)
{
    // 提示文案：开启智能搜索后支持在文件管理器中使用自然语言搜索文件
    setInactiveText(tr("Enable to search files using natural language, e.g. \"PPT documents edited yesterday\""));

    // 读取文件索引当前状态以初始化启用/禁用状态
    m_fileIndexEnabled = DConfigManager::instance()->value(
                                                           DConfig::kSearchCfgPath, DConfig::kEnableFileIndexSearch, true)
                                 .toBool();

    connect(SearchManager::instance(), &SearchManager::enableFileIndexSearchChanged,
            this, &CheckBoxWithSemanticIndex::setDisabledByFileIndex);

    connect(SearchManager::instance(), &SearchManager::enableSemanticSearchChanged, this, [this](bool enable) {
        setChecked(enable);
    });
}

void CheckBoxWithSemanticIndex::initStatusBar()
{
    // 智能搜索没有索引后端，始终以 Inactive 状态展示提示文案
    setStatus(Status::Inactive);
    setDisabledByFileIndex(m_fileIndexEnabled);
}

void CheckBoxWithSemanticIndex::setDisabledByFileIndex(bool fileIndexEnabled)
{
    m_fileIndexEnabled = fileIndexEnabled;

    // 禁用整个 widget（含 checkbox + 提示文案）置灰，
    // 但不调用 setChecked()，保留用户之前的勾选状态。
    setEnabled(fileIndexEnabled);
}

}   // namespace dfmplugin_search
