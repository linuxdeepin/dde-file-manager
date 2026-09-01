// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchhintcontroller.h"
#include "utils/searchhelper.h"
#include "utils/textindexclient.h"
#include "utils/ocrindexclient.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/viewhintmessage/viewhintmessage.h>

#include <dfm-framework/dpf.h>

#include <dfm-search/dsearch_global.h>

#include <DSpinner>

#include <QLocale>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

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

void SearchHintController::startListening()
{
    if (m_listening)
        return;

    fmInfo() << "SearchHintController: start listening to index status";
    m_listening = true;

    auto *textClient = TextIndexClient::instance();
    connect(textClient, &AbstractIndexClient::indexStatusResult,
            this, &SearchHintController::onTextStatusResult);
    connect(textClient, &AbstractIndexClient::indexStatusChanged,
            this, &SearchHintController::onTextStatusChanged);

    auto *ocrClient = OcrIndexClient::instance();
    connect(ocrClient, &AbstractIndexClient::indexStatusResult,
            this, &SearchHintController::onOcrStatusResult);
    connect(ocrClient, &AbstractIndexClient::indexStatusChanged,
            this, &SearchHintController::onOcrStatusChanged);

    m_textStatusValid = false;
    m_ocrStatusValid = false;
    textClient->getIndexStatus();
    ocrClient->getIndexStatus();
}

void SearchHintController::stopListening()
{
    if (!m_listening || hasSearchWindow())
        return;

    fmInfo() << "SearchHintController: stop listening, clearing all state";
    m_listening = false;

    disconnect(TextIndexClient::instance(), nullptr, this, nullptr);
    disconnect(OcrIndexClient::instance(), nullptr, this, nullptr);

    m_textState.clear();
    m_textGrade.clear();
    m_ocrState.clear();
    m_ocrGrade.clear();
    m_textStatusValid = false;
    m_ocrStatusValid = false;

    hints.clear();
    windowStates.clear();
}

bool SearchHintController::hasSearchWindow() const
{
    for (quint64 id : FMWindowsIns.windowIdList()) {
        auto *window = FMWindowsIns.findWindowById(id);
        if (window && SearchHelper::isSearchFile(window->currentUrl()))
            return true;
    }
    return false;
}

bool SearchHintController::shouldShowAuthHint(QString *text) const
{
    const QString &cfg = DConfig::kSearchCfgPath;
    if (DConfigManager::instance()->value(cfg, DConfig::kSearchAuthHintDone, false).toBool())
        return false;

    bool needHint = !DConfigManager::instance()->value(cfg, DConfig::kEnableFileIndexSearch, false).toBool();
    if (!needHint)
        return false;

    QStringList modes = disabledSearchModes();
    if (modes.isEmpty())
        return false;

    if (text) {
        *text = SearchHintController::tr("Authorize to enable %1")
                        .arg(QLocale().createSeparatedList(modes));
    }
    return true;
}

void SearchHintController::authorizeSearchExperience()
{
    const QString &cfg = DConfig::kSearchCfgPath;
    DConfigManager::instance()->setValue(cfg, DConfig::kEnableFileIndexSearch, true);
    DConfigManager::instance()->setValue(cfg, DConfig::kEnableFullTextSearch, true);
    DConfigManager::instance()->setValue(cfg, DConfig::kEnableOcrTextSearch, true);
    DConfigManager::instance()->setValue(cfg, DConfig::kEnableSemanticSearch, true);

    fmInfo() << "Search authorization: all index features enabled";
}

void SearchHintController::dismissAuthHint()
{
    DConfigManager::instance()->setValue(DConfig::kSearchCfgPath, DConfig::kSearchAuthHintDone, true);

    QStringList disabledModes = disabledSearchModes();
    if (!disabledModes.isEmpty()) {
        const QString &modeList = QLocale().createSeparatedList(disabledModes);
        UniversalUtils::notifyMessage(
                tr("File Manager"),
                tr("You can manually enable %1 in Settings — Advanced — Search.").arg(modeList));
    }

    fmInfo() << "Search authorization hint dismissed by user";
}

void SearchHintController::tryShowHint(quint64 winId)
{
    if (!m_listening)
        startListening();

    if (hints.value(winId))
        return;

    evaluateAndShow(winId);

    if (!m_textStatusValid)
        TextIndexClient::instance()->getIndexStatus();
    if (!m_ocrStatusValid)
        OcrIndexClient::instance()->getIndexStatus();
}

