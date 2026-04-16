// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkboxwidthfileindex.h"

#include "dfmplugin_search_global.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <unistd.h>

DPSEARCH_BEGIN_NAMESPACE

namespace {
constexpr int kPollIntervalMs = 3000;
constexpr int kCommandTimeoutMs = 3000;
const char kAnythingServiceName[] = "deepin-anything-daemon.service";
}

CheckBoxWidthFileIndex::CheckBoxWidthFileIndex(QWidget *parent)
    : IndexStatusCheckBox(parent)
{
    setInactiveText(tr("Enable to build the file index immediately for faster file name searches"));
    setIndexingTexts(tr("Updating index"), QString(), QString());

    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(kPollIntervalMs);

    connect(this, &IndexStatusCheckBox::checkStateChanged,
            this, &CheckBoxWidthFileIndex::handleCheckStateChanged);
    connect(this, &IndexStatusCheckBox::resetRequested, this, [this]() {
        if (!isChecked())
            return;

        setStatus(Status::Indexing);
        if (!restartFileIndex())
            fmWarning() << "[FileIndex] Failed to restart file index daemon";
        refreshState();
    });
    connect(m_pollTimer, &QTimer::timeout, this, &CheckBoxWidthFileIndex::refreshState);
}

void CheckBoxWidthFileIndex::initStatusBar()
{
    refreshState();
    m_pollTimer->start();
}

void CheckBoxWidthFileIndex::handleCheckStateChanged(Qt::CheckState state)
{
    if (m_syncingState || m_operationInProgress)
        return;

    m_operationInProgress = true;

    bool success = false;
    if (state == Qt::CheckState::Checked) {
        setStatus(Status::Indexing);
        success = enableFileIndex();
    } else {
        setStatus(Status::Inactive);
        success = disableFileIndex();
    }

    if (!success)
        fmWarning() << "[FileIndex] Failed to handle check state change:" << state;

    m_operationInProgress = false;
    refreshState();
}

void CheckBoxWidthFileIndex::refreshState()
{
    if (m_operationInProgress)
        return;

    applyState(queryState());
}

CheckBoxWidthFileIndex::FileIndexState CheckBoxWidthFileIndex::queryState() const
{
    FileIndexState state;

    const auto enabledResult = runSystemctlCommand({ "--user", "is-enabled", kAnythingServiceName });
    if (!enabledResult.started || !enabledResult.finished) {
        fmWarning() << "[FileIndex] Failed to query is-enabled";
        return state;
    }

    const QString enabledText = enabledResult.standardOutput.trimmed();
    if (enabledText == QStringLiteral("masked")) {
        state.querySuccess = true;
        state.enabled = false;
        return state;
    }

    if (enabledText != QStringLiteral("static")) {
        fmWarning() << "[FileIndex] Unexpected is-enabled output:" << enabledText
                    << "stderr:" << enabledResult.standardError.trimmed();
        return state;
    }

    state.enabled = true;

    const auto activeResult = runSystemctlCommand({ "--user", "is-active", kAnythingServiceName });
    if (!activeResult.started || !activeResult.finished) {
        fmWarning() << "[FileIndex] Failed to query is-active";
        return state;
    }

    state.serviceActive = (activeResult.standardOutput.trimmed() == QStringLiteral("active"));

    QFile statusFile(statusFilePath());
    if (statusFile.exists() && statusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const auto document = QJsonDocument::fromJson(statusFile.readAll());
        if (document.isObject()) {
            const auto object = document.object();
            state.status = object.value(QStringLiteral("status")).toString();
            state.lastUpdateTime = object.value(QStringLiteral("time")).toString();
        } else {
            fmWarning() << "[FileIndex] status.json is not a JSON object";
        }
    } else if (statusFile.exists()) {
        fmWarning() << "[FileIndex] Failed to read status file:" << statusFile.fileName();
    }

    state.querySuccess = true;
    return state;
}

void CheckBoxWidthFileIndex::applyState(const FileIndexState &state)
{
    if (!state.querySuccess) {
        if (isChecked()) {
            setStatus(Status::Failed);
            setFailedText(tr("Index update failed"), tr("try updating again"));
        } else {
            setStatus(Status::Inactive);
        }
        return;
    }

    m_syncingState = true;
    setChecked(state.enabled);
    m_syncingState = false;

    if (!state.enabled) {
        setStatus(Status::Inactive);
        return;
    }

    if (!state.serviceActive) {
        setStatus(Status::Failed);
        setFailedText(tr("Index update failed"), tr("try updating again"));
        return;
    }

    if (state.status == QStringLiteral("monitoring")) {
        setStatus(Status::Completed);
        setCompletedText(tr("Index update completed, last update time: %1").arg(formatDisplayTime(state.lastUpdateTime)),
                         tr("Update index now"));
        return;
    }

    setStatus(Status::Indexing);
}

