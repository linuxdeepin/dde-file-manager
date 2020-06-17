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

#include "dfileservices.h"
#include "dabstractfilecontroller.h"
#include "dabstractfileinfo.h"
#include "dabstractfilewatcher.h"
#include "dfmeventdispatcher.h"
#include "dfmfilecontrollerfactory.h"
#include "dfiledevice.h"
#include "dfilehandler.h"
#include "dstorageinfo.h"

#include "app/filesignalmanager.h"
#include "dfmevent.h"
#include "app/define.h"
#include "controllers/jobcontroller.h"
#include "controllers/appcontroller.h"
#include "controllers/mergeddesktopcontroller.h"
#include "views/windowmanager.h"
#include "dfileinfo.h"
#include "models/trashfileinfo.h"
#include "models/desktopfileinfo.h"
#include "controllers/pathmanager.h"
#include "dfmstandardpaths.h"
#include "views/windowmanager.h"
#include "models/avfsfileinfo.h"

#include "shutil/fileutils.h"
#include "shutil/filebatchprocess.h"

#include "dialogs/dialogmanager.h"

#include "deviceinfo/udisklistener.h"
#include "interfaces/dfmglobal.h"
#include "singleton.h"
#include "models/dfmrootfileinfo.h"

#include <ddialog.h>

#include <QUrl>
#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QMimeData>
#include <QTimer>
#include <QStandardPaths>
#include <QNetworkConfigurationManager>
#include <QHostInfo>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

DWIDGET_USE_NAMESPACE

class DFileServicePrivate
{
public:

    static QMultiHash<const HandlerType, DAbstractFileController *> controllerHash;
    static QHash<const DAbstractFileController *, HandlerType> handlerHash;
    static QMultiHash<const HandlerType, HandlerCreatorType> controllerCreatorHash;
    static QList<DUrl> rootfilelist;
    bool bstartonce = false;
    bool m_bcursorbusy = false;
    bool m_bonline = false;
    bool m_bdoingcleartrash = false;
    JobController *m_jobcontroller = nullptr;
    QNetworkConfigurationManager *m_networkmgr = nullptr;
    QEventLoop *m_loop = nullptr;
    //fix bug,当快速点击左边侧边栏会出现鼠标一直在转圈圈
    QMutex m_mutexCursorState;
    QMutex m_mutexrootfilechange;
};

QMultiHash<const HandlerType, DAbstractFileController *> DFileServicePrivate::controllerHash;
QHash<const DAbstractFileController *, HandlerType> DFileServicePrivate::handlerHash;
QMultiHash<const HandlerType, HandlerCreatorType> DFileServicePrivate::controllerCreatorHash;
QList<DUrl> DFileServicePrivate::rootfilelist;//本地跟踪root目录，本地磁盘，外部磁盘挂载，网络文件挂载

DFileService::DFileService(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFileServicePrivate())
{
    /// init url handler register
    AppController::registerUrlHandle();
    // register plugins
    for (const QString &key : DFMFileControllerFactory::keys()) {
        const QUrl url(key);

        insertToCreatorHash(HandlerType(url.scheme(), url.host()), HandlerCreatorType(typeid(DFMFileControllerFactory).name(), [key] {
            return DFMFileControllerFactory::create(key);
        }));
    }
    //判断当前自己的网络状态
    d_ptr->m_networkmgr = new QNetworkConfigurationManager();
    d_ptr->m_bonline = d_ptr->m_networkmgr->isOnline();
    d_ptr->m_loop = new QEventLoop();
    connect(d_ptr->m_networkmgr, &QNetworkConfigurationManager::onlineStateChanged,[this](bool state){
        d_ptr->m_bonline = state;
        if (!d_ptr->m_bonline && d_ptr->m_loop)
        {
            d_ptr->m_loop->exit();
        }
    });

}

DFileService::~DFileService()
{

}

template<typename T>
QVariant eventProcess(DFileService *service, const QSharedPointer<DFMEvent> &event, T function)
{
    QSet<DAbstractFileController *> controller_set;
    QSet<QString> scheme_set;

    for (const DUrl &url : event->handleUrlList()) {
        QList<DAbstractFileController *> list = service->getHandlerTypeByUrl(url);
        if (!scheme_set.contains(url.scheme())) {
            list << service->getHandlerTypeByUrl(url, true);
        } else {
            scheme_set << url.scheme();
        }

        for (DAbstractFileController *controller : list) {
            if (controller_set.contains(controller)) {
                continue;
            }

            controller_set << controller;

            typedef typename std::remove_reference<typename QtPrivate::FunctionPointer<T>::Arguments::Car>::type::Type DFMEventType;

            const QVariant result = QVariant::fromValue((controller->*function)(event.staticCast<DFMEventType>()));

            if (event->isAccepted()) {
                return result;
            }
        }
    }

    return QVariant();
}

