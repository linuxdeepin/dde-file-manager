// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processdialog.h"

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QProcess>

#include <unistd.h>
#include <signal.h>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)

DWIDGET_USE_NAMESPACE
using namespace dfm_upgrade;

ProcessDialog::ProcessDialog(QWidget *parent)
    : DDialog(parent)
{
}

void ProcessDialog::initialize(bool desktop)
{
    onDesktop = desktop;

    if (desktop)
        setMessage(tr("File Manager will be updated to a new version, during which the tasks in progress will be terminated. Do you want to update now?"));
    else
        setMessage(tr("The desktop services will be updated to a new version, during which the tasks in progress will be terminated. Do you want to update now?"));
    accept = addButton(tr("Update", "button"), true, DDialog::ButtonWarning);
    addButton(tr("Cancel", "button"));

    setIcon(QIcon::fromTheme("dde-file-manager"));
}

bool ProcessDialog::execDialog()
{
    const QString exe = onDesktop ? QString("/usr/libexec/dde-file-manager") : QString("/usr/bin/dde-shell");
    auto process = queryProcess(exe);
    if (process.isEmpty())
        return true;

    if (exec() == accept) {
        killAll(process);
        killed = true;
        return true;
    }
    return false;
}

void ProcessDialog::restart()
{
    if (killed && !onDesktop) {
        qCInfo(logToolUpgrade) << "restart dde-shell service...";
        QProcess::startDetached("systemctl", { "--user", "restart", "dde-shell-plugin@org.deepin.ds.desktop.service" });
    }
}

QList<int> ProcessDialog::queryProcess(const QString &exec)
{
    int currentUser = getuid();
    QList<int> pids;
    QDir dir("/proc");
    auto entryInfos = dir.entryInfoList(QDir::Dirs);
    for (auto info : entryInfos) {
        QString name = info.fileName();
        bool ok = false;
        int pid = name.toInt(&ok);
        if (!ok || pid < 0)
            continue;

        auto exePath = targetExe(info.absoluteFilePath());
        if (isEqual(exePath, exec)) {
            int tuid = targetUid(info.absoluteFilePath());
            if (tuid == currentUser) {
                qCInfo(logToolUpgrade) << "find active process:" << exePath << pid << "user" << tuid;
                pids.append(pid);
            } else {
                qCInfo(logToolUpgrade) << "find anthoer user's active process:" << exePath << pid << "user" << tuid << currentUser;
            }
        }
    }

    return pids;
}

void ProcessDialog::killAll(const QList<int> &pids)
{
    for (int pid : pids)
        kill(pid, SIGKILL);
}

QString ProcessDialog::targetExe(const QString &proc)
{
    QFileInfo exeFile(proc + "/exe");
#if (QT_VERSION <= QT_VERSION_CHECK(5, 13, 0))
    return exeFile.readLink();
#else
    return exeFile.symLinkTarget();
#endif
}

int ProcessDialog::targetUid(const QString &proc)
{
    int ret = -2;
    QFile loginUid(proc + "/loginuid");
    if (loginUid.open(QFile::ReadOnly)) {
        QString str = loginUid.readAll();
        bool ok = false;
        int tmp = str.toInt(&ok);
        if (ok)
            ret = tmp;
    }

    return ret;
}

bool ProcessDialog::isEqual(const QString &link, QString match) const
{
    if (link == match)
        return true;

    // the exePath will contain suffix (deleted) like /usr/bin/dde-desktop (deleted) after upgrading.
    //! It is not sure that the suffix '(deleted)' is stable.
    match.append(" (deleted)");
    if (link == match) {
        qCWarning(logToolUpgrade) << "unstable match:" << match;
        return true;
    }

    return false;
}
