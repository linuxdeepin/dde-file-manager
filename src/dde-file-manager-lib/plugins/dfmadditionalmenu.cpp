/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Mike Chen <kegechen@gmail.com>
 *
 * Maintainer: Mike Chen <chenke_cm@deepin.com>
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
#include "dfmadditionalmenu_p.h"
#include "dfmadditionalmenu.h"
#include "dfileservices.h"
#include "controllers/vaultcontroller.h"
#include "shutil/fileutils.h"

#include <QDir>
#include <QMenu>
#include <QMimeDatabase>
#include <QTimer>
#include <XdgDesktopFile>
#include <dabstractfilewatcher.h>
#include "interfaces/dfilemenu.h"
DFM_BEGIN_NAMESPACE

#define MENUEXTENSIONS_PATH  "/usr/share/deepin/dde-file-manager/oem-menuextensions/"

#define SET_PROPERTY_IFEXIST(action, file, key)\
    do{\
        QStringList values = d->getValues(file, key);\
        if (file.contains(key)) {\
            action->setProperty(key.data(), values);\
        }\
    }while(false)

#define MIME_TYPE_KEY "MimeType"

DFMAdditionalMenuPrivate::DFMAdditionalMenuPrivate(DFMAdditionalMenu *qq)
    : q_ptr(qq)
{
    m_delayedLoadFileTimer = new QTimer(qq);
    m_delayedLoadFileTimer->setSingleShot(true);
    m_delayedLoadFileTimer->setInterval(500);
    QObject::connect(m_delayedLoadFileTimer, &QTimer::timeout, qq, &DFMAdditionalMenu::loadDesktopFile);
    DUrl url = DUrl::fromLocalFile(MENUEXTENSIONS_PATH);
    DAbstractFileWatcher *dirWatch = DFileService::instance()->createFileWatcher(qq, url, qq);
    if (dirWatch) {
        dirWatch->startWatcher();
    }

    QObject::connect(dirWatch, &DAbstractFileWatcher::fileDeleted, m_delayedLoadFileTimer, [ = ]() {
        m_delayedLoadFileTimer->start();
    });

    QObject::connect(dirWatch, &DAbstractFileWatcher::subfileCreated, m_delayedLoadFileTimer, [ = ]() {
        m_delayedLoadFileTimer->start();
    });
}

QStringList DFMAdditionalMenuPrivate::getValues(XdgDesktopFile &file, const QLatin1String &key, const QStringList &whiteList)
{
    QStringList values;
    if (file.contains(key)) {
        values = file.value(key).toString().split(';', QString::SkipEmptyParts);
        for (const QString &one : values) {
            if (whiteList.size() > 0 && !whiteList.contains(one)) {
                values.removeAll(one);
            }
        }
    } else {
        values = whiteList;
    }
    return values;
}