bool DFileService::fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData)
{
#ifdef DDE_COMPUTER_TRASH
    switch (event->type()) {
    case DFMEvent::WriteUrlsToClipboard:
    case DFMEvent::DeleteFiles:
    case DFMEvent::MoveToTrash: {
        DUrlList urlList = event->fileUrlList();

        if (urlList.contains(DesktopFileInfo::computerDesktopFileUrl())) {
            DesktopFile df(DesktopFileInfo::computerDesktopFileUrl().toLocalFile());
            if (df.getDeepinId() == "dde-computer") {
                urlList.removeOne(DesktopFileInfo::computerDesktopFileUrl());
            }
        }

        if (urlList.contains(DesktopFileInfo::trashDesktopFileUrl())) {
            DesktopFile df(DesktopFileInfo::trashDesktopFileUrl().toLocalFile());
            if (df.getDeepinId() == "dde-trash") {
                urlList.removeOne(DesktopFileInfo::trashDesktopFileUrl());
            }
        }

        event->setData(urlList);
        break;
    }
    default:
        break;
    }
#endif

#define CALL_CONTROLLER(Fun)\
    eventProcess(this, event, &DAbstractFileController::Fun);

    QVariant result;

    switch (event->type()) {
    case DFMEvent::OpenFile:
        result = CALL_CONTROLLER(openFile);

        if (result.toBool()) {
            emit fileOpened(event->fileUrl());
        }
        break;
    case DFMEvent::OpenFileByApp:
        result = CALL_CONTROLLER(openFileByApp);

        if (result.toBool()) {
            emit fileOpened(event->fileUrl());
        }

        break;
    case DFMEvent::CompressFiles:
        result = CALL_CONTROLLER(compressFiles);
        break;
    case DFMEvent::DecompressFile:
        result = CALL_CONTROLLER(decompressFile);
        break;
    case DFMEvent::DecompressFileHere:
        result = CALL_CONTROLLER(decompressFileHere);
        break;
    case DFMEvent::WriteUrlsToClipboard:
        result = CALL_CONTROLLER(writeFilesToClipboard);
        break;
    case DFMEvent::RenameFile: {
        const QSharedPointer<DFMRenameEvent> &e = event.staticCast<DFMRenameEvent>();
        const DAbstractFileInfoPointer &f = createFileInfo(this, e->toUrl());
        if (f && f->exists() && !e->silent()) {
            DThreadUtil::runInMainThread(dialogManager, &DialogManager::showRenameNameSameErrorDialog, f->fileDisplayName(), *event.data());
            result = false;
        } else {
            result = CALL_CONTROLLER(renameFile);

            if (result.toBool()) {
                emit fileRenamed(e->fromUrl(), e->toUrl());
            }
        }

        break;
    }
    case DFMEvent::DeleteFiles: {

        // 解决撤销操作后文件删除不提示问题
        for (const DUrl &url : event->fileUrlList()) {
            //书签保存已删除目录，在这里剔除对书签文件是否存在的判断
            if (url.scheme() == "bookmark") {
                result = CALL_CONTROLLER(deleteFiles);

                if (result.toBool()) {
                    for (const DUrl &url : event->fileUrlList()) {
                        emit fileDeleted(url);
                    }
                }
                break;
            }
            const DAbstractFileInfoPointer &f = createFileInfo(this, url);
            if (f && f->exists()) {
                static bool lock = false;
                if (!lock && DThreadUtil::runInMainThread(dialogManager, &DialogManager::showDeleteFilesClearTrashDialog, DFMUrlListBaseEvent(nullptr, event->fileUrlList())) == DDialog::Accepted) {
                    lock = true;
                    result = CALL_CONTROLLER(deleteFiles);
                    lock = false;
                    if (result.toBool()) {
                        for (const DUrl &url : event->fileUrlList()) {
                            emit fileDeleted(url);
                        }
                    }
                }

                if (lock) {
                    result = CALL_CONTROLLER(deleteFiles);
                    if (result.toBool()) {
                        for (const DUrl &url : event->fileUrlList()) {
                            emit fileDeleted(url);
                        }
                    }
                }
                else {
                    //fix bug 31324,判断当前操作是否是清空回收站，是就在结束时改变清空回收站状态
                    if (event->fileUrlList().count() == 1 && event->fileUrlList().first().toString().endsWith("trash:///")){
                        setDoClearTrashState(false);
                    }
                }

                break;
            } else {
                continue;
            }
        }


//        if (DThreadUtil::runInMainThread(dialogManager, &DialogManager::showDeleteFilesClearTrashDialog, DFMUrlListBaseEvent(nullptr, event->fileUrlList())) == DDialog::Accepted) {
//            result = CALL_CONTROLLER(deleteFiles);

//            if (result.toBool()) {
//                for (const DUrl &url : event->fileUrlList()) {
//                    emit fileDeleted(url);
//                }
//            }

//        }
        break;

    }
    case DFMEvent::MoveToTrash: {
        //handle system files should not be able to move to trash
        foreach (const DUrl &url, event->fileUrlList()) {
            if (systemPathManager->isSystemPath(url.toLocalFile())) {
                DThreadUtil::runInMainThread(dialogManager, &DialogManager::showDeleteSystemPathWarnDialog, event->windowId());
                result = QVariant::fromValue(event->fileUrlList());
                goto end;
            }
        }

        //handle files whom could not be moved to trash
//        DUrlList enableList;
//        DUrlList disableList;
//        foreach (const DUrl& url, event->fileUrlList()) {
//            const DAbstractFileInfoPointer& info = createFileInfo(this, url);
//            if(info->isDir() && !info->isWritable()){
//                disableList << url;
//                continue;
//            }
//            enableList << url;
//        }
//        if (!disableList.isEmpty()){
//            DFMUrlListBaseEvent noPermissionEvent{event->sender(), disableList};
//            noPermissionEvent.setWindowId(event->windowId());
//            emit fileSignalManager->requestShowNoPermissionDialog(noPermissionEvent);
//        }

//        event->setData(enableList);
        result = CALL_CONTROLLER(moveToTrash);

        const DUrlList &list = event->fileUrlList();
        const DUrlList new_list = qvariant_cast<DUrlList>(result);

        for (int i = 0; i < new_list.count(); ++i) {
            if (!new_list.at(i).isValid())
                continue;

            emit fileMovedToTrash(list.at(i), new_list.at(i));
            //        emit fileRenamed(list.at(i), result.at(i));
        }

        break;
    }
    case DFMEvent::RestoreFromTrash: {
        result = CALL_CONTROLLER(restoreFile);
        break;
    }
    case DFMEvent::PasteFile: {
        result = CALL_CONTROLLER(pasteFile);

        if (event->isAccepted()) {
            DFMUrlListBaseEvent e(event->sender(), qvariant_cast<DUrlList>(result));

            e.setWindowId(event->windowId());
            laterRequestSelectFiles(e);
        }

        const DUrlList &list = event->fileUrlList();
        const DUrlList new_list = qvariant_cast<DUrlList>(result);

        for (int i = 0; i < new_list.count(); ++i) {
            const DUrl &url = new_list.at(i);

            if (url.isEmpty())
                continue;

            DFMGlobal::ClipboardAction action = event.staticCast<DFMPasteEvent>()->action();

            if (action == DFMGlobal::ClipboardAction::CopyAction) {
                emit fileCopied(list.at(i), url);
            } else if (action == DFMGlobal::ClipboardAction::CutAction) {
                emit fileRenamed(list.at(i), url);
            }
        }

        break;
    }
    case DFMEvent::Mkdir:
        result = CALL_CONTROLLER(mkdir);
        break;
    case DFMEvent::TouchFile:
        result = CALL_CONTROLLER(touch);
        break;
    case DFMEvent::OpenFileLocation:
        result = CALL_CONTROLLER(openFileLocation);
        break;
    case DFMEvent::AddToBookmark:
        result = CALL_CONTROLLER(addToBookmark);
        break;
    case DFMEvent::RemoveBookmark:
        result = CALL_CONTROLLER(removeBookmark);
        break;
    case DFMEvent::CreateSymlink:
        result = CALL_CONTROLLER(createSymlink);
        break;
    case DFMEvent::FileShare:
        result = CALL_CONTROLLER(shareFolder);
        break;
    case DFMEvent::CancelFileShare:
        result = CALL_CONTROLLER(unShareFolder);
        break;
    case DFMEvent::OpenInTerminal:
        result = CALL_CONTROLLER(openInTerminal);
        break;
    case DFMEvent::GetChildrens:
        result = CALL_CONTROLLER(getChildren);
        break;
    case DFMEvent::CreateFileInfo:
        result = CALL_CONTROLLER(createFileInfo);
        break;
    case DFMEvent::CreateDiriterator:
        result = CALL_CONTROLLER(createDirIterator);
        break;
    case DFMEvent::CreateGetChildrensJob: {
        result = CALL_CONTROLLER(createDirIterator);

        const QSharedPointer<DFMCreateGetChildrensJob> &e = event.staticCast<DFMCreateGetChildrensJob>();

        if (event->isAccepted()) {
            result = QVariant::fromValue(new JobController(e->url(), qvariant_cast<DDirIteratorPointer>(result)));
        } else {
            result = QVariant::fromValue(new JobController(e->url(), e->nameFilters(), e->filters()));
        }

        break;
    }
    case DFMEvent::CreateFileWatcher:
        result = CALL_CONTROLLER(createFileWatcher);
        break;
    case DFMEvent::CreateFileDevice:
        result = CALL_CONTROLLER(createFileDevice);
        break;
    case DFMEvent::CreateFileHandler:
        result = CALL_CONTROLLER(createFileHandler);
        break;
    case DFMEvent::CreateStorageInfo:
        result = CALL_CONTROLLER(createStorageInfo);
        break;
    case DFMEvent::Tag: {
        result = CALL_CONTROLLER(setFileTags);
        break;
    }
    case DFMEvent::Untag:
        result = CALL_CONTROLLER(removeTagsOfFile);
        break;
    case DFMEvent::GetTagsThroughFiles:
        result = CALL_CONTROLLER(getTagsThroughFiles);
        break;
    case DFMEvent::SetFileExtraProperties:
        result = CALL_CONTROLLER(setExtraProperties);
        break;
    case DFMEvent::SetPermission:
        result = CALL_CONTROLLER(setPermissions);
        break;
    case DFMEvent::OpenFiles:
        result = CALL_CONTROLLER(openFiles);
        if (result.toBool()) {
            for( auto url : event->fileUrlList()) {
                emit fileOpened(url);
            }
        }
//        if (result.toBool()) {
//            emit fileOpened(event->fileUrl());
//        }
        break;
    case DFMEvent::OpenFilesByApp:
        result = CALL_CONTROLLER(openFilesByApp);
        if (result.toBool()) {
            for( auto url : event->fileUrlList()) {
                emit fileOpened(url);
            }
        }
//        if (result.toBool()) {
//            emit fileOpened(event->fileUrl());
//        }

        break;
    default:
        return false;
    }

end:
    if (resultData) {
        *resultData = result;
    }

    return true;
}

