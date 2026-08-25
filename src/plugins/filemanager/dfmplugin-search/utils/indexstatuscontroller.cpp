// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indexstatuscontroller.h"

#include <dfm-search/dsearch_global.h>

namespace dfmplugin_search {

IndexStatusController::IndexStatusController(IndexStatusCheckBox *view,
                                             AbstractIndexClient *client,
                                             const IndexStatusControllerOptions &options,
                                             QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_client(client)
    , m_options(options)
{
    m_view->setInactiveText(m_options.inactiveText);
    m_view->setIndexingTexts(m_options.indexingInitialText,
                             m_options.indexingFilesText,
                             m_options.indexingItemsText);

    connect(m_view, &IndexStatusCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        if (state == Qt::CheckState::Checked) {
            m_view->setStatus(IndexStatusCheckBox::Status::Indexing);
            m_client->getIndexStatus();
        } else {
            m_view->setStatus(IndexStatusCheckBox::Status::Inactive);
        }
    });

    connect(m_view, &IndexStatusCheckBox::resetRequested, this, [this](const QString &href) {
        const QStringList &paths = DFMSEARCH::Global::defaultIndexedDirectory();
        if (href == QLatin1String("manual")) {
            m_client->forceUpdateIndex(paths);
        } else {
            m_client->updateIndexBypassEnv(paths);
        }
    });

    // Server-driven status: initial query result
    connect(m_client, &AbstractIndexClient::indexStatusResult, this, [this](const QString &state, const QString &grade, bool success) {
        if (!success) {
            fmWarning() << "[" << m_options.logTag << "] Failed to get index status";
            return;
        }
        applyServerStatus(state);
    });

    // Server-driven status: real-time changes
    connect(m_client, &AbstractIndexClient::indexStatusChanged, this, [this](const QString &state, const QString &grade) {
        applyServerStatus(state);
    });

    connect(m_client, &AbstractIndexClient::lastUpdateTimeResult, this, [this](const QString &time, bool success) {
        if (success && !time.isEmpty()) {
            if (m_view->status() == IndexStatusCheckBox::Status::Completed)
                m_view->setCompletedText(m_options.completedMainText.arg(time),
                                         m_options.completedLinkText, QStringLiteral("manual"));
            return;
        }

        fmWarning() << "Failed to get" << m_options.logTag << "last update time, success:" << success;
    });
}

void IndexStatusController::syncCheckedState(bool enabled)
{
    fmInfo() << m_options.logTag << "enabled state changed to:" << enabled;
    m_view->setChecked(enabled);
}

void IndexStatusController::connectToBackend()
{
    m_client->checkServiceStatus();

    connect(m_client, &AbstractIndexClient::serviceStatusResult, this, [this](AbstractIndexClient::ServiceStatus status) {
        fmDebug() << m_options.logTag << "backend status:" << status;
    });

    // Progress only updates numbers, not status (status driven by indexStatusChanged)
    connect(m_client, &AbstractIndexClient::taskProgressChanged, this,
            [this](AbstractIndexClient::TaskType type, const QString &path, qlonglong count, qlonglong total) {
                if (!shouldHandleIndexEvent(path, type))
                    return;

                if (m_view->status() != IndexStatusCheckBox::Status::Indexing)
                    m_view->setStatus(IndexStatusCheckBox::Status::Indexing);
                m_view->updateIndexingProgress(count, total);
            });

    // Task finished → query authoritative status from server
    connect(m_client, &AbstractIndexClient::taskFinished, this,
            [this](AbstractIndexClient::TaskType type, const QString &path, bool success) {
                if (!shouldHandleIndexEvent(path, type))
                    return;
                m_client->getIndexStatus();
            });

    connect(m_client, &AbstractIndexClient::taskFailed, this,
            [this](AbstractIndexClient::TaskType type, const QString &path, const QString &error) {
                if (!shouldHandleIndexEvent(path, type))
                    return;
                m_client->getIndexStatus();
            });
}

void IndexStatusController::initStatusBar()
{
    if (!m_view->isChecked()) {
        m_view->setStatus(IndexStatusCheckBox::Status::Inactive);
        return;
    }

    m_client->getIndexStatus();
}

bool IndexStatusController::shouldHandleIndexEvent(const QString &path, AbstractIndexClient::TaskType type) const
{
    if (!m_view->isChecked())
        return false;

    if (!m_options.shouldHandleEvent)
        return true;

    return m_options.shouldHandleEvent(path, type);
}

void IndexStatusController::applyServerStatus(const QString &state)
{
    if (!m_view->isChecked())
        return;

    fmDebug() << "[" << m_options.logTag << "] applying server status:" << state;

    if (state == "Running") {
        m_view->setStatus(IndexStatusCheckBox::Status::Indexing);
    } else if (state == "Idle") {
        m_view->setStatus(IndexStatusCheckBox::Status::Completed);
        m_client->getLastUpdateTime();
    } else if (state == "Failed") {
        m_view->setStatus(IndexStatusCheckBox::Status::Failed);
        m_view->setFailedText(m_options.failedMainText, m_options.failedLinkText, QStringLiteral("manual"));
    } else if (state == "WaitingPower") {
        applyWaitingStatus(IndexStatusCheckBox::Status::WaitingPower);
    } else if (state == "WaitingPowerSave") {
        applyWaitingStatus(IndexStatusCheckBox::Status::WaitingPowerSave);
    } else if (state == "WaitingIdle") {
        applyWaitingStatus(IndexStatusCheckBox::Status::WaitingIdle);
    } else if (state == "WaitingUpgrade") {
        applyWaitingStatus(IndexStatusCheckBox::Status::WaitingUpgrade);
    } else {
        fmWarning() << "[" << m_options.logTag << "] unknown server status:" << state;
    }
}

void IndexStatusController::applyWaitingStatus(IndexStatusCheckBox::Status status)
{
    m_view->setStatus(status);

    QString mainText;
    QString linkText;
    QString href;

    switch (status) {
    case IndexStatusCheckBox::Status::WaitingPower:
        mainText = m_options.waitingPowerMainText;
        linkText = m_options.waitingUpdateLinkText;
        href = QStringLiteral("bypass");
        break;
    case IndexStatusCheckBox::Status::WaitingPowerSave:
        mainText = m_options.waitingPowerSaveMainText;
        linkText = m_options.waitingUpdateLinkText;
        href = QStringLiteral("bypass");
        break;
    case IndexStatusCheckBox::Status::WaitingIdle:
        mainText = m_options.waitingIdleMainText;
        linkText = m_options.waitingUpdateLinkText;
        href = QStringLiteral("bypass");
        break;
    case IndexStatusCheckBox::Status::WaitingUpgrade:
        mainText = m_options.waitingUpgradeMainText;
        linkText = m_options.waitingUpgradeLinkText;
        href = QStringLiteral("manual");
        break;
    default:
        return;
    }

    m_view->setWaitingText(mainText, linkText, href);
}

}   // namespace dfmplugin_search
