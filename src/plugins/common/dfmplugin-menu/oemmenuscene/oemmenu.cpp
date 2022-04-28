/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "private/oemmenu_p.h"
#include "oemmenu.h"

#include "dfm-base/file/local/localfilewatcher.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"

#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMenu>
#include <QDebug>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DCORE_USE_NAMESPACE

static const char *const kDesktopEntryGroup = "Desktop Entry";
static const char *const kNameKey = "Name";
static const char *const kIconKey = "Icon";
static const char *const kActionsKey = "Actions";
static const char *const kTypeKey = "Type";
static const char *const kLocaleKey = "default";
static const char *const kActionsGroup = "Desktop Action ";
static const char *const kMimeType = "MimeType";
static const char *const kMenuTypeKey = "X-DDE-FileManager-MenuTypes";
static const char *const kMenuTypeAliasKey = "X-DFM-MenuTypes";
static const char *const kMimeTypeExcludeKey = "X-DDE-FileManager-ExcludeMimeTypes";
static const char *const kMimeTypeExcludeAliasKey = "X-DFM-ExcludeMimeTypes";
static const char *const kMenuHiddenKey = "X-DDE-FileManager-NotShowIn";   // "Desktop", "Filemanager"
static const char *const kMenuHiddenAliasKey = "X-DFM-NotShowIn";
static const char *const kSupportSchemesKey = "X-DDE-FileManager-SupportSchemes";
static const char *const kSupportSchemesAliasKey = "X-DFM-SupportSchemes";
static const char *const kSupportSuffixKey = "X-DDE-FileManager-SupportSuffix";   // for deepin-compress *.7z.001,*.7z.002,*.7z.003...
static const char *const kSupportSuffixAliasKey = "X-DFM-SupportSuffix";
static const char *const kDesktop = "Desktop";
static const char *const kFilemanager = "Filemanager";
static const char *const kEmptyArea = "EmptyArea";
static const char *const kSingleFile = "SingleFile";
static const char *const kSingleDir = "SingleDir";
static const char *const kMultiFileDirs = "MultiFileDirs";

class GlobalOemMenu : public OemMenu
{
};
Q_GLOBAL_STATIC(GlobalOemMenu, globalOemMenu)

OemMenuPrivate::OemMenuPrivate(OemMenu *qq)
    : q(qq)
{
    delayedLoadFileTimer.reset(new QTimer(q));
    delayedLoadFileTimer->setSingleShot(true);
    delayedLoadFileTimer->setInterval(500);

    QObject::connect(delayedLoadFileTimer.data(), &QTimer::timeout, q, &OemMenu::loadDesktopFile);

    oemMenuPath << QStringLiteral("/usr/etc/deepin/menu-extensions")
                << QStringLiteral("/etc/deepin/menu-extensions")
                << QStringLiteral("/usr/share/deepin/dde-file-manager/oem-menuextensions");

    menuTypes << kEmptyArea
              << kSingleFile
              << kSingleDir
              << kMultiFileDirs;

    actionProperties << kMimeType
                     << kMenuHiddenKey
                     << kMenuHiddenAliasKey
                     << kMimeTypeExcludeKey
                     << kMimeTypeExcludeAliasKey
                     << kSupportSchemesKey
                     << kSupportSchemesAliasKey
                     << kSupportSuffixKey
                     << kSupportSuffixAliasKey;

    for (auto path : oemMenuPath) {
        QUrl pathUrl = QUrl::fromLocalFile(path);
        auto watcher = new LocalFileWatcher(pathUrl, q);
        if (watcher) {
            QObject::connect(watcher, &LocalFileWatcher::fileDeleted, delayedLoadFileTimer.data(), [=]() {
                delayedLoadFileTimer->start();
            });
            QObject::connect(watcher, &LocalFileWatcher::subfileCreated, delayedLoadFileTimer.data(), [=]() {
                delayedLoadFileTimer->start();
            });

            watcher->startWatcher();
        }
    }
}

OemMenuPrivate::~OemMenuPrivate()
{
    clearSubMenus();
}