bool DFileService::isRegisted(const QString &scheme, const QString &host, const std::type_info &info)
{
    const HandlerType &type = HandlerType(scheme, host);

    foreach (const HandlerCreatorType &value, DFileServicePrivate::controllerCreatorHash.values(type)) {
        if (value.first == info.name()) {
            return true;
        }
    }

    foreach (const DAbstractFileController *controller, DFileServicePrivate::controllerHash.values(type)) {
        if (typeid(*controller) == info) {
            return true;
        }
    }

    return false;
}

bool DFileService::isRegisted(const QString &scheme, const QString &host)
{
    const HandlerType &type = HandlerType(scheme, host);

    return !DFileServicePrivate::controllerCreatorHash.values(type).isEmpty() || !DFileServicePrivate::controllerHash.values(type).isEmpty();
}

void DFileService::initHandlersByCreators()
{
    QMultiHash<const HandlerType, HandlerCreatorType>::const_iterator begin = DFileServicePrivate::controllerCreatorHash.constBegin();

    while (begin != DFileServicePrivate::controllerCreatorHash.constEnd()) {
        setFileUrlHandler(begin.key().first, begin.key().second, (begin.value().second)());
        ++begin;
    }

    DFileServicePrivate::controllerCreatorHash.clear();
}

DFileService *DFileService::instance()
{
    static DFileService services;

    return &services;
}

bool DFileService::setFileUrlHandler(const QString &scheme, const QString &host,
                                     DAbstractFileController *controller)
{
    if (DFileServicePrivate::handlerHash.contains(controller)) {
        return true;
    }

    const HandlerType &type = HandlerType(scheme, host);

    foreach (const DAbstractFileController *value, DFileServicePrivate::controllerHash.values(type)) {
        if (typeid(*value) == typeid(*controller)) {
            controller->deleteLater();
            return false;
        }
    }

    DFileServicePrivate::handlerHash[controller] = type;
    DFileServicePrivate::controllerHash.insertMulti(type, controller);

    return true;
}

void DFileService::unsetFileUrlHandler(DAbstractFileController *controller)
{
    if (!DFileServicePrivate::handlerHash.contains(controller)) {
        return;
    }

    DFileServicePrivate::controllerHash.remove(DFileServicePrivate::handlerHash.value(controller), controller);
}

