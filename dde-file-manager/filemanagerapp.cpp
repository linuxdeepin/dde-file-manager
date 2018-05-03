/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "filemanagerapp.h"
#include "app/filesignalmanager.h"
#include "dfmglobal.h"
#include "views/windowmanager.h"
#include "views/dfilemanagerwindow.h"

#include "controllers/appcontroller.h"
#include "controllers/pathmanager.h"
#include "app/define.h"

#include "dialogs/dialogmanager.h"

#include "models/fmstate.h"

#include "shutil/mimesappsmanager.h"
#include "interfaces/dfmstandardpaths.h"
#include "gvfs/gvfsmountmanager.h"
#include "interfaces/dfileinfo.h"

#include "singleton.h"
#include "commandlinemanager.h"
#include "interfaces/durl.h"
#include "plugins/pluginmanager.h"
#include "interfaces/dfmstandardpaths.h"
#include "models/desktopfileinfo.h"
#include "interfaces/dfileservices.h"
#include "interfaces/dfmsetting.h"
#include "shutil/fileutils.h"
#include "utils/utils.h"
#include "app/filesignalmanager.h"
#include "dabstractfilewatcher.h"

#include "tag/tagmanager.h"

#include <QDataStream>
#include <QGuiApplication>
#include <QTimer>
#include <QThreadPool>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QProcess>

class FileManagerAppGlobal : public FileManagerApp {};
Q_GLOBAL_STATIC(FileManagerAppGlobal, fmaGlobal)

FileManagerApp::FileManagerApp(QObject *parent) : QObject(parent)
{
    initApp();
    initView();
    lazyRunInitServiceTask();
    initSysPathWatcher();
    initConnect();
}

FileManagerApp *FileManagerApp::instance()
{
    return fmaGlobal;
}

FileManagerApp::~FileManagerApp()
{

}

void FileManagerApp::initApp()
{
    /*init configure*/
    DFMGlobal::initGlobalSettings();

    /*add menuextensions path*/
    DFMGlobal::autoLoadDefaultMenuExtensions();

    /*add plugin path*/
    DFMGlobal::autoLoadDefaultPlugins();

    /*init plugin manager */
    DFMGlobal::initPluginManager();

    /*init searchHistoryManager */
    DFMGlobal::initSearchHistoryManager();

    /*init bookmarkManager */
    DFMGlobal::initBookmarkManager();

    /*init fileMenuManager */
    DFMGlobal::initFileMenuManager();

    /*init fileSignalManger */
    DFMGlobal::initFileSiganlManager();

    /* init dialog manager */
    DFMGlobal::initDialogManager();

    /*init appController */
    DFMGlobal::initAppcontroller();

    /*init fileService */
    DFMGlobal::initFileService();

    /*init deviceListener */
    DFMGlobal::initDeviceListener();

    /*init mimeAppsManager*/
    DFMGlobal::initMimesAppsManager();

    /*init systemPathMnager */
    DFMGlobal::initSystemPathManager();

    /*init mimeTypeDisplayManager */
    DFMGlobal::initMimeTypeDisplayManager();

    /*init networkManager */
    DFMGlobal::initNetworkManager();

    /*init gvfsMountClient */
    DFMGlobal::initGvfsMountClient();

    /*init gvfsMountClient */
    DFMGlobal::initGvfsMountManager();
#ifdef AUTOMOUNT
    gvfsMountManager->setAutoMountSwitch(true);
#endif

    /*init secretManger */
    DFMGlobal::initSecretManager();

    /*init userShareManager */
    DFMGlobal::initUserShareManager();

    /*init controllers for different scheme*/
    fileService->initHandlersByCreators();

    /*init viewStatesManager*/
    DFMGlobal::initViewStatesManager();

    /*init operator revocation*/
    DFMGlobal::initOperatorRevocation();

    QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD_COUNT);
}

void FileManagerApp::initView()
{
    m_windowManager = WindowManager::instance();
}

void FileManagerApp::initManager()
{

}

void FileManagerApp::initTranslation()
{

}

void FileManagerApp::lazyRunInitServiceTask()
{
    QTimer::singleShot(1500, initService);
}

void FileManagerApp::initSysPathWatcher()
{
    m_sysPathWatcher = new QFileSystemWatcher(this);
    m_sysPathWatcher->addPath(QDir::homePath());
}