QStringList OemMenuPrivate::getValues(const DDesktopEntry &entry, const QString &key, const QString &aliasKey, const QString &section, const QStringList &whiteList) const
{
    QStringList values(whiteList);
    if (entry.contains(key, section) || entry.contains(aliasKey, section)) {
        values = entry.stringListValue(key, section) + entry.stringListValue(aliasKey, section);

        if (whiteList.isEmpty())
            return values;

        for (const QString &value : values) {
            if (!whiteList.contains(value))
                values.removeAll(value);
        }
    }

    return values;
}

bool OemMenuPrivate::isMimeTypeSupport(const QString &mt, const QStringList &fileMimeTypes) const
{
    for (auto fmt : fileMimeTypes) {
        if (fmt.contains(mt, Qt::CaseInsensitive))
            return true;
    }

    return false;
}

bool OemMenuPrivate::isMimeTypeMatch(const QStringList &fileMimeTypes, const QStringList &supportMimeTypes) const
{
    for (auto mt : supportMimeTypes) {
        if (fileMimeTypes.contains(mt, Qt::CaseInsensitive))
            return true;

        int index = mt.indexOf("*");
        if (index >= 0 && isMimeTypeSupport(mt.left(index), fileMimeTypes))
            return true;
    }

    return false;
}

bool OemMenuPrivate::isActionShouldShow(const QAction *action, bool onDesktop) const
{
    if (!action)
        return false;

    // X-DFM-NotShowIn not exist
    if (!action->property(kMenuHiddenKey).isValid() && !action->property(kMenuHiddenAliasKey).isValid())
        return true;

    QStringList notShowInList = action->property(kMenuHiddenKey).toStringList();
    notShowInList << action->property(kMenuHiddenAliasKey).toStringList();

    return (onDesktop && !notShowInList.contains(kDesktop, Qt::CaseInsensitive))
            || (!onDesktop && !notShowInList.contains(kFilemanager, Qt::CaseInsensitive));
}

bool OemMenuPrivate::isSchemeSupport(const QAction *action, const QUrl &url) const
{
    // X-DFM-SupportSchemes not exist
    if (!action || (!action->property(kSupportSchemesKey).isValid() && !action->property(kSupportSchemesAliasKey).isValid())) {
        return true;
    }

    QStringList supportList = action->property(kSupportSchemesKey).toStringList();
    supportList << action->property(kSupportSchemesAliasKey).toStringList();

    return supportList.contains(url.scheme(), Qt::CaseInsensitive);
}

bool OemMenuPrivate::isSuffixSupport(const QAction *action, const QUrl &url, const bool allEx7z) const
{
    QString errString;
    auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(url, true, &errString);

    // X-DFM-SupportSuffix not exist
    if (!fileInfo || fileInfo->isDir() || !action || (!action->property(kSupportSuffixKey).isValid() && !action->property(kSupportSuffixAliasKey).isValid())) {
        if (allEx7z) {
            return false;
        }
        return true;
    }

    QStringList supportList = action->property(kSupportSuffixKey).toStringList();
    supportList << action->property(kSupportSuffixAliasKey).toStringList();

    // 7z.001,7z.002, 7z.003 ... 7z.xxx
    QString cs = fileInfo->completeSuffix();
    if (supportList.contains(cs, Qt::CaseInsensitive)) {
        return true;
    }

    for (QString suffix : supportList) {
        int endPos = suffix.lastIndexOf("*");   // 7z.*
        if (endPos >= 0 && cs.length() > endPos && suffix.left(endPos) == cs.left(endPos)) {
            return true;
        }
    }

    return false;
}

bool OemMenuPrivate::isAllEx7zFile(const QList<QUrl> &files) const
{
    if (files.size() <= 1) {
        return false;
    }

    QString errString;
    for (const QUrl &f : files) {

        auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(f, true, &errString);
        if (fileInfo.isNull()) {
            qDebug() << errString;
            return false;
        }

        // 7z.001,7z.002, 7z.003 ... 7z.xxx
        QString cs = fileInfo->completeSuffix();
        if (!cs.startsWith(QString("7z."))) {
            return false;
        }
    }

    return true;
}

bool OemMenuPrivate::isValid(const QAction *action, const QUrl &url, const bool onDesktop, const bool allEx7z) const
{
    if (!action)
        return false;

    return isActionShouldShow(action, onDesktop) && isSchemeSupport(action, url) && isSuffixSupport(action, url, allEx7z);
}