void DFileService::clearFileUrlHandler(const QString &scheme, const QString &host)
{
    const HandlerType handler(scheme, host);

    DFileServicePrivate::controllerHash.remove(handler);
    DFileServicePrivate::controllerCreatorHash.remove(handler);
}

bool DFileService::openFile(const QObject *sender, const DUrl &url) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenFileEvent>(sender, url)).toBool();
}

bool DFileService::openFiles(const QObject *sender, const DUrlList &list) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenFilesEvent>(sender, list)).toBool();
}

bool DFileService::openFileByApp(const QObject *sender, const QString &appName, const DUrl &url) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenFileByAppEvent>(sender, appName, url)).toBool();
}

bool DFileService::openFilesByApp(const QObject *sender, const QString &appName, const QList<DUrl> &urllist) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenFilesByAppEvent>(sender, appName, urllist)).toBool();
}

bool DFileService::compressFiles(const QObject *sender, const DUrlList &list) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMCompressEvent>(sender, list)).toBool();
}

bool DFileService::decompressFile(const QObject *sender, const DUrlList &list) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMDecompressEvent>(sender, list)).toBool();
}

bool DFileService::decompressFileHere(const QObject *sender, const DUrlList &list) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMDecompressHereEvent>(sender, list)).toBool();
}

bool DFileService::writeFilesToClipboard(const QObject *sender, DFMGlobal::ClipboardAction action, const DUrlList &list) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(sender, action, list)).toBool();
}

bool DFileService::renameFile(const QObject *sender, const DUrl &from, const DUrl &to, const bool silent) const
{
    bool ok = DFMEventDispatcher::instance()->processEvent<DFMRenameEvent>(sender, from, to, silent).toBool();

    return ok;
}

bool DFileService::deleteFiles(const QObject *sender, const DUrlList &list, bool confirmationDialog, bool slient, bool force) const
{
    if (list.isEmpty())
        return false;

    foreach (const DUrl &url, list) {
        if (systemPathManager->isSystemPath(url.toLocalFile())) {
            if (!slient) {
                DThreadUtil::runInMainThread(dialogManager, &DialogManager::showDeleteSystemPathWarnDialog, DFMEvent::windowIdByQObject(sender));
            }

            return false;
        }
    }

//    if (!confirmationDialog || DThreadUtil::runInMainThread(dialogManager, &DialogManager::showDeleteFilesClearTrashDialog, DFMUrlListBaseEvent(sender, list)) == DDialog::Accepted) {
//    if (!confirmationDialog ) {
    return DFMEventDispatcher::instance()->processEventWithEventLoop(dMakeEventPointer<DFMDeleteEvent>(sender, list, slient, force)).toBool();
//    }

//    return false;
}

DUrlList DFileService::moveToTrash(const QObject *sender, const DUrlList &list) const
{
    if (list.isEmpty()) {
        return list;
    }

    if (FileUtils::isGvfsMountFile(list.first().toLocalFile())) {
        deleteFiles(sender, list);
        return list;
    }

    const DUrlList &result = qvariant_cast<DUrlList>(DFMEventDispatcher::instance()->processEventWithEventLoop(dMakeEventPointer<DFMMoveToTrashEvent>(sender, list)));

    return result;
}

void DFileService::pasteFileByClipboard(const QObject *sender, const DUrl &targetUrl) const
{
    DFMGlobal::ClipboardAction action = DFMGlobal::instance()->clipboardAction();

    if (action == DFMGlobal::UnknowAction) {
        return;
    }
    if (targetUrl.scheme() == SEARCH_SCHEME) {
        return;
    }
    const DUrlList &list = DUrl::fromQUrlList(DFMGlobal::instance()->clipboardFileUrlList());

    if (action == DFMGlobal::CutAction)
        DFMGlobal::instance()->clearClipboard();

    pasteFile(sender, action, targetUrl, list);
}

DUrlList DFileService::pasteFile(const QObject *sender, DFMGlobal::ClipboardAction action, const DUrl &targetUrl, const DUrlList &list) const
{
    const QSharedPointer<DFMPasteEvent> &event = dMakeEventPointer<DFMPasteEvent>(sender, action, targetUrl, list);
    const DUrlList &new_list = qvariant_cast<DUrlList>(DFMEventDispatcher::instance()->processEventWithEventLoop(event));

    return new_list;
}

bool DFileService::restoreFile(const QObject *sender, const DUrlList &list) const
{
    return DFMEventDispatcher::instance()->processEventWithEventLoop<DFMRestoreFromTrashEvent>(sender, list).toBool();
}

bool DFileService::mkdir(const QObject *sender, const DUrl &targetUrl) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMMkdirEvent>(sender, targetUrl)).toBool();
}

bool DFileService::touchFile(const QObject *sender, const DUrl &targetUrl) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMTouchFileEvent>(sender, targetUrl)).toBool();
}

bool DFileService::openFileLocation(const QObject *sender, const DUrl &url) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenFileLocation>(sender, url)).toBool();
}

bool DFileService::setPermissions(const QObject *sender, const DUrl &url, const QFileDevice::Permissions permissions) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMSetPermissionEvent>(sender, url, permissions)).toBool();
}

bool DFileService::addToBookmark(const QObject *sender, const DUrl &fileUrl) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMAddToBookmarkEvent>(sender, fileUrl)).toBool();
}

bool DFileService::removeBookmark(const QObject *sender, const DUrl &fileUrl) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMRemoveBookmarkEvent>(sender, fileUrl)).toBool();
}

bool DFileService::createSymlink(const QObject *sender, const DUrl &fileUrl) const
{
    QString linkName = getSymlinkFileName(fileUrl);
    QString linkPath = QFileDialog::getSaveFileName(qobject_cast<const QWidget *>(sender) ? qobject_cast<const QWidget *>(sender)->window() : Q_NULLPTR,
                                                    QObject::tr("Create symlink"), linkName);
    //handle for cancel select file
    if (linkPath.isEmpty()) {
        return false;
    }

    return createSymlink(sender, fileUrl, DUrl::fromLocalFile(linkPath), true);
}

bool DFileService::createSymlink(const QObject *sender, const DUrl &fileUrl, const DUrl &linkToUrl, bool force/* = false*/) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMCreateSymlinkEvent>(sender, fileUrl, linkToUrl, force)).toBool();
}

