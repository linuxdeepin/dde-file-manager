// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/templatemenu_p.h"

#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <dfm-io/dfmio_utils.h>

#include <DDesktopEntry>

#include <QDir>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TemplateMenuPrivate::TemplateMenuPrivate(TemplateMenu *qq)
    : q(qq)
{
}

TemplateMenuPrivate::~TemplateMenuPrivate()
{
}

void TemplateMenuPrivate::loadTemplatePaths()
{
    USING_IO_NAMESPACE
    DCORE_USE_NAMESPACE

    // blumia: Following is support for `xdg-user-dir TEMPLATES` Templates folder.
    //         It's suppored by many GNOME Nautilus based file manager. I don't think it's a good idea
    //         since we can't localization the file name text at all.
    // blumia: templateFolderPathCStr owned by glib, should NOT be freeed.

    templateFolderPath = DFMUtils::userSpecialDir(DGlibUserDirectory::kUserDirectoryTemplates);
    QStringList templateFilePaths;
    templateFilePaths << templateFolderPath;

    // blumia: Following is support for `kf5-config --path templates` Templates folder.
    templateFilePaths = templateFilePaths + DFMUtils::systemDataDirs();
    templateFilePaths << DFMUtils::userDataDir();
    if (templateFilePaths.isEmpty())
        return;

    for (const QString &oneFolderPath : templateFilePaths) {
        if (oneFolderPath.isEmpty())
            continue;

        bool isDesktopEntryFolderPath = (oneFolderPath != templateFolderPath);
        QString tempFolderPath = isDesktopEntryFolderPath
                ? DFMUtils::buildFilePath(oneFolderPath.toLocal8Bit(), "/templates", nullptr)
                : oneFolderPath;

        traverseFolderToCreateActions(tempFolderPath, isDesktopEntryFolderPath);
    }
}

void TemplateMenuPrivate::createActionByNormalFile(const QString &path)
{
    if (path.isEmpty())
        return;

    QString errString;
    auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(QUrl::fromLocalFile(path), Global::CreateFileInfoType::kCreateFileInfoSync, &errString);
    if (!fileInfo) {
        qInfo() << "createActionByDesktopFile create FileInfo error: " << errString << path;
        return;
    }

    const QString &entryText = fileInfo->nameOf(FileInfo::FileNameInfoType::kCompleteBaseName);
    const QIcon &icon = fileInfo->fileIcon();
    QAction *action = new QAction(icon, entryText, Q_NULLPTR);
    action->setData(QVariant::fromValue(path));
    templateActions << action;
}

void TemplateMenuPrivate::createActionByDesktopFile(const QDir &dir, const QString &path)
{
    DCORE_USE_NAMESPACE
    if (!dir.exists() || path.isEmpty())
        return;

    DDesktopEntry desktopFile(path);
    const QString &entrySourcePath = dir.absoluteFilePath(desktopFile.stringValue("URL"));
    const QString &entryText = desktopFile.localizedValue("Name");

    QString errString;
    auto itemInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(entrySourcePath), Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
    if (Q_UNLIKELY(!itemInfo)) {
        qInfo() << "createActionByDesktopFile create FileInfo error: " << errString << entrySourcePath;
        return;
    }

    qDebug() << "desktop path: " << path << "URL: " << entrySourcePath;
    const QIcon &icon = QIcon::fromTheme(desktopFile.stringValue("Icon"));
    QAction *action = new QAction(icon, entryText, Q_NULLPTR);
    action->setData(QVariant::fromValue(entrySourcePath));
    templateActions << action;
}

void TemplateMenuPrivate::traverseFolderToCreateActions(const QString &path, bool val)
{
    if (path.isEmpty())
        return;

    QDir templateFolder(path);
    if (templateFolder.exists() && (val ? (templateFolder != QDir::home()) : true)) {
        const QStringList &templateFileList = val ? templateFolder.entryList(QStringList(QStringLiteral("*.desktop")),
                                                                             QDir::Files | QDir::Readable | QDir::NoSymLinks)
                                                  : templateFolder.entryList(QDir::Files | QDir::Readable | QDir::NoSymLinks);

        if (templateFileList.isEmpty())
            return;

        for (const auto &fileName : templateFileList)
            val ? createActionByDesktopFile(templateFolder, templateFolder.absoluteFilePath(fileName))
                : createActionByNormalFile(templateFolder.absoluteFilePath(fileName));
    }
}

TemplateMenu::TemplateMenu(QObject *parent)
    : QObject(parent), d(new TemplateMenuPrivate(this))
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
}

TemplateMenu::~TemplateMenu()
{
}

void TemplateMenu::loadTemplateFile()
{
    d->loadTemplatePaths();
}

QList<QAction *> TemplateMenu::actionList()
{
    return d->templateActions;
}
