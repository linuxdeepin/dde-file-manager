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
#include <QStandardPaths>
#include <QRegularExpression>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

Q_LOGGING_CATEGORY(fileManagerPluginLog, "org.deepin.dde.shell.filemanager")

namespace dock {

static const QStringList s_colorThemeNames = {
    QStringLiteral("Ocean"),
    QStringLiteral("Sunset"),
    QStringLiteral("Aurora"),
    QStringLiteral("Neon"),
};

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

int FileManagerPlugin::gridCount() const
{
    return m_gridCount;
}

void FileManagerPlugin::setGridCount(int count)
{
    if (m_gridCount != count && count >= 1 && count <= 4) {
        m_gridCount = count;
        Q_EMIT gridCountChanged();
    }
}

int FileManagerPlugin::colorTheme() const
{
    return m_colorTheme;
}

void FileManagerPlugin::setColorTheme(int theme)
{
    if (m_colorTheme != theme && theme >= 0 && theme < s_colorThemeNames.size()) {
        m_colorTheme = theme;
        Q_EMIT colorThemeChanged();
    }
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

QStringList FileManagerPlugin::availableColorThemes() const
{
    return s_colorThemeNames;
}

void FileManagerPlugin::openFile(const QString &filePath)
{
    if (filePath.endsWith(QLatin1String(".desktop"))) {
        QSettings desktopFile(filePath, QSettings::IniFormat);
        desktopFile.beginGroup(QStringLiteral("Desktop Entry"));
        QString execLine = desktopFile.value(QStringLiteral("Exec")).toString();
        desktopFile.endGroup();

        if (!execLine.isEmpty()) {
            // Remove freedesktop field codes (%f, %F, %u, %U, etc.)
            execLine.remove(QRegularExpression(QStringLiteral(R"(%[fFuUdDnNickvm)")));

            QStringList parts = execLine.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (!parts.isEmpty()) {
                QString program = parts.takeFirst();

                // Resolve to absolute path if needed
                if (!program.startsWith(QLatin1Char('/'))) {
                    QString resolved = QStandardPaths::findExecutable(program);
                    if (!resolved.isEmpty())
                        program = resolved;
                }

                QString runId = QFileInfo(filePath).completeBaseName();

                QDBusMessage msg = QDBusMessage::createMethodCall(
                    QStringLiteral("org.desktopspec.ApplicationManager1"),
                    QStringLiteral("/org/desktopspec/ApplicationManager1"),
                    QStringLiteral("org.desktopspec.ApplicationManager1"),
                    QStringLiteral("executeCommand"));
                msg << program << parts
                    << QStringLiteral("portablebinary") << runId
                    << QVariant::fromValue(QMap<QString, QString>()) << QString();

                QDBusReply<QDBusObjectPath> reply = QDBusConnection::sessionBus().call(msg);
                if (reply.isValid())
                    return;

                qCWarning(fileManagerPluginLog) << "AM1 executeCommand failed:" << reply.error().message()
                                                 << "falling back to openUrl";
            }
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