bool DFileService::sendToDesktop(const QObject *sender, const DUrlList &urlList) const
{
    const QString &desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    if (desktopPath.isEmpty()) {
        return false;
    }

    const QDir &desktopDir(desktopPath);
    bool ok = true;

    for (const DUrl &url : urlList) {
        const QString &linkName = getSymlinkFileName(url, desktopDir);

        ok = ok && createSymlink(sender, url, DUrl::fromLocalFile(desktopDir.filePath(linkName)));
    }

    return ok;
}

bool DFileService::shareFolder(const QObject *sender, const DUrl &fileUrl, const QString &name, bool isWritable, bool allowGuest)
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMFileShareEvent>(sender, fileUrl, name, isWritable, allowGuest)).toBool();
}

bool DFileService::unShareFolder(const QObject *sender, const DUrl &fileUrl) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMCancelFileShareEvent>(sender, fileUrl)).toBool();
}

bool DFileService::openInTerminal(const QObject *sender, const DUrl &fileUrl) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenInTerminalEvent>(sender, fileUrl)).toBool();
}


///###: make file tag(s).
bool DFileService::setFileTags(const QObject *sender, const DUrl &url, const QList<QString> &tags) const
{
    QSharedPointer<DFMSetFileTagsEvent> event(new DFMSetFileTagsEvent(sender, url, tags));
    return DFMEventDispatcher::instance()->processEvent(event).toBool();
}

bool DFileService::makeTagsOfFiles(const QObject *sender, const DUrlList &urlList, const QStringList &tags, const QSet<QString> dirtyTagFilter) const
{
    QRegExp rx("[\\\\/\':\\*\\?\"<>|%&]");
    for (const QString &tag : tags) {
        if (tag.indexOf(rx) >= 0) {
            return false;
        }
    }

    QStringList old_tagNames = getTagsThroughFiles(sender, urlList);//###: the mutual tags of multi files.
    QStringList dirty_tagNames; //###: for deleting.
    const QSet<QString> tags_set = QSet<QString>::fromList(tags);

    for (const QString &tag : old_tagNames) {
        if (!tags_set.contains(tag) && (dirtyTagFilter.isEmpty() || dirtyTagFilter.contains(tag))) {
            dirty_tagNames << tag;
        }
    }

    for (const DUrl &url : urlList) {
        QStringList tags_of_file = getTagsThroughFiles(sender, {url});
        QSet<QString> tags_of_file_set = tags_set;

        tags_of_file_set += QSet<QString>::fromList(tags_of_file);

        for (const QString &dirty_tag : dirty_tagNames) {
            tags_of_file_set.remove(dirty_tag);
        }

        QSharedPointer<DFMSetFileTagsEvent> event(new DFMSetFileTagsEvent(sender, url, tags_of_file_set.toList()));
        bool result = DFMEventDispatcher::instance()->processEventWithEventLoop(event).toBool();

        if (!result)
            return false;
    }

    return true;
}

///###: remove tag(s) of file.
bool DFileService::removeTagsOfFile(const QObject *sender, const DUrl &url, const QList<QString> &tags) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMRemoveTagsOfFileEvent>(sender, url, tags)).toBool();
}

QList<QString> DFileService::getTagsThroughFiles(const QObject *sender, const QList<DUrl> &urls) const
{
    if (urls.count() > 10)
        return DFMEventDispatcher::instance()->processEventWithEventLoop(dMakeEventPointer<DFMGetTagsThroughFilesEvent>(sender, urls)).value<QList<QString>>();

    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMGetTagsThroughFilesEvent>(sender, urls)).value<QList<QString>>();
}

const DAbstractFileInfoPointer DFileService::createFileInfo(const QObject *sender, const DUrl &fileUrl) const
{
    const DAbstractFileInfoPointer &info = DAbstractFileInfo::getFileInfo(fileUrl);

    if (info) {
        info->refresh();
        return info;
    }

    const auto &&event = dMakeEventPointer<DFMCreateFileInfoEvent>(sender, fileUrl);

    return qvariant_cast<DAbstractFileInfoPointer>(DFMEventDispatcher::instance()->processEvent(event));
}

const DDirIteratorPointer DFileService::createDirIterator(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                                          QDir::Filters filters, QDirIterator::IteratorFlags flags, bool silent) const
{
    const auto &&event = dMakeEventPointer<DFMCreateDiriterator>(sender, fileUrl, nameFilters, filters, flags, silent);

    return qvariant_cast<DDirIteratorPointer>(DFMEventDispatcher::instance()->processEvent(event));
}

const QList<DAbstractFileInfoPointer> DFileService::getChildren(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                                                QDir::Filters filters, QDirIterator::IteratorFlags flags, bool silent,bool canconst) const
{
    const auto &&event = dMakeEventPointer<DFMGetChildrensEvent>(sender, fileUrl, nameFilters, filters, flags, silent,canconst);

    return qvariant_cast<QList<DAbstractFileInfoPointer>>(DFMEventDispatcher::instance()->processEvent(event));
}

JobController *DFileService::getChildrenJob(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                            QDir::Filters filters, QDirIterator::IteratorFlags flags, bool silent) const
{
    const auto &&event = dMakeEventPointer<DFMCreateGetChildrensJob>(sender, fileUrl, nameFilters, filters, flags, silent);

    return qvariant_cast<JobController *>(DFMEventDispatcher::instance()->processEvent(event));
}

DAbstractFileWatcher *DFileService::createFileWatcher(const QObject *sender, const DUrl &fileUrl, QObject *parent) const
{
    DAbstractFileWatcher *w = qvariant_cast<DAbstractFileWatcher *>(DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMCreateFileWatcherEvent>(sender, fileUrl)));

    if (w) {
        w->setParent(parent);
    }

    return w;
}

bool DFileService::setExtraProperties(const QObject *sender, const DUrl &fileUrl, const QVariantHash &ep) const
{
    const auto &&event = dMakeEventPointer<DFMSetFileExtraProperties>(sender, fileUrl, ep);

    return DFMEventDispatcher::instance()->processEvent(event).toBool();
}

