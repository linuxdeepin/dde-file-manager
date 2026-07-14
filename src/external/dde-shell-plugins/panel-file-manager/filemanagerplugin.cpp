// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filemanagerplugin.h"
#include <dde-shell/pluginfactory.h>

#include <QLoggingCategory>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>

Q_LOGGING_CATEGORY(fileManagerPluginLog, "org.deepin.dde.shell.filemanager")

namespace dock {

FileManagerPlugin::FileManagerPlugin(QObject *parent)
    : DApplet(parent)
    , m_directoryModel(new DirectoryModel(this))
{
}

bool FileManagerPlugin::init()
{
    DApplet::init();
    connect(m_directoryModel, &DirectoryModel::pathChanged, this, &FileManagerPlugin::directoryPathChanged);
    connect(m_directoryModel, &DirectoryModel::navigationChanged, this, &FileManagerPlugin::navigationChanged);
    connect(m_directoryModel, &DirectoryModel::countChanged, this, &FileManagerPlugin::folderCountChanged);
    connect(m_directoryModel, &DirectoryModel::countChanged, this, &FileManagerPlugin::previewIconNamesChanged);
    connect(m_directoryModel, &DirectoryModel::thumbnailChanged, this, &FileManagerPlugin::thumbnailChanged);

    // Auto-save path on navigation change
    connect(m_directoryModel, &DirectoryModel::pathChanged, this, [this]() {
        QSettings settings(QStringLiteral("deepin/dde-shell"), QStringLiteral("ds-panel-file-manager"));
        settings.setValue(QStringLiteral("lastPath"), m_directoryModel->path());
    });

    // Restore last path or default to home
    QSettings settings(QStringLiteral("deepin/dde-shell"), QStringLiteral("ds-panel-file-manager"));
    QString lastPath = settings.value(QStringLiteral("lastPath")).toString();
    if (lastPath.isEmpty() || !QDir(lastPath).exists())
        lastPath = QDir::homePath();
    m_directoryModel->navigateTo(lastPath);
    return true;
}

DirectoryModel *FileManagerPlugin::directoryModel() const
{
    return m_directoryModel;
}

int FileManagerPlugin::folderCount() const
{
    return m_directoryModel->folderCount();
}

int FileManagerPlugin::displayFolderCount() const
{
    return qMin(m_directoryModel->totalCount(), 4);
}

QString FileManagerPlugin::directoryPath() const
{
    return m_directoryModel->path();
}

int FileManagerPlugin::iconViewMode() const
{
    return m_iconViewMode;
}

void FileManagerPlugin::setIconViewMode(int mode)
{
    if (m_iconViewMode != mode && mode >= 0 && mode <= 1) {
        m_iconViewMode = mode;
        Q_EMIT iconViewModeChanged();
    }
}

QStringList FileManagerPlugin::previewIconNames() const
{
    QStringList names;
    const int count = qMin(m_directoryModel->rowCount(), 4);
    for (int i = 0; i < count; ++i) {
        auto entry = m_directoryModel->get(i);
        QString iconName = entry.value(QStringLiteral("iconName")).toString();
        if (!iconName.isEmpty())
            names << iconName;
        if (names.size() >= 4)
            break;
    }
    return names;
}

void FileManagerPlugin::openFile(const QString &filePath)
{
    if (filePath.endsWith(QLatin1String(".desktop"))) {
        QFileInfo desktopInfo(filePath);
        QString appId = desktopInfo.completeBaseName();

        // object path: /org.desktopspec/ApplicationManager1/{appId}
        QString objectPath = QStringLiteral("/org.desktopspec/ApplicationManager1/%1").arg(appId);

        QDBusInterface iface(
            QStringLiteral("org.desktopspec.ApplicationManager1"),
            objectPath,
            QStringLiteral("org.desktopspec.ApplicationManager1.Application"),
            QDBusConnection::sessionBus());

        if (iface.isValid()) {
            QVariantMap options { { QStringLiteral("_launch_type"), QStringLiteral("dde-file-manager") } };
            QDBusReply<void> reply = iface.call(QStringLiteral("Launch"),
                                                         filePath,
                                                         QStringList(),
                                                         options);
            if (reply.isValid())
                return;

            qCWarning(fileManagerPluginLog) << "AM1 Launch failed:" << reply.error().message()
                                                     << "falling back to openUrl";
        }
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

void FileManagerPlugin::refreshDirectory()
{
    m_directoryModel->refresh();
}

void FileManagerPlugin::navigateTo(const QString &path)
{
    m_directoryModel->navigateTo(path);
}

void FileManagerPlugin::goBack()
{
    m_directoryModel->goBack();
}

void FileManagerPlugin::goForward()
{
    m_directoryModel->goForward();
}

bool FileManagerPlugin::canGoBack() const
{
    return m_directoryModel->canGoBack();
}

bool FileManagerPlugin::canGoForward() const
{
    return m_directoryModel->canGoForward();
}

bool FileManagerPlugin::isDirectory(const QString &path) const
{
    return QFileInfo(path).isDir();
}

bool FileManagerPlugin::isFile(const QString &path) const
{
    return QFileInfo(path).isFile();
}

D_APPLET_CLASS(FileManagerPlugin)
}

#include "filemanagerplugin.moc"