bool DFMAdditionalMenuPrivate::isMimeTypeSupport(const QString &mt, const QStringList &fileMimeTypes)
{
    foreach (const QString &fmt, fileMimeTypes) {
        if (fmt.contains(mt, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

bool DFMAdditionalMenuPrivate::isMimeTypeMatch(const QStringList &fileMimeTypes, const QStringList &supportMimeTypes)
{
    bool match = false;
    for (QString mt : supportMimeTypes) {
        if (fileMimeTypes.contains(mt, Qt::CaseInsensitive)) {
            match = true;
            break;
        }

        int starPos = mt.indexOf("*");
        if (starPos >= 0 && isMimeTypeSupport(mt.left(starPos), fileMimeTypes)) {
            match = true;
            break;
        }
    }
    return match;
}

bool DFMAdditionalMenuPrivate::isActionShouldShow(QAction *action, bool onDesktop)
{
    if (!action) {
        return false;
    }

    // X-DFM-NotShowIn not exist
    if (!action->property(MENU_HIDDEN_KEY.data()).isValid()) {
        return true;
    }

    // is menu triggered on desktop
    QStringList notShowInList =  action->property(MENU_HIDDEN_KEY.data()).toStringList();
    return (onDesktop && !notShowInList.contains("Desktop", Qt::CaseInsensitive)) ||
           (!onDesktop && !notShowInList.contains("Filemanager", Qt::CaseInsensitive));
}

bool DFMAdditionalMenuPrivate::isSchemeSupport(QAction *action, const DUrl &url)
{
    // X-DFM-SupportSchemes not exist
    if (!action || !action->property(SUPPORT_SCHEMES_KEY.data()).isValid()) {
        return true;
    }
    QStringList supportList =  action->property(SUPPORT_SCHEMES_KEY.data()).toStringList();
    return supportList.contains(url.scheme(), Qt::CaseInsensitive);
}

bool DFMAdditionalMenuPrivate::isSuffixSupport(QAction *action, const DUrl &url, const bool ballEx7z)
{
    Q_Q(DFMAdditionalMenu);
    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(q, url);
    // X-DFM-SupportSuffix not exist
    if (!fileInfo || fileInfo->isDir() || !action || !action->property(SUPPORT_SUFFIX_KEY.data()).isValid()) {
        if (ballEx7z) {
            return false;
        }
        return true;
    }

    QFileInfo info(url.toLocalFile());
    QStringList supportList =  action->property(SUPPORT_SUFFIX_KEY.data()).toStringList();
    // 7z.001,7z.002, 7z.003 ... 7z.xxx
    QString cs = info.completeSuffix();
    if (supportList.contains(cs, Qt::CaseInsensitive)) {
        return true;
    }

    bool match = false;
    for (QString suffix : supportList) {
        int endPos = suffix.lastIndexOf("*"); // 7z.*
        if (endPos >= 0 && cs.length() > endPos && suffix.left(endPos) == cs.left(endPos)) {
            match = true;
            break;
        }
    }
    return match;
}

//都是7z分卷压缩文件
bool DFMAdditionalMenuPrivate::isAllEx7zFile(const QStringList &files)
{
    if (files.size() <= 1) {
        return false;
    }
    for (const QString &f : files) {
        QFileInfo info(f);
        // 7z.001,7z.002, 7z.003 ... 7z.xxx
        QString cs = info.completeSuffix();
        if (!cs.startsWith(QString("7z."))) {
            return false;
        }
    }
    return true;
}

QList<QAction *>DFMAdditionalMenuPrivate::emptyAreaActoins(const QString &currentDir, bool onDesktop)
{
    QString menuType = "EmptyArea";
    QList<QAction *> actions = actionListByType[menuType];
    for (auto it = actions.begin(); it != actions.end();) {
        QAction *action = *it;
        if (!action || !isActionShouldShow(action, onDesktop) ||
                !isSchemeSupport(action, DUrl(currentDir)) ||
                !isSuffixSupport(action, DUrl(currentDir))) {
            it = actions.erase(it);
            continue;
        }

        // Add file list data.
        action->setData(currentDir);
        if (action->menu()) {
            for (QAction *subAction : action->menu()->actions()) {
                subAction->setData(currentDir);
            }
        }

        ++it;
    }

    return actions;
}

DFMAdditionalMenuPrivate::~DFMAdditionalMenuPrivate()
{
    for (DFileMenu *menu : menuList) {
        delete menu;
        menu = nullptr;
    }
}

void DFMAdditionalMenu::loadDesktopFile()
{
    Q_D(DFMAdditionalMenu);
    qDebug() << d->actionList;

    if (d->menuActionHolder) {
        d->menuActionHolder->deleteLater();
    }
    for (DFileMenu *menu : d->menuList) {
        menu->deleteLater();
    }

    d->menuActionHolder = new QObject(this);

    d->actionList.clear();
    d->actionListByType.clear();

    QDir oemPath(MENUEXTENSIONS_PATH);
    if (oemPath.exists()) {
        for (const QFileInfo &fileInfo : oemPath.entryInfoList({"*.desktop"})) {
            XdgDesktopFile file;
            file.load(fileInfo.filePath());
            if (!file.isValid()) {
                continue;
            }

            QStringList menuTypes = d->getValues(file, d->MENU_TYPE_KEY, d->AllMenuTypes);
            if (!file.contains(d->MENU_TYPE_KEY)) {
                qDebug() << "[OEM Menu Support] Entry will probably not be shown due to empty or have no valid"
                         << d->MENU_TYPE_KEY << "key in the desktop file.";
                qDebug() << "[OEM Menu Support] Details:" << fileInfo.filePath() << "with entry name" << file.name();
            }

            // the XdgDesktopFile::icon() empty fallback is not really an empty fallback, so we need to check it manually.
            QString iconStr = file.localizedValue("Icon").toString();
            QAction *action = new QAction(iconStr.isEmpty() ? QIcon() : file.icon(), file.name(), d->menuActionHolder);
            QStringList entryActionList = file.actions();
            if (!entryActionList.isEmpty()) {
                DFileMenu *menu = new DFileMenu();
                d->menuList.append(menu);
                for (const QString &actionName : entryActionList) {
                    QAction *subAction = new QAction(file.actionIcon(actionName), file.actionName(actionName), d->menuActionHolder);
                    connect(subAction, &QAction::triggered, this, [subAction, actionName, file]() {
                        QStringList files = subAction->data().toStringList();
                        file.actionActivate(actionName, files);
                    });
                    menu->addAction(subAction);
                }
                action->setMenu(menu);
            }

            connect(action, &QAction::triggered, this, [action, file]() {
                QStringList files = action->data().toStringList();
                file.startDetached(files);
            });

            d->actionList.append(action);

            SET_PROPERTY_IFEXIST(action, file, QLatin1String(MIME_TYPE_KEY));
            SET_PROPERTY_IFEXIST(action, file, d->MENU_HIDDEN_KEY);
            SET_PROPERTY_IFEXIST(action, file, d->MIMETYPE_EXCLUDE_KEY);
            SET_PROPERTY_IFEXIST(action, file, d->SUPPORT_SCHEMES_KEY);
            SET_PROPERTY_IFEXIST(action, file, d->SUPPORT_SUFFIX_KEY);

            for (const QString &oneType : menuTypes) {
                d->actionListByType[oneType].append(action);
            }
        }
    }
}

DFMAdditionalMenu::DFMAdditionalMenu(QObject *parent)
    : QObject(parent)
    , d_private(new DFMAdditionalMenuPrivate(this))
{
    loadDesktopFile();
}

DFMAdditionalMenu::~DFMAdditionalMenu()
{

}

void DFMAdditionalMenu::appendParentMineType(const QStringList &parentmimeTypes,  QStringList &mimeTypes)
{
    if (parentmimeTypes.size() == 0)
        return;

    for (const QString &mtName : parentmimeTypes) {
        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForName(mtName);
        mimeTypes.append(mt.name());
        mimeTypes.append(mt.aliases());
        QStringList pmts = mt.parentMimeTypes();
        appendParentMineType(pmts, mimeTypes);
    }
}

QList<QAction *> DFMAdditionalMenu::actions(const QStringList &files, const QString &currentDir, bool onDesktop/* = false*/)
{
    Q_D(DFMAdditionalMenu);
    QString menuType = "EmptyArea";
    if (files.size() == 0) {
        return d->emptyAreaActoins(currentDir, onDesktop);
    }

    if (files.count() == 1) {
        menuType = QFileInfo(DUrl(files.first()).toLocalFile()).isDir() ? "SingleDir" : "SingleFile";
    } else {
        menuType = "MultiFileDirs";
    }
    QList<QAction *> actions = d->actionListByType[menuType];
    bool bex7z = d->isAllEx7zFile(files);
    for (const QString &f : files) {
        DAbstractFileInfoPointer fileInfo;
        DUrl url = DUrl(f);
        fileInfo = DFileService::instance()->createFileInfo(this, url);

        if (!fileInfo) {
            qWarning() << "createFileInfo failed: " << f;
            continue;
        }

        if (actions.size() == 0) {
            break;
        }

        QStringList fileMimeTypes, fmts;
        fileMimeTypes.append(fileInfo->mimeType().name());
        fileMimeTypes.append(fileInfo->mimeType().aliases());
        const QMimeType &mt = fileInfo->mimeType();
        fmts = fileMimeTypes;
        appendParentMineType(mt.parentMimeTypes(), fileMimeTypes);
        fileMimeTypes.removeAll({});
        fmts.removeAll({});

        for (auto it = actions.begin(); it != actions.end();) {
            QAction *action = *it;
            if (!action || !d->isActionShouldShow(action, onDesktop) ||
                    !d->isSchemeSupport(action, url) ||
                    !d->isSuffixSupport(action, url, bex7z)) {
                it = actions.erase(it);
                continue;
            }
            //fix bug 63917 ftp上不支持压缩，所以这里去掉压缩
            if (action->text() == QObject::tr("Compress") &&
                    FileUtils::isFtpFile(url)){
                it = actions.erase(it);
                continue;
            }

            // match exclude mime types
            QStringList excludeMimeTypes = action->property(d->MIMETYPE_EXCLUDE_KEY.data()).toStringList();
            excludeMimeTypes.removeAll({});
            // 排除时不包含 parentMimeTypes 不然容易误伤， 比如xlsx的 parentMimeTypes 是application/zip
            bool match = d->isMimeTypeMatch(fmts, excludeMimeTypes);
            if (match) {
                it = actions.erase(it);
                continue;
            }

            // MimeType not exist == MimeType=*
            if (!action->property(MIME_TYPE_KEY).isValid()) {
                ++it;
                continue;
            }

            // match support mime types
            QStringList supportMimeTypes =  action->property(MIME_TYPE_KEY).toStringList();
            supportMimeTypes.removeAll({});
            match = d->isMimeTypeMatch(fileMimeTypes, supportMimeTypes);

            //部分mtp挂载设备目录下文件属性不符合规范(普通目录mimetype被认为是octet-stream)，暂时做特殊处理
            if (url.path().contains("/mtp:host") && supportMimeTypes.contains("application/octet-stream") && fileMimeTypes.contains("application/octet-stream")) {
                match = false;
            }

            if (!match) {
                it = actions.erase(it);
                continue;
            }

            ++it;
        }
    }

    // Add file list data.
    for (QAction *action : actions) {
        action->setData(files);
        if (action->menu()) {
            for (QAction *subAction : action->menu()->actions()) {
                subAction->setData(files);
            }
        }
    }

    return actions;
}


DFM_END_NAMESPACE
