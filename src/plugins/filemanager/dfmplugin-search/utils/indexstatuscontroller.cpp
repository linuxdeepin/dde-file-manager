// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
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
        if (state == Qt::CheckState::Checked)
            m_view->setStatus(IndexStatusCheckBox::Status::Indexing);
        else
            m_view->setStatus(IndexStatusCheckBox::Status::Inactive);
    });

    connect(m_view, &IndexStatusCheckBox::resetRequested, this, [this]() {
        m_checkContext = CheckContext::ResetIndex;
        m_client->checkIndexExists();
    });

    connect(m_client, &AbstractIndexClient::indexExistsResult, this, [this](bool exists, bool success) {
        if (!success) {
            fmWarning() << "[" << m_options.logTag << "] Failed to check if index exists";
            return;
        }

        if (m_checkContext == CheckContext::ResetIndex) {
            m_client->setEnable(true);
            const auto taskType = exists ? AbstractIndexClient::TaskType::Update
                                         : AbstractIndexClient::TaskType::Create;
            m_client->startTask(taskType, DFMSEARCH::Global::defaultIndexedDirectory());
            m_view->setStatus(IndexStatusCheckBox::Status::Indexing);
        } else if (m_checkContext == CheckContext::InitStatus && m_view->isChecked()) {
            m_view->setStatus(exists ? IndexStatusCheckBox::Status::Completed
                                     : IndexStatusCheckBox::Status::Failed);
            if (exists)
                m_client->getLastUpdateTime();
            else
                m_view->setFailedText(m_options.failedMainText, m_options.failedLinkText);
        }

        m_checkContext = CheckContext::None;
    });

    connect(m_client, &AbstractIndexClient::lastUpdateTimeResult, this, [this](const QString &time, bool success) {
        if (success && !time.isEmpty()) {
            m_view->setCompletedText(m_options.completedMainText.arg(time),
                                     m_options.completedLinkText);
            return;
        }

        fmWarning() << "Failed to get" << m_options.logTag << "last update time, success:" << success;
    });

    connect(m_client, &AbstractIndexClient::hasRunningRootTaskResult, this, [this](bool running, bool success) {
        if (!success) {
            fmWarning() << "[" << m_options.logTag << "] Failed to check if root task is running";
            return;
        }

        if (!m_view->isChecked())
            return;

        if (running) {
            m_view->setStatus(IndexStatusCheckBox::Status::Indexing);
            return;
        }

        m_checkContext = CheckContext::InitStatus;
        m_client->checkIndexExists();
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

    connect(m_client, &AbstractIndexClient::taskProgressChanged, this,
            [this](AbstractIndexClient::TaskType type, const QString &path, qlonglong count, qlonglong total) {
                fmDebug() << m_options.logTag << "task progress changed - type:" << static_cast<int>(type)
                          << "path:" << path << "progress:" << count << "/" << total;
                if (!shouldHandleIndexEvent(path, type))
                    return;

                if (m_view->status() != IndexStatusCheckBox::Status::Indexing)
                    m_view->setStatus(IndexStatusCheckBox::Status::Indexing);
                m_view->updateIndexingProgress(count, total);
            });

    connect(m_client, &AbstractIndexClient::taskFinished, this,
            [this](AbstractIndexClient::TaskType type, const QString &path, bool success) {
                fmDebug() << m_options.logTag << "task finished - type:" << static_cast<int>(type)
                          << "path:" << path << "success:" << success;
                if (!shouldHandleIndexEvent(path, type))
                    return;

                if (success) {
                    m_view->setStatus(IndexStatusCheckBox::Status::Completed);
                    m_client->getLastUpdateTime();
                } else {
                    m_view->setStatus(IndexStatusCheckBox::Status::Failed);
                    m_view->setFailedText(m_options.failedMainText, m_options.failedLinkText);
                }
            });

    connect(m_client, &AbstractIndexClient::taskFailed, this,
            [this](AbstractIndexClient::TaskType type, const QString &path, const QString &error) {
                fmWarning() << m_options.logTag << "task failed - type:" << static_cast<int>(type)
                            << "path:" << path << "error:" << error;
                if (!shouldHandleIndexEvent(path, type))
                    return;

                m_view->setStatus(IndexStatusCheckBox::Status::Failed);
                m_view->setFailedText(m_options.failedMainText, m_options.failedLinkText);
            });
}

void IndexStatusController::initStatusBar()
{
    if (m_view->isChecked()) {
        m_client->checkHasRunningRootTask();
        return;
    }

    m_view->setStatus(IndexStatusCheckBox::Status::Inactive);
}

bool IndexStatusController::shouldHandleIndexEvent(const QString &path, AbstractIndexClient::TaskType type) const
{
    if (!m_view->isChecked())
        return false;

    if (!m_options.shouldHandleEvent)
        return true;

    return m_options.shouldHandleEvent(path, type);
}

}   // namespace dfmplugin_search