bool CheckBoxWidthFileIndex::enableFileIndex()
{
    const auto unmaskResult = runSystemctlCommand({ "--user", "unmask", "deepin-anything-daemon" });
    if (!unmaskResult.started || !unmaskResult.finished || !unmaskResult.normalExit || unmaskResult.exitCode != 0) {
        fmWarning() << "[FileIndex] Failed to unmask service:" << unmaskResult.standardError.trimmed();
        return false;
    }

    if (!createRefreshIndexFile())
        return false;

    const auto startResult = runSystemctlCommand({ "--user", "start", "deepin-anything-daemon" });
    if (!startResult.started || !startResult.finished || !startResult.normalExit || startResult.exitCode != 0) {
        fmWarning() << "[FileIndex] Failed to start service:" << startResult.standardError.trimmed();
        return false;
    }

    return true;
}

bool CheckBoxWidthFileIndex::disableFileIndex()
{
    const auto stopResult = runSystemctlCommand({ "--user", "stop", "deepin-anything-daemon" });
    if (!stopResult.started || !stopResult.finished || !stopResult.normalExit || stopResult.exitCode != 0) {
        fmWarning() << "[FileIndex] Failed to stop service:" << stopResult.standardError.trimmed();
        return false;
    }

    const auto maskResult = runSystemctlCommand({ "--user", "mask", "deepin-anything-daemon" });
    if (!maskResult.started || !maskResult.finished || !maskResult.normalExit || maskResult.exitCode != 0) {
        fmWarning() << "[FileIndex] Failed to mask service:" << maskResult.standardError.trimmed();
        return false;
    }

    return true;
}

bool CheckBoxWidthFileIndex::restartFileIndex()
{
    if (!createRefreshIndexFile())
        return false;

    const auto restartResult = runSystemctlCommand({ "--user", "restart", "deepin-anything-daemon" });
    if (!restartResult.started || !restartResult.finished || !restartResult.normalExit || restartResult.exitCode != 0) {
        fmWarning() << "[FileIndex] Failed to restart service:" << restartResult.standardError.trimmed();
        return false;
    }

    return true;
}

bool CheckBoxWidthFileIndex::createRefreshIndexFile() const
{
    const QFileInfo fileInfo(refreshFilePath());
    QDir dir = fileInfo.dir();
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        fmWarning() << "[FileIndex] Failed to create refresh directory:" << dir.path();
        return false;
    }

    QFile refreshFile(fileInfo.filePath());
    if (!refreshFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        fmWarning() << "[FileIndex] Failed to create refresh file:" << refreshFile.fileName();
        return false;
    }

    refreshFile.close();
    return true;
}

CheckBoxWidthFileIndex::CommandResult CheckBoxWidthFileIndex::runSystemctlCommand(const QStringList &arguments) const
{
    CommandResult result;

    QProcess process;
    process.setProgram(QStringLiteral("systemctl"));
    process.setArguments(arguments);
    process.start();
    result.started = process.waitForStarted(kCommandTimeoutMs);
    if (!result.started) {
        fmWarning() << "[FileIndex] Failed to start command: systemctl" << arguments;
        return result;
    }

    result.finished = process.waitForFinished(kCommandTimeoutMs);
    if (!result.finished) {
        fmWarning() << "[FileIndex] Command timed out: systemctl" << arguments;
        process.kill();
        process.waitForFinished();
        return result;
    }

    result.exitCode = process.exitCode();
    result.normalExit = (process.exitStatus() == QProcess::NormalExit);
    result.standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
    result.standardError = QString::fromLocal8Bit(process.readAllStandardError());
    return result;
}

QString CheckBoxWidthFileIndex::statusFilePath() const
{
    return QStringLiteral("/run/user/%1/deepin-anything-server/status.json").arg(::getuid());
}

QString CheckBoxWidthFileIndex::refreshFilePath() const
{
    return QStringLiteral("/run/user/%1/deepin-anything-server/refresh_index").arg(::getuid());
}

QString CheckBoxWidthFileIndex::formatDisplayTime(const QString &isoTime) const
{
    const QDateTime parsedTime = QDateTime::fromString(isoTime, Qt::ISODate);
    if (parsedTime.isValid())
        return parsedTime.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));

    return QString(isoTime).replace(QLatin1Char('T'), QLatin1Char(' '));
}

DPSEARCH_END_NAMESPACE   // namespace dfmplugin_search