void OemMenuPrivate::clearSubMenus()
{
    for (auto menu : subMenus) {
        menu->deleteLater();
        menu = nullptr;
    }
    subMenus.clear();
}

void OemMenuPrivate::setActionProperty(QAction *const action, const DDesktopEntry &entry, const QString &key, const QString &section) const
{
    if (!entry.contains(key, section))
        return;

    const QStringList &&values = entry.stringListValue(key, section);
    action->setProperty(key.toLatin1(), values);
}

void OemMenuPrivate::setActionData(QList<QAction *> actions, const QStringList &files) const
{
    if (actions.isEmpty() || files.isEmpty())
        return;

    for (auto action : actions) {
        setActionData(action, files);
    }
}

void OemMenuPrivate::setActionData(QAction *action, const QStringList &files) const
{
    if (!action)
        return;

    action->setData(files);
    if (action->menu()) {
        for (QAction *subAction : action->menu()->actions()) {
            subAction->setData(files);
        }
    }
}

void OemMenuPrivate::appendParentMineType(const QStringList &parentmimeTypes, QStringList &mimeTypes) const
{
    if (parentmimeTypes.isEmpty())
        return;

    const static QMimeDatabase db;
    for (const QString &mtName : parentmimeTypes) {
        QMimeType mt = db.mimeTypeForName(mtName);
        mimeTypes.append(mt.name());
        mimeTypes.append(mt.aliases());
        QStringList pmts = mt.parentMimeTypes();
        appendParentMineType(pmts, mimeTypes);
    }
}

OemMenu::OemMenu(QObject *parent)
    : QObject(parent), d(new OemMenuPrivate(this))
{
}

OemMenu::~OemMenu()
{
}

OemMenu *OemMenu::instance()
{
    return globalOemMenu;
}

void OemMenu::loadDesktopFile()
{
    d->menuActionHolder.reset(new QObject(this));
    d->actionListByType.clear();
    d->clearSubMenus();

    for (auto path : d->oemMenuPath) {
        QDir oemDir(path);
        if (!oemDir.exists())
            continue;

        for (const QFileInfo &fileInfo : oemDir.entryInfoList({ "*.desktop" })) {

            DDesktopEntry entry(fileInfo.absoluteFilePath());
            QStringList &&menuTypes = d->getValues(entry, kMenuTypeKey, kMenuTypeAliasKey, kDesktopEntryGroup, d->menuTypes);

            menuTypes.removeAll("");
            if (menuTypes.isEmpty()) {
                qDebug() << "[OEM Menu Support] Entry will probably not be shown due to empty or have no valid"
                         << kMenuTypeKey << " and " << kMenuTypeAliasKey << "key in the desktop file.";
                qDebug() << "[OEM Menu Support] Details:" << fileInfo.filePath() << "with entry name" << entry.localizedValue(kNameKey, kLocaleKey, kDesktopEntryGroup);
                continue;
            }

            QString &&iconStr = entry.localizedValue(kIconKey, kLocaleKey, kDesktopEntryGroup);
            QString &&nameStr = entry.localizedValue(kNameKey, kLocaleKey, kDesktopEntryGroup);
            QAction *action = new QAction(QIcon::fromTheme(iconStr), nameStr, d->menuActionHolder.data());

            for (auto propery : d->actionProperties) {
                d->setActionProperty(action, entry, propery, kDesktopEntryGroup);
            }

            for (const QString &type : menuTypes) {
                d->actionListByType[type].append(action);
            }

            // XdgDesktopFile file;
            //            connect(action, &QAction::triggered, this, [action, file]() {
            //                QStringList files = action->data().toStringList();
            //                file.startDetached(files);
            //            });

            // todo(wangcl):与自定义菜单一致，统一调用fileUtils中的接口

            // sub action
            QStringList &&entryActions = entry.stringListValue(kActionsKey, kDesktopEntryGroup);
            entryActions.removeAll("");
            if (!entryActions.isEmpty()) {

                QMenu *menu = new QMenu();
                d->subMenus.append(menu);

                for (const QString &actionName : entryActions) {
                    QString subGroupName(kActionsGroup + actionName);
                    QString &&subActionIconStr = entry.localizedValue(kIconKey, kLocaleKey, subGroupName);
                    QString &&subActionNameStr = entry.localizedValue(kNameKey, kLocaleKey, subGroupName);
                    QAction *subAction = new QAction(QIcon(subActionIconStr), subActionNameStr, d->menuActionHolder.data());

                    // XdgDesktopFile file;
                    //                    connect(subAction, &QAction::triggered, this, [subAction, actionName, file]() {
                    //                        QStringList files = subAction->data().toStringList();
                    //                        file.actionActivate(actionName, files);
                    //                    });

                    // todo(wangcl):与自定义菜单一致，统一调用fileUtils中的接口
                    //                    QString &&cmd = entry.stringValue(actionName, subGroupName);

                    menu->addAction(subAction);
                }

                action->setMenu(menu);
            }
        }
    }
}