DFileDevice *DFileService::createFileDevice(const QObject *sender, const DUrl &url)
{
    const auto &&event = dMakeEventPointer<DFMUrlBaseEvent>(DFMEvent::CreateFileDevice, sender, url);

    return qvariant_cast<DFileDevice *>(DFMEventDispatcher::instance()->processEvent(event));
}

DFileHandler *DFileService::createFileHandler(const QObject *sender, const DUrl &url)
{
    const auto &&event = dMakeEventPointer<DFMUrlBaseEvent>(DFMEvent::CreateFileHandler, sender, url);

    return qvariant_cast<DFileHandler *>(DFMEventDispatcher::instance()->processEvent(event));
}

DStorageInfo *DFileService::createStorageInfo(const QObject *sender, const DUrl &url)
{
    const auto &&event = dMakeEventPointer<DFMUrlBaseEvent>(DFMEvent::CreateStorageInfo, sender, url);

    return qvariant_cast<DStorageInfo *>(DFMEventDispatcher::instance()->processEvent(event));
}

QList<DAbstractFileInfoPointer> DFileService::getRootFile()
{
    QList<DAbstractFileInfoPointer> ret;
    QMutex mex;
    mex.lock();
    setCursorBusyState(true);
    for (auto url : d_ptr->rootfilelist)
    {
        DAbstractFileInfoPointer rootinfo = createFileInfo(nullptr,url);
        if (rootinfo->exists()) {
            ret.push_back(rootinfo);
        }
    }
    // fix 25778 每次打开文管，"我的目录" 顺序随机排列
    static const QList<QString> udir = {"desktop", "videos", "music", "pictures", "documents", "downloads"};
    for (int i = 0; i < udir.count(); i++) {
        for (int j = 0; j < ret.count(); j++) {
            if (ret[j]->fileUrl().path().contains(udir[i]) && ret[j]->suffix() == SUFFIX_USRDIR && i != j) {
                ret.move(j, i);
                break;
            }
        }
    }

    setCursorBusyState(false);
    mex.unlock();
    return ret;
}

void DFileService::changeRootFile(const DUrl &fileurl, const bool bcreate)
{
    QMutexLocker lock(&d_ptr->m_mutexrootfilechange);
    if (bcreate) {
        if(!d_ptr->rootfilelist.contains(fileurl)){
            DAbstractFileInfoPointer info = createFileInfo(nullptr, fileurl);
            if(info->exists()) {
                d_ptr->rootfilelist.push_back(fileurl);
                qDebug() << "  insert   " << fileurl;
            }
        }
    }
    else {
        qDebug() << "  remove   " << d_ptr->rootfilelist;
        if(d_ptr->rootfilelist.contains(fileurl)){
            qDebug() << "  remove   " << fileurl;
            d_ptr->rootfilelist.removeOne(fileurl);
        }
    }
}

void DFileService::startQuryRootFile()
{
    if(!d_ptr->bstartonce) {
        d_ptr->bstartonce = true;
    }
    else {
        return;
    }
    qDebug() << "start thread    startQuryRootFile   ===== " << d_ptr->rootfilelist.size();
    //启用异步线程去读取
    d_ptr->m_jobcontroller = fileService->getChildrenJob(this, DUrl(DFMROOT_ROOT), QStringList(), QDir::AllEntries);
    connect(d_ptr->m_jobcontroller,&JobController::addChildren,this ,[this](const DAbstractFileInfoPointer &chi){
        QMutexLocker lock(&d_ptr->m_mutexrootfilechange);
        if (!d_ptr->rootfilelist.contains(chi->fileUrl()) && chi->exists()) {
            d_ptr->rootfilelist.push_back(chi->fileUrl());
            qDebug() << "  addChildren " << chi->fileUrl();
            emit rootFileChange(chi);
        }
    });

    connect(d_ptr->m_jobcontroller,&JobController::addChildrenList,this ,[this](QList<DAbstractFileInfoPointer> ch){
        QMutexLocker lock(&d_ptr->m_mutexrootfilechange);
        for (auto chi : ch) {
            if (!d_ptr->rootfilelist.contains(chi->fileUrl()) && chi->exists()) {
                d_ptr->rootfilelist.push_back(chi->fileUrl());
                qDebug() << "  addChildrenlist " << chi->fileUrl();
                emit rootFileChange(chi);
            }
        }
    });
    connect(d_ptr->m_jobcontroller,&JobController::finished,this,[this](){
        d_ptr->m_jobcontroller->deleteLater();
        qDebug() << "获取 m_jobcontroller  finished  " << QThread::currentThreadId();
        d_ptr->m_jobcontroller = nullptr;
        emit queryRootFileFinsh();
    });
    d_ptr->m_jobcontroller->start();
}

void DFileService::clearThread()
{
    if (d_ptr->m_jobcontroller && !d_ptr->m_jobcontroller->isFinished()) {
        d_ptr->m_jobcontroller->stop();
        d_ptr->m_jobcontroller->quit();
        QEventLoop eventLoop;

        connect(d_ptr->m_jobcontroller, &JobController::destroyed, &eventLoop, &QEventLoop::quit);
        if (!d_ptr->m_jobcontroller->isFinished()) {
            eventLoop.exec();
        }
        d_ptr->m_jobcontroller = nullptr;
    }

}

void DFileService::setCursorBusyState(const bool bbusy)
{
    //fix bug,当快速点击左边侧边栏会出现鼠标一直在转圈圈
    QMutexLocker lock(&d_ptr->m_mutexCursorState);
    if (d_ptr->m_bcursorbusy == bbusy) {
        return;
    }
    d_ptr->m_bcursorbusy = bbusy;
    if (d_ptr->m_bcursorbusy) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    }
    else {
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
    }

}

