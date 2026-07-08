// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileindexcontroller.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QtConcurrent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QFutureWatcher>
#include <unistd.h>

DFMBASE_USE_NAMESPACE
DAEMONPCORE_BEGIN_NAMESPACE

namespace {
constexpr int kCommandTimeoutMs = 3000;
const char kAnythingServiceName[] = "deepin-anything-daemon";
const char kSearchCfgPath[] = "org.deepin.dde.file-manager.search";
const char kEnableFileIndexSearch[] = "enableFileIndexSearch";
}

FileIndexController::FileIndexController(QObject *parent)
    : QObject(parent)
{
}

FileIndexController::~FileIndexController() = default;

void FileIndexController::initialize()
{
    fmInfo() << "[FileIndexController] Initializing file index controller";

    QString err;
    if (!DConfigManager::instance()->addConfig(kSearchCfgPath, &err)) {
        fmWarning() << "[FileIndexController] Failed to register search config:" << err;
        return;
    }

    isConfigEnabled = DConfigManager::instance()->value(kSearchCfgPath, kEnableFileIndexSearch, true).toBool();
    fmInfo() << "[FileIndexController] Config registered, enabled:" << isConfigEnabled;

    scheduleApply();

    connect(DConfigManager::instance(), &DConfigManager::valueChanged,
            this, [this](const QString &config, const QString &key) {
                handleConfigChanged(config, key);
            });
}

void FileIndexController::handleConfigChanged(const QString &config, const QString &key)
{
    if (config != kSearchCfgPath || key != kEnableFileIndexSearch)
        return;

    const bool newEnabled = DConfigManager::instance()->value(config, key, true).toBool();
    if (newEnabled == isConfigEnabled)
        return;

    fmInfo() << "[FileIndexController] Config changed from" << isConfigEnabled << "to" << newEnabled;
    isConfigEnabled = newEnabled;

    scheduleApply();
}

void FileIndexController::scheduleApply()
{
    if (m_operationInProgress)
        return;

    m_operationInProgress = true;
    const bool enabled = isConfigEnabled;

    auto *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, enabled, watcher]() {
        m_operationInProgress = false;
        if (isConfigEnabled != enabled)
            scheduleApply();
        watcher->deleteLater();
    });
    watcher->setFuture(QtConcurrent::run([this, enabled]() {
        if (enabled)
            enableFileIndex();
        else
            disableFileIndex();
    }));
}

bool FileIndexController::enableFileIndex()
{
    if (isServiceActive() && isServiceEnabled()) {
        fmInfo() << "[FileIndexController] Service already active and enabled, skipping";
        return true;
    }

    if (!isServiceEnabled()) {
        if (!runSystemctlCommand({ "--user", "unmask", kAnythingServiceName })) {
            fmWarning() << "[FileIndexController] Failed to unmask service";
            return false;
        }
    }

    if (!createRefreshIndexFile())
        fmWarning() << "[FileIndexController] Failed to create refresh index file";

    if (!isServiceActive()) {
        if (!runSystemctlCommand({ "--user", "start", kAnythingServiceName })) {
            fmWarning() << "[FileIndexController] Failed to start service";
            return false;
        }
    }

    fmInfo() << "[FileIndexController] File index service enabled successfully";
    return true;
}

bool FileIndexController::disableFileIndex()
{
    if (!isServiceEnabled()) {
        fmInfo() << "[FileIndexController] Service already masked, skipping";
        return true;
    }

    if (isServiceActive()) {
        if (!runSystemctlCommand({ "--user", "stop", kAnythingServiceName })) {
            fmWarning() << "[FileIndexController] Failed to stop service";
            return false;
        }
    }

    if (!runSystemctlCommand({ "--user", "mask", kAnythingServiceName })) {
        fmWarning() << "[FileIndexController] Failed to mask service";
        return false;
    }

    fmInfo() << "[FileIndexController] File index service disabled successfully";
    return true;
}

bool FileIndexController::isServiceEnabled() const
{
    QProcess process;
    process.setProgram(QStringLiteral("systemctl"));
    process.setArguments({ "--user", "is-enabled", kAnythingServiceName });
    process.start();
    if (!process.waitForStarted(kCommandTimeoutMs) || !process.waitForFinished(kCommandTimeoutMs))
        return false;
    const QString output = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    return output == QStringLiteral("static") || output == QStringLiteral("enabled");
}

bool FileIndexController::isServiceActive() const
{
    QProcess process;
    process.setProgram(QStringLiteral("systemctl"));
    process.setArguments({ "--user", "is-active", kAnythingServiceName });
    process.start();
    if (!process.waitForStarted(kCommandTimeoutMs) || !process.waitForFinished(kCommandTimeoutMs))
        return false;
    return QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed() == QStringLiteral("active");
}

bool FileIndexController::runSystemctlCommand(const QStringList &arguments) const
{
    QProcess process;
    process.setProgram(QStringLiteral("systemctl"));
    process.setArguments(arguments);
    process.start();

    if (!process.waitForStarted(kCommandTimeoutMs)) {
        fmWarning() << "[FileIndexController] Failed to start command: systemctl" << arguments;
        return false;
    }

    if (!process.waitForFinished(kCommandTimeoutMs)) {
        fmWarning() << "[FileIndexController] Command timed out: systemctl" << arguments;
        process.kill();
        process.waitForFinished();
        return false;
    }

    if (process.exitCode() != 0) {
        fmWarning() << "[FileIndexController] Command failed:" << arguments
                     << "exit code:" << process.exitCode()
                     << "stderr:" << QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
        return false;
    }

    return true;
}

bool FileIndexController::createRefreshIndexFile() const
{
    const QFileInfo fileInfo(refreshFilePath());
    QDir dir = fileInfo.dir();
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        fmWarning() << "[FileIndexController] Failed to create refresh directory:" << dir.path();
        return false;
    }

    QFile refreshFile(fileInfo.filePath());
    if (!refreshFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        fmWarning() << "[FileIndexController] Failed to create refresh file:" << refreshFile.fileName();
        return false;
    }

    refreshFile.close();
    return true;
}

QString FileIndexController::refreshFilePath() const
{
    return QStringLiteral("/run/user/%1/deepin-anything-server/refresh_index").arg(::getuid());
}

DAEMONPCORE_END_NAMESPACE