void SearchHintController::closeAllHints(quint64 winId)
{
    auto &ws = windowStates[winId];
    ws.currentType = HintType::None;
    ws.currentText.clear();

    if (hints.value(winId)) {
        hints[winId]->close();
    }
}

void SearchHintController::closeHint(quint64 winId)
{
    if (hints.value(winId))
        hints[winId]->close();

    windowStates[winId].currentType = HintType::None;
    windowStates[winId].currentText.clear();
}

void SearchHintController::onTextStatusResult(const QString &state, const QString &grade, bool success)
{
    if (!success) {
        fmWarning() << "Failed to get text index status";
        return;
    }
    m_textState = state;
    m_textGrade = grade;
    m_textStatusValid = true;
    fmDebug() << "Text index status:" << state << "grade:" << grade;
    evaluateAndShowAllWindows();
}

void SearchHintController::onOcrStatusResult(const QString &state, const QString &grade, bool success)
{
    if (!success) {
        fmWarning() << "Failed to get OCR index status";
        return;
    }
    m_ocrState = state;
    m_ocrGrade = grade;
    m_ocrStatusValid = true;
    fmDebug() << "OCR index status:" << state << "grade:" << grade;
    evaluateAndShowAllWindows();
}

void SearchHintController::onTextStatusChanged(const QString &state, const QString &grade)
{
    fmDebug() << "Text index status changed:" << state << "grade:" << grade;
    if (m_textStatusValid && m_textState != state && m_textState != "Running" && state == "Running") {
        for (auto it = windowStates.begin(); it != windowStates.end(); ++it)
            it.value().dismissedTypes.clear();
    }
    m_textState = state;
    m_textGrade = grade;
    m_textStatusValid = true;
    evaluateAndShowAllWindows();
}

void SearchHintController::onOcrStatusChanged(const QString &state, const QString &grade)
{
    fmDebug() << "OCR index status changed:" << state << "grade:" << grade;
    if (m_ocrStatusValid && m_ocrState != state && m_ocrState != "Running" && state == "Running") {
        for (auto it = windowStates.begin(); it != windowStates.end(); ++it)
            it.value().dismissedTypes.clear();
    }
    m_ocrState = state;
    m_ocrGrade = grade;
    m_ocrStatusValid = true;
    evaluateAndShowAllWindows();
}

void SearchHintController::evaluateAndShowAllWindows()
{
    for (quint64 id : FMWindowsIns.windowIdList()) {
        auto *window = FMWindowsIns.findWindowById(id);
        if (window && SearchHelper::isSearchFile(window->currentUrl()))
            evaluateAndShow(id);
    }
}

void SearchHintController::evaluateAndShow(quint64 winId)
{
    HintType newType = evaluateHint(winId);
    auto &ws = windowStates[winId];

    if (newType == HintType::None) {
        if (hints.value(winId)) {
            hints[winId]->close();
            hints[winId] = nullptr;
        }
        ws.currentType = HintType::None;
        ws.currentText.clear();
        return;
    }

    QVariantMap content = buildHintContent(winId, newType);
    QString newText = content.value(QStringLiteral("text")).toString();

    if (newType == ws.currentType && newText == ws.currentText)
        return;

    showHintToWindow(winId, newType, content);
    ws.currentType = newType;
    ws.currentText = newText;
}