QList<QAction *> OemMenu::emptyActions(const QUrl &currentDir, bool onDesktop)
{
    QList<QAction *> actions = d->actionListByType[kEmptyArea];

    auto it = actions.begin();
    while (it != actions.end()) {
        QAction *action = *it;
        if (!d->isValid(action, currentDir, onDesktop)) {
            it = actions.erase(it);
            continue;
        }

        d->setActionData(action, { currentDir.path() });

        ++it;
    }

    return actions;
}

QList<QAction *> OemMenu::normalActions(const QList<QUrl> &files, bool onDesktop)
{
    QString menuType;

    QString errString;
    if (1 == files.count()) {
        auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(files.first(), true, &errString);
        if (!fileInfo) {
            qDebug() << errString;
            return {};
        }

        menuType = fileInfo->isDir() ? kSingleDir : kSingleFile;
    } else {
        menuType = kMultiFileDirs;
    }

    QList<QAction *> actions = d->actionListByType[menuType];
    if (actions.isEmpty())
        return actions;

    QStringList filePaths;
    bool bex7z = d->isAllEx7zFile(files);
    for (const QUrl &file : files) {

        auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(file, true, &errString);

        if (!fileInfo) {
            qWarning() << "createFileInfo failed: " << file;
            continue;
        }
        filePaths << file.path();

        QStringList fileMimeTypes, fmts;
        fileMimeTypes.append(fileInfo->fileMimeType().name());
        fileMimeTypes.append(fileInfo->fileMimeType().aliases());
        const QMimeType &mt = fileInfo->fileMimeType();
        fmts = fileMimeTypes;
        d->appendParentMineType(mt.parentMimeTypes(), fileMimeTypes);
        fileMimeTypes.removeAll({});
        fmts.removeAll({});

        for (auto it = actions.begin(); it != actions.end();) {
            QAction *action = *it;
            if (!d->isValid(action, file, onDesktop, bex7z)) {
                it = actions.erase(it);
                continue;
            }

            // compression is not supported on FTP
            if (action->text() == QObject::tr("Compress") && FileUtils::isFtpPath(file)) {
                it = actions.erase(it);
                continue;
            }

            // match exclude mime types
            QStringList excludeMimeTypes = action->property(kMimeTypeExcludeKey).toStringList();
            excludeMimeTypes << action->property(kMimeTypeExcludeAliasKey).toStringList();

            excludeMimeTypes.removeAll({});
            // e.g. xlsx parentMimeTypes is application/zip
            bool match = d->isMimeTypeMatch(fmts, excludeMimeTypes);
            if (match) {
                it = actions.erase(it);
                continue;
            }

            // MimeType not exist == MimeType=*
            if (!action->property(kMimeType).isValid()) {
                ++it;
                continue;
            }

            // match support mime types
            QStringList supportMimeTypes = action->property(kMimeType).toStringList();
            supportMimeTypes.removeAll({});
            match = d->isMimeTypeMatch(fileMimeTypes, supportMimeTypes);

            //The file attributes of some MTP mounted device directories do not meet the specifications
            //(the ordinary directory mimeType is considered octet stream), so special treatment is required
            if (file.path().contains("/mtp:host") && supportMimeTypes.contains("application/octet-stream") && fileMimeTypes.contains("application/octet-stream")) {
                match = false;
            }

            if (!match) {
                it = actions.erase(it);
                continue;
            }

            ++it;
        }
    }

    d->setActionData(actions, filePaths);
    return actions;
}
