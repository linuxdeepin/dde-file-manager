// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchhintcontroller.h"
#include "utils/searchhelper.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/dpf.h>

#include <QLocale>
#include <QJsonDocument>
#include <QJsonObject>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_search;

SearchHintController *SearchHintController::instance()
{
    static SearchHintController controller;
    return &controller;
}

SearchHintController::SearchHintController(QObject *parent)
    : QObject(parent)
{
}

SearchHintController::~SearchHintController()
{
}

bool SearchHintController::shouldShowAuthHint(const QUrl &url, QString *text)
{
    Q_UNUSED(url)
    const QString &cfg = DConfig::kSearchCfgPath;
    if (DConfigManager::instance()->value(cfg, DConfig::kSearchAuthHintDone, false).toBool())
        return false;

    bool needHint = !DConfigManager::instance()->value(cfg, DConfig::kEnableFileIndexSearch, false).toBool();
    if (!needHint)
        return false;

    QStringList modes = SearchHintController::instance()->disabledSearchModes();
    if (modes.isEmpty())
        return false;

    if (text) {
        *text = SearchHintController::tr("Authorize to enable %1")
                        .arg(QLocale().createSeparatedList(modes));
    }
    return true;
}

void SearchHintController::onAuthHintAction(const QString &id)
{
    if (id == "authorize")
        authorizeSearchExperience();
    else if (id == "close")
        dismissAuthHint();
}

void SearchHintController::authorizeSearchExperience()
{
    const QString &cfg = DConfig::kSearchCfgPath;
    DConfigManager::instance()->setValue(cfg, DConfig::kEnableFileIndexSearch, true);
    DConfigManager::instance()->setValue(cfg, DConfig::kEnableFullTextSearch, true);
    DConfigManager::instance()->setValue(cfg, DConfig::kEnableOcrTextSearch, true);
    DConfigManager::instance()->setValue(cfg, DConfig::kEnableSemanticSearch, true);

    QStringList actions = { "view-index-status", tr("View") };
    QJsonObject paramObj;
    paramObj.insert("group", SEARCH_SETTING_GROUP);
    QJsonObject argsObj;
    argsObj.insert("action", "settings");
    argsObj.insert("params", paramObj);
    const QStringList cmdShowSettings { "file-manager.sh",
                                        "--event",
                                        QJsonDocument(argsObj).toJson(QJsonDocument::Compact) };
    QVariantMap hints = { { "x-deepin-action-view-index-status", cmdShowSettings } };
    UniversalUtils::notifyMessage(
            "",
            tr("Index is being built. You can check the index status in Settings."),
            actions,
            hints);
}

void SearchHintController::dismissAuthHint()
{
    DConfigManager::instance()->setValue(DConfig::kSearchCfgPath, DConfig::kSearchAuthHintDone, true);

    QStringList disabledModes = SearchHintController::instance()->disabledSearchModes();
    if (!disabledModes.isEmpty()) {
        const QString &modeList = QLocale().createSeparatedList(disabledModes);
        UniversalUtils::notifyMessage(
                tr("File Manager"),
                tr("You can manually enable %1 in Settings — Advanced — Search.").arg(modeList));
    }

    fmInfo() << "Search authorization hint dismissed by user";
}

void SearchHintController::showHint(const QVariantMap &content)
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_ShowViewHint",
                         SearchHelper::scheme(), content);
}

void SearchHintController::closeHint()
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_CloseViewHint", SearchHelper::scheme());
}

void SearchHintController::updateHint(const QVariantMap &updates)
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_UpdateViewHint",
                         SearchHelper::scheme(), updates);
}

QStringList SearchHintController::disabledSearchModes()
{
    const QString &cfg = DConfig::kSearchCfgPath;
    QStringList modes;
    if (!DConfigManager::instance()->value(cfg, DConfig::kEnableFullTextSearch, true).toBool())
        modes << tr("\"Full-Text search\"");
    if (!DConfigManager::instance()->value(cfg, DConfig::kEnableOcrTextSearch, false).toBool())
        modes << tr("\"Image-Content search\"");
    if (!DConfigManager::instance()->value(cfg, DConfig::kEnableSemanticSearch, false).toBool()
        || !DConfigManager::instance()->value(cfg, DConfig::kEnableFileIndexSearch, false).toBool())
        modes << tr("\"Smart search\"");
    return modes;
}