SearchHintController::HintType SearchHintController::evaluateHint(quint64 winId) const
{
    const auto &ws = windowStates.value(winId);

    // 1. Auth hint (highest priority)
    if (!ws.dismissedTypes.contains(HintType::AuthHint) && shouldShowAuthHint(nullptr))
        return HintType::AuthHint;

    // Only show index hints when file index search is enabled
    const QString &cfg = DConfig::kSearchCfgPath;
    if (!DConfigManager::instance()->value(cfg, DConfig::kEnableFileIndexSearch, false).toBool())
        return HintType::None;

    // 2. Failed (highest among index hints)
    if (!ws.dismissedTypes.contains(HintType::IndexFailed)) {
        if ((m_textStatusValid && m_textState == "Failed")
            || (m_ocrStatusValid && m_ocrState == "Failed"))
            return HintType::IndexFailed;
    }

    // 3. Paused (battery > power save > idle > upgrade)
    if (!ws.dismissedTypes.contains(HintType::IndexPausedBattery)) {
        if ((m_textStatusValid && m_textState == "WaitingPower")
            || (m_ocrStatusValid && m_ocrState == "WaitingPower"))
            return HintType::IndexPausedBattery;
    }

    if (!ws.dismissedTypes.contains(HintType::IndexPausedPowerSave)) {
        if ((m_textStatusValid && m_textState == "WaitingPowerSave")
            || (m_ocrStatusValid && m_ocrState == "WaitingPowerSave"))
            return HintType::IndexPausedPowerSave;
    }

    if (!ws.dismissedTypes.contains(HintType::IndexPausedIdle)) {
        if ((m_textStatusValid && m_textState == "WaitingIdle")
            || (m_ocrStatusValid && m_ocrState == "WaitingIdle"))
            return HintType::IndexPausedIdle;
    }

    if (!ws.dismissedTypes.contains(HintType::IndexWaitingUpgrade)) {
        if ((m_textStatusValid && m_textState == "WaitingUpgrade")
            || (m_ocrStatusValid && m_ocrState == "WaitingUpgrade"))
            return HintType::IndexWaitingUpgrade;
    }

    // 5. Updating
    if (!ws.dismissedTypes.contains(HintType::IndexUpdating)) {
        if ((m_textStatusValid && m_textState == "Running")
            || (m_ocrStatusValid && m_ocrState == "Running"))
            return HintType::IndexUpdating;
    }

    return HintType::None;
}

QVariantMap SearchHintController::buildHintContent(quint64 winId, HintType type) const
{
    QVariantMap content;

    QVariantList actions;

    switch (type) {
    case HintType::AuthHint: {
        content[QStringLiteral("icon")] = QStringLiteral("dfm-search-tips");
        QString text;
        shouldShowAuthHint(&text);
        content[QStringLiteral("text")] = text;
        actions.append(QVariantMap { { "id", "authorize" }, { "label", tr("Enable") } });
        break;
    }
    case HintType::IndexFailed:
        content[QStringLiteral("icon")] = QStringLiteral("warning");
        content[QStringLiteral("text")] = tr("Some index updates failed. Search results may be incomplete.");
        actions.append(QVariantMap { { "id", "retry-update" }, { "label", tr("Retry update") } });
        actions.append(QVariantMap { { "id", "view-status" }, { "label", tr("View") } });
        break;
    case HintType::IndexPausedBattery:
        content[QStringLiteral("icon")] = QStringLiteral("waiting");
        content[QStringLiteral("text")] = tr("Running on battery. Some content indexing has been paused.");
        actions.append(QVariantMap { { "id", "continue-update" }, { "label", tr("Continue updating") } });
        actions.append(QVariantMap { { "id", "view-status" }, { "label", tr("View") } });
        break;
    case HintType::IndexPausedPowerSave:
        content[QStringLiteral("icon")] = QStringLiteral("waiting");
        content[QStringLiteral("text")] = tr("Power save mode is enabled. Some content indexing has been paused.");
        actions.append(QVariantMap { { "id", "continue-update" }, { "label", tr("Continue updating") } });
        actions.append(QVariantMap { { "id", "view-status" }, { "label", tr("View") } });
        break;
    case HintType::IndexPausedIdle:
        content[QStringLiteral("icon")] = QStringLiteral("waiting");
        content[QStringLiteral("text")] = tr("Waiting for the device to become idle. Some content indexing has been paused.");
        actions.append(QVariantMap { { "id", "continue-update" }, { "label", tr("Continue updating") } });
        actions.append(QVariantMap { { "id", "view-status" }, { "label", tr("View") } });
        break;
    case HintType::IndexWaitingUpgrade:
        content[QStringLiteral("icon")] = QStringLiteral("waiting");
        content[QStringLiteral("text")] = tr("Waiting for index service upgrade. Some content indexing has been paused.");
        actions.append(QVariantMap { { "id", "retry-update" }, { "label", tr("Update index now") } });
        actions.append(QVariantMap { { "id", "view-status" }, { "label", tr("View") } });
        break;
    case HintType::IndexUpdating:
        content[QStringLiteral("icon")] = QString();
        content[QStringLiteral("text")] = updatingHintText();
        actions.append(QVariantMap { { "id", "view-status" }, { "label", tr("View") } });
        break;
    default:
        break;
    }

    content[QStringLiteral("actions")] = actions;
    return content;
}