void FileManagerApp::initConnect()
{
    connect(m_sysPathWatcher, &QFileSystemWatcher::directoryChanged, systemPathManager, &PathManager::loadSystemPaths);

    connect(TagManager::instance(), static_cast<void(TagManager::*)(const QMap<QString, QString>&)>(&TagManager::changeTagColor),
            this, [this] (const QMap<QString, QString>& tag_and_new_color) {
        for (auto i = tag_and_new_color.constBegin(); i != tag_and_new_color.constEnd(); ++i) {
            const QString &tag_name = i.key();
            const QStringList &files = TagManager::instance()->getFilesThroughTag(tag_name);

            for (const QString &file : files) {
                DUrl url = DUrl::fromLocalFile(file);
                DAbstractFileWatcher::ghostSignal(url.parentUrl(), &DAbstractFileWatcher::fileAttributeChanged, url);
            }
        }
    });
    connect(TagManager::instance(), &TagManager::filesWereTagged, this, [this] (const QMap<QString, QList<QString>>& files_were_tagged) {
        for (auto i = files_were_tagged.constBegin(); i != files_were_tagged.constEnd(); ++i) {
            DUrl url = DUrl::fromLocalFile(i.key());
            DAbstractFileWatcher::ghostSignal(url.parentUrl(), &DAbstractFileWatcher::fileAttributeChanged, url);

            // for tag watcher
            for (const QString &tag : i.value()) {
                const DUrl &parent_url = DUrl::fromUserTaggedFile("/" + tag, QString());
                const DUrl &file_url = DUrl::fromUserTaggedFile("/" + tag, i.key());

                DAbstractFileWatcher::ghostSignal(parent_url, &DAbstractFileWatcher::subfileCreated, file_url);
            }
        }
    });
    connect(TagManager::instance(), &TagManager::untagFiles, this, [this] (const QMap<QString, QList<QString>>& tag_be_removed_files) {
        for (auto i = tag_be_removed_files.constBegin(); i != tag_be_removed_files.constEnd(); ++i) {
            DUrl url = DUrl::fromLocalFile(i.key());
            DAbstractFileWatcher::ghostSignal(url.parentUrl(), &DAbstractFileWatcher::fileAttributeChanged, url);

            // for tag watcher
            for (const QString &tag : i.value()) {
                const DUrl &parent_url = DUrl::fromUserTaggedFile("/" + tag, QString());
                const DUrl &file_url = DUrl::fromUserTaggedFile("/" + tag, i.key());

                DAbstractFileWatcher::ghostSignal(parent_url, &DAbstractFileWatcher::fileDeleted, file_url);
            }
        }
    });

    // for tag watcher
    connect(TagManager::instance(), &TagManager::addNewTag, this, [this] (const QList<QString>& new_tags) {
        for (const QString &tag : new_tags) {
            DAbstractFileWatcher::ghostSignal(DUrl(TAG_ROOT), &DAbstractFileWatcher::subfileCreated, DUrl::fromUserTaggedFile(tag, QString()));
        }
    });
    connect(TagManager::instance(), &TagManager::deleteTag, this, [this] (const QList<QString>& new_tags) {
        for (const QString &tag : new_tags) {
            DAbstractFileWatcher::ghostSignal(DUrl(TAG_ROOT), &DAbstractFileWatcher::fileDeleted, DUrl::fromUserTaggedFile(tag, QString()));
        }
    });
    connect(TagManager::instance(), static_cast<void(TagManager::*)(const QMap<QString, QString>&)>(&TagManager::changeTagName),
            this, [this] (const QMap<QString, QString>& old_and_new_name) {
        for (auto i = old_and_new_name.constBegin(); i != old_and_new_name.constEnd(); ++i) {
            const DUrl &old_url = DUrl::fromUserTaggedFile("/" + i.key(), QString());
            const DUrl &new_url = DUrl::fromUserTaggedFile("/" + i.value(), QString());

            DAbstractFileWatcher::ghostSignal(DUrl(TAG_ROOT), &DAbstractFileWatcher::fileMoved, old_url, new_url);
        }
    });
}

void FileManagerApp::initService()
{
    if (globalSetting->isCompressFilePreview()){
        FileUtils::mountAVFS();
    }
}

void FileManagerApp::show(const DUrl &url)
{
    m_windowManager->showNewWindow(url);
}

void FileManagerApp::showPropertyDialog(const QStringList paths)
{
    DUrlList urlList;
    foreach (QString path, paths) {
        DUrl url = DUrl::fromUserInput(path);
        QString uPath = url.path();
        if(uPath.endsWith(QDir::separator()) && uPath.size() > 1)
            uPath.chop(1);
        url.setPath(uPath);

        //symlink , desktop files filters
        const DAbstractFileInfoPointer& info = fileService->createFileInfo(this, url);
        DUrl realTargetUrl = url;
        if(info && info->isSymLink()){
            realTargetUrl = info->rootSymLinkTarget();
        }

        if(realTargetUrl.toLocalFile().endsWith(".desktop")){
            DesktopFile df(realTargetUrl.toLocalFile());
            if(df.getDeepinId() == "dde-trash"){
                DFMEvent event(this);
                event.setData(DUrl::fromTrashFile("/"));
                dialogManager->showTrashPropertyDialog(event);
                continue;
            } else if(df.getDeepinId() == "dde-computer"){
                dialogManager->showComputerPropertyDialog();
                continue;
            }
        }

        //trash:/// and computer:///
        if(url == DUrl::fromComputerFile("/")){
            dialogManager->showComputerPropertyDialog();
            continue;
        }
        if(url == DUrl::fromTrashFile("/")){
            DFMEvent event(this);
            event.setData(DUrl::fromTrashFile("/"));
            dialogManager->showTrashPropertyDialog(event);
            continue;
        }
        if (!url.scheme().isEmpty()){
            if(url.scheme() == FILE_SCHEME && !QFile::exists(url.path()))
                continue;
            if(url == DUrl::fromTrashFile("/") ||
                    url == DesktopFileInfo::trashDesktopFileUrl()){
                DFMEvent event(this);
                DUrlList urls;
                urls << url;
                event.setData(urls);
                emit fileSignalManager->requestShowTrashPropertyDialog(event);
                continue;
            }
        }else{
            if(!QFile::exists(DUrl::fromLocalFile(path).path()))
                continue;
        }

        if(urlList.contains(url))
            continue;
        urlList << url;
    }
    if(urlList.isEmpty())
        return;

    emit fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(this, urlList));
}