bool DFileService::checkGvfsMountfileBusy(const DUrl &url, const bool showdailog)
{
    //找出url的rootfile路径，判断rootfile是否存在
//    qDebug() << url << QThread::currentThreadId();
    if (!url.isValid()) {
        return false;
    }
    if (d_ptr->m_loop) {
        d_ptr->m_loop->exit();
    }
    //还原设置鼠标状态
    setCursorBusyState(false);

    DUrl rooturl;
    QString urlpath = url.path();
    QString rootfilename;
    if (url.scheme() == DFMROOT_SCHEME && urlpath.endsWith(SUFFIX_GVFSMP))
    {
        rootfilename = QUrl::fromPercentEncoding( url.path().toUtf8());
        QStringList rootstrlist = rootfilename.split(QRegularExpression("^//run/user/\\d+/gvfs/"));
        if (rootstrlist.size() >= 2) {
            rootfilename = rootstrlist.at(1);
            rootfilename = rootfilename.replace(QString(".") + QString(SUFFIX_GVFSMP),"");
        }
        if (!(rootfilename.startsWith("smb") || rootfilename.startsWith("ftp"))) {
            return false;
        }
        rooturl = url;
    }
    else {
        static QRegularExpression regExp("^/run/user/\\d+/gvfs/.+$",
                                         QRegularExpression::DotMatchesEverythingOption
                                         | QRegularExpression::DontCaptureOption
                                         | QRegularExpression::OptimizeOnFirstUsageOption);

        if (!regExp.match(urlpath, 0, QRegularExpression::NormalMatch, QRegularExpression::DontCheckSubjectStringMatchOption).hasMatch()) {
            return false;
        }
        int qi = 0;
        QStringList urlpathlist = urlpath.split(QRegularExpression("^/run/user/\\d+/gvfs/"));
        QString urlstr;
        QString urllast;
        if (urlpathlist.size() >= 2) {
            urllast = urlpathlist[1];
            urlstr = urlpath.left(urlpath.indexOf(urllast));
        }

        qi = urllast.indexOf("/");
        QString path;
        if (0 >= qi) {
            path = urlpath;
            QStringList rootstrlist = path.split(QRegularExpression("^/run/user/\\d+/gvfs/"));
            if (rootstrlist.size() >= 2) {
                rootfilename = rootstrlist.at(1);
                rootfilename = rootfilename.replace(QString(".") + QString(SUFFIX_GVFSMP),"");
            }
        }
        else {
            rootfilename = urllast.left(qi);
            path = urlstr + urllast.left(qi);
        }
        if (path.isNull() || path.isEmpty() ||
                !(rootfilename.startsWith("smb") || rootfilename.startsWith("ftp"))) {
            return false;
        }
        rooturl.setScheme(DFMROOT_SCHEME);
        rooturl.setPath("/" + QUrl::toPercentEncoding(path) + "." SUFFIX_GVFSMP);
    }

    return checkGvfsMountfileBusy(rooturl, rootfilename,showdailog);

}

bool DFileService::checkGvfsMountfileBusy(const DUrl &rootUrl, const QString &rootfilename, const bool showdailog)
{
    if(!rootUrl.isValid()) {
        return false;
    }
    //设置鼠标状态，查看文件状态是否存在
    setCursorBusyState(true);
    //check network online
    bool bonline = isNetWorkOnline();
    bool fileexit = false;
    if (!bonline) {
        setCursorBusyState(false);
        //文件不存在弹提示框
        if (showdailog) {
            dialogManager->showUnableToLocateDir(rootfilename);
        }
        return true;
    }

    if (rootfilename.startsWith("smb")) {
        DAbstractFileInfoPointer rootptr = createFileInfo(nullptr, rootUrl);
        fileexit = rootptr->exists();
        setCursorBusyState(false);
        //文件不存在弹提示框
        if (!fileexit && showdailog) {
            dialogManager->showUnableToLocateDir(rootfilename);
        }
        return !fileexit;
    }

    //是网络文件，就去确定host和port端口号
    bool bvist = false;
    QString host = rootfilename.mid(rootfilename.indexOf("host=")+5,rootfilename.indexOf(","));
    QString port;
    if (-1 != rootfilename.indexOf("port=")) {
        port = rootfilename.right(rootfilename.indexOf("port=")+5);
        port = port.mid(0,port.indexOf(","));
    }

    QString roottemp = rootfilename;
    QUrl url; //该QUrl类提供了一个方便的接口,用于处理URL
    url.setScheme(roottemp.left(roottemp.indexOf(":")));//设置该计划描述了URL的类型（或协议）
    if (rootfilename.startsWith("ftp")) {
        //设置URL的端口。该端口是URL的权限的一部分，如setAuthority（描述）。
        //端口必须是介于0和65535（含）。端口设置为-1表示该端口是不确定的。
         url.setPort(21);
    }
    else {
        url.setPort(port.isNull() ? -1 : port.toInt());
    }
    url.setHost(host);//设置主机地址
    url.setPath("/.hidden");//设置URL路径。该路径是自带权限后的URL的一部分，但在查询字符串之前

    QNetworkRequest request;//该QNetworkReply类包含的数据和标题,对QNetworkAccessManager发送请求
    request.setUrl(url); //这只request的请求

    QNetworkAccessManager manager;//QNetworkAccessManager 允许发送网络请求和接收回复
    //发送请求，以获得目标要求的内容，并返回一个新的QNetworkReply对象打开阅读，
    //每当新的数据到达发射的readyRead（）信号。要求的内容以及相关的头文件会被下载。
    manager.get(request);
    qDebug() << "loop sart ===========";
    connect(&manager, &QNetworkAccessManager::finished, this, [&](QNetworkReply *reply){
        qDebug() << reply->error() << reply->errorString();
        bvist = QNetworkReply::UnknownNetworkError < reply->error();
        if (d_ptr->m_loop) {
            d_ptr->m_loop->exit();
        }
    });
    if (d_ptr->m_loop) {
        d_ptr->m_loop->exec();
    }

    bonline = isNetWorkOnline();
    if (!bonline) {
        setCursorBusyState(false);
        //文件不存在弹提示框
        if (showdailog) {
            dialogManager->showUnableToLocateDir(rootfilename);
        }
        return true;
    }

    setCursorBusyState(false);
    //文件不存在弹提示框
    if (!bvist && showdailog) {
        dialogManager->showUnableToLocateDir(rootfilename);
    }
    return !bvist;
}