QString SearchHintController::updatingHintText() const
{
    bool textReady = m_textStatusValid && m_textState == "Idle";
    bool ocrReady = m_ocrStatusValid && m_ocrState == "Idle";

    if (textReady && !ocrReady)
        return tr("Index is being updated. File name and file content search are available.");
    if (!textReady && ocrReady)
        return tr("Index is being updated. File name and image content search are available.");
    return tr("Index is being updated. File name search is available.");
}

void SearchHintController::showHintToWindow(quint64 winId, HintType type, const QVariantMap &content)
{
    if (hints.value(winId)) {
        hints[winId]->close();
        hints[winId] = nullptr;
    }

    auto obj = dpfSlotChannel->push("dfmplugin_workspace", "slot_ShowViewHint",
                                    winId, content)
                       .value<QObject *>();
    auto *hint = qobject_cast<DFMBASE_NAMESPACE::ViewHintMessage *>(obj);
    if (hint) {
        if (type == HintType::IndexUpdating) {
            DFMBASE_NAMESPACE::ViewHintCustomWidgetFactory spinnerFactory =
                    [](QWidget *parent) -> QWidget * {
                auto *spinner = new DTK_WIDGET_NAMESPACE::DSpinner(parent);
                spinner->setFixedSize(16, 16);
                spinner->start();
                return spinner;
            };
            hint->setCustomWidgetFactory(spinnerFactory,
                                         DFMBASE_NAMESPACE::ViewHintMessage::Side::Left);
        }

        hints[winId] = hint;
        connect(hint, &DFMBASE_NAMESPACE::ViewHintMessage::actionTriggered,
                this, [this, winId](const QString &id) {
                    onHintAction(winId, id);
                });
    }
}

void SearchHintController::onHintAction(quint64 winId, const QString &id)
{
    auto &ws = windowStates[winId];
    const HintType actedType = ws.currentType;

    if (id == "authorize") {
        authorizeSearchExperience();
        dismissAndReevaluate(winId, actedType);
    } else if (id == "close") {
        dismissAndReevaluate(winId, actedType);
    } else if (id == "continue-update" || id == "retry-update") {
        requestUpdate(actedType);
        dismissAndReevaluate(winId, actedType);
    } else if (id == "view-status") {
        openSettingsPage(winId);
        dismissAndReevaluate(winId, actedType);
    }
}

void SearchHintController::dismissAndReevaluate(quint64 winId, HintType actedType)
{
    if (actedType == HintType::AuthHint)
        dismissAuthHint();

    auto &ws = windowStates[winId];
    ws.dismissedTypes.insert(actedType);
    ws.currentType = HintType::None;
    ws.currentText.clear();
    QTimer::singleShot(0, this, [this, winId] { evaluateAndShow(winId); });
}

void SearchHintController::openSettingsPage(quint64 winId) const
{
    dpfSignalDispatcher->publish(GlobalEventType::kShowSettingDialog, winId, QString(SEARCH_SETTING_GROUP));
}

void SearchHintController::requestUpdate(HintType type) const
{
    // Only update the index that is actually in the state matching the hint type.
    QString targetState;
    bool force = false;
    switch (type) {
    case HintType::IndexPausedBattery:
        targetState = QStringLiteral("WaitingPower");
        break;
    case HintType::IndexPausedPowerSave:
        targetState = QStringLiteral("WaitingPowerSave");
        break;
    case HintType::IndexPausedIdle:
        targetState = QStringLiteral("WaitingIdle");
        break;
    case HintType::IndexFailed:
        targetState = QStringLiteral("Failed");
        force = true;
        break;
    case HintType::IndexWaitingUpgrade:
        targetState = QStringLiteral("WaitingUpgrade");
        force = true;
        break;
    default:
        return;   // No update action for other hint types
    }

    const QStringList &paths = DFMSEARCH::Global::defaultIndexedDirectory();
    fmInfo() << "User requested" << (force ? "force update" : "continue update (bypass env)")
             << "for" << paths.size() << "paths, type:" << static_cast<int>(type);

    auto applyToClient = [force, &paths](AbstractIndexClient *client) {
        if (force)
            client->forceUpdateIndex(paths);
        else
            client->updateIndexBypassEnv(paths);
    };

    if (m_textStatusValid && m_textState == targetState)
        applyToClient(TextIndexClient::instance());
    if (m_ocrStatusValid && m_ocrState == targetState)
        applyToClient(OcrIndexClient::instance());
}

QStringList SearchHintController::disabledSearchModes() const
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