void DFileService::changRootFile(const QList<DAbstractFileInfoPointer> &rootinfo)
{
    QMutex mex;
    mex.lock();
    for (const DAbstractFileInfoPointer &fi : rootinfo) {
        DUrl url = fi->fileUrl();
        if(!d_ptr->rootfilelist.contains(url) && fi->exists()) {
            d_ptr->rootfilelist.push_back(url);
        }
    }
    mex.unlock();
}

bool DFileService::isNetWorkOnline()
{
    return d_ptr->m_bonline;
}

bool DFileService::checkNetWorkToVistHost(const QString &host)
{
    if (host.isNull()) {
        return false;
    }
    bool bvisit = false;
    QEventLoop eventLoop;
    QHostInfo::lookupHost(host,this,[&](QHostInfo &info){
        qDebug() << " -----       " << info.errorString() << info.hostName();
        bvisit = info.error() == QHostInfo::NoError;
        eventLoop.exit();
    });
    eventLoop.exec();

    return bvisit;
}

bool DFileService::getDoClearTrashState() const
{
    Q_D(const DFileService);

    return  d->m_bdoingcleartrash;
}

void DFileService::setDoClearTrashState(const bool bdoing)
{
    Q_D(DFileService);

    d->m_bdoingcleartrash = bdoing;
}

QList<DAbstractFileController *> DFileService::getHandlerTypeByUrl(const DUrl &fileUrl, bool ignoreHost, bool ignoreScheme)
{
    HandlerType handlerType(ignoreScheme ? "" : fileUrl.scheme(), ignoreHost ? "" : fileUrl.host());

    if (DFileServicePrivate::controllerCreatorHash.contains(handlerType)) {
        QList<DAbstractFileController *> list = DFileServicePrivate::controllerHash.values(handlerType);

        for (const HandlerCreatorType &creator : DFileServicePrivate::controllerCreatorHash.values(handlerType)) {
            DAbstractFileController *controller = (creator.second)();

            setFileUrlHandler(handlerType.first, handlerType.second, controller);

            list << controller;
        }

        DFileServicePrivate::controllerCreatorHash.remove(handlerType);
    }

    return DFileServicePrivate::controllerHash.values(handlerType);
}

QString DFileService::getSymlinkFileName(const DUrl &fileUrl, const QDir &targetDir)
{
    const DAbstractFileInfoPointer &pInfo = instance()->createFileInfo(Q_NULLPTR, fileUrl);

    if (pInfo->exists()) {
        QString baseName = pInfo->fileDisplayName() == pInfo->fileName() ? pInfo->baseName() : pInfo->fileDisplayName();
        QString shortcut = QObject::tr("Shortcut");
        QString linkBaseName;

        int number = 1;

        forever {
            if (pInfo->isFile()) {
                if (pInfo->suffix().isEmpty()) {
                    if (number == 1) {
                        linkBaseName = QString("%1 %2").arg(baseName, shortcut);
                    } else {
                        linkBaseName = QString("%1 %2%3").arg(baseName, shortcut, QString::number(number));
                    }
                } else {
                    if (number == 1) {
                        linkBaseName = QString("%1 %2.%3").arg(baseName, shortcut, pInfo->suffix());
                    } else {
                        linkBaseName = QString("%1 %2%3.%4").arg(baseName, shortcut, QString::number(number), pInfo->suffix());
                    }
                }
            } else if (pInfo->isDir()) {
                if (number == 1) {
                    linkBaseName = QString("%1 %2").arg(baseName, shortcut);
                } else {
                    linkBaseName = QString("%1 %2%3").arg(baseName, shortcut, QString::number(number));
                }
            } else if (pInfo->isSymLink()) {
                return QString();
            }

            if (targetDir.path().isEmpty()) {
                return linkBaseName;
            }

            if (targetDir.exists(linkBaseName)) {
                ++number;
            } else {
                return linkBaseName;
            }
        }
    }

    return QString();
}

void DFileService::insertToCreatorHash(const HandlerType &type, const HandlerCreatorType &creator)
{
    DFileServicePrivate::controllerCreatorHash.insertMulti(type, creator);
}

void DFileService::laterRequestSelectFiles(const DFMUrlListBaseEvent &event) const
{
    FileSignalManager *manager = fileSignalManager;

    TIMER_SINGLESHOT_OBJECT(manager, qMax(event.fileUrlList().count() * (10 + event.fileUrlList().count() / 150), 200), {
        manager->requestSelectFile(event);
                            }, event, manager)
}

void DFileService::slotError(QNetworkReply::NetworkError err)
{
    qDebug() << static_cast<QNetworkReply *>(sender())->errorString() << err;
}

///###: replace
bool DFileService::multiFilesReplaceName(const QList<DUrl> &urls, const QPair<QString, QString> &pair)const
{
    auto alteredAndUnAlteredUrls = FileBatchProcess::instance()->replaceText(urls, pair);
    AppController::multiSelectionFilesCache.first = FileBatchProcess::batchProcessFile(alteredAndUnAlteredUrls).values();

    return DFileService::checkMultiSelectionFilesCache();
}


///###: add
bool DFileService::multiFilesAddStrToName(const QList<DUrl> &urls, const QPair<QString, DFileService::AddTextFlags> &pair)const
{
    auto alteredAndUnAlteredUrls = FileBatchProcess::instance()->addText(urls, pair);
    AppController::multiSelectionFilesCache.first = FileBatchProcess::batchProcessFile(alteredAndUnAlteredUrls).values();

    return DFileService::checkMultiSelectionFilesCache();
}

///###: customize
bool DFileService::multiFilesCustomName(const QList<DUrl> &urls, const QPair<QString, QString> &pair)const
{
    auto alteredAndUnAlteredUrls = FileBatchProcess::instance()->customText(urls, pair);
    AppController::multiSelectionFilesCache.first = FileBatchProcess::batchProcessFile(alteredAndUnAlteredUrls).values();

    return DFileService::checkMultiSelectionFilesCache();
}

///###: helper function.
bool DFileService::checkMultiSelectionFilesCache()
{
    if (AppController::multiSelectionFilesCache.first.isEmpty()) {
        return false;
    }

    return true;
}
