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

#include "localfile/dfileservices.h"
#include "base/dabstractfilecontroller.h"
#include "base/dabstractfileinfo.h"
//#include "base/dabstractfilewatcher.h"
//#include "event/dfmeventdispatcher.h"
#include "localfile/dfmfilecontrollerfactory.h"
#include "localfile/dfiledevice.h"
#include "localfile/dfilehandler.h"
#include "base/dfmapplication.h"

//#include "localfile/filesignalmanager.h"
#include "event/protocol/dfmevent.h"
#include "base/define.h"
#include "base/appcontroller.h"
#include "localfile/dfmlocalfileinfo.h"
//#include "localfile/desktopfileinfo.h"
#include "localfile/pathmanager.h"
#include "base/dfmstandardpaths.h"
#include "shutil/fileutils.h"
#include "base/dfmglobal.h"
#include "base/singleton.h"
#include "base/dabstractdiriterator.h"
#include "shutil/checknetwork.h"
#include "execinfo.h"
#include "math.h"

#include <QUrl>
#include <QDebug>
#include <ddialog.h>
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
#include <QRegularExpression>

#include <sys/stat.h>

DWIDGET_USE_NAMESPACE

#define GVFS_ROOT_MATCH "^//run/user/\\d+/gvfs/"
#define GVFS_MATCH "^/run/user/\\d+/gvfs/"
#define FTP_HOST "ftp:host="
#define GVFS_MATCH_EX "^/run/user/\\d+/gvfs/.+$"

class DFileServicePrivate
{
public:

    static QMultiHash<const HandlerType, DAbstractFileController *> controllerHash;
    static QHash<const DAbstractFileController *, HandlerType> handlerHash;
    static QMultiHash<const HandlerType, HandlerCreatorType> controllerCreatorHash;

    bool m_bcursorbusy = false;
    bool m_bdoingcleartrash = false;
    QHash<QUrl, bool> m_rootsmbftpurllist;
    QMutex checkgvfsmtx, smbftpmutex, timerMutex;
    QNetworkConfigurationManager *m_networkmgr = nullptr;

    QTimer m_tagEditorChangeTimer;
    CheckNetwork m_checknetwork;
    QList<QUrl> m_tagEditorFiles;
    QStringList m_tagEditorTags;
    bool m_isMoveToTrashOver = true; //! 移除事件是否已处理完成，避免嵌套调用
};

QMultiHash<const HandlerType, DAbstractFileController *> DFileServicePrivate::controllerHash;
QHash<const DAbstractFileController *, HandlerType> DFileServicePrivate::handlerHash;
QMultiHash<const HandlerType, HandlerCreatorType> DFileServicePrivate::controllerCreatorHash;

DFileService::DFileService(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFileServicePrivate())
{
    /// init url handler register
//    AppController::registerUrlHandle();
    // register plugins
    for (const QString &key : DFMFileControllerFactory::keys()) {
        const QUrl qurl(key);

        insertToCreatorHash(HandlerType(qurl.scheme(), qurl.host()), HandlerCreatorType(typeid(DFMFileControllerFactory).name(), [key] {
            return DFMFileControllerFactory::create(key);
        }));
    }

    d_ptr->m_tagEditorChangeTimer.setSingleShot(true);
    connect(&d_ptr->m_tagEditorChangeTimer, &QTimer::timeout, this, [ = ] {
        makeTagsOfFiles(nullptr, d_ptr->m_tagEditorFiles, d_ptr->m_tagEditorTags);
    });
}

DFileService::~DFileService()
{
    d_ptr->m_rootsmbftpurllist.clear();
}

template<typename T>
QVariant eventProcess(DFileService *service, const QSharedPointer<DFMEvent> &event, T function)
{
    QSet<DAbstractFileController *> controller_set;
    QSet<QString> scheme_set;

    //    for (const QUrl &QUrl : event->handleUrlList()) {
    //        QList<DAbstractFileController *> list = service->getHandlerTypeByUrl(QUrl);
    //        if (!scheme_set.contains(QUrl.scheme())) {
    //            list << service->getHandlerTypeByUrl(QUrl, true);
    //        } else {
    //            scheme_set << QUrl.scheme();
    //        }

    //        for (DAbstractFileController *controller : list) {
    //            if (!controller || controller_set.contains(controller)) {
    //                continue;
    //            }

    //            controller_set << controller;

    //            typedef typename std::remove_reference<typename QtPrivate::FunctionPointer<T>::Arguments::Car>::type::Type DFMEventType;

    //            const QVariant result = QVariant::fromValue((controller->*function)(event.staticCast<DFMEventType>()));

    //            if (event->isAccepted()) {
    //                return result;
    //            }
    //        }
    //    }

    return QVariant();
}

//bool DFileService::fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData)
//{
//#ifdef DDE_COMPUTER_TRASH
//    switch (event->type()) {
//    case DFMEvent::WriteUrlsToClipboard:
//    case DFMEvent::DeleteFiles:
//    case DFMEvent::MoveToTrash: {
//        QList<QUrl> urlList = event->fileUrlList();

//        if (urlList.contains(DesktopFileInfo::computerDesktopFileUrl())) {
//            DesktopFile df(DesktopFileInfo::computerDesktopFileUrl().toLocalFile());
//            if (df.getDeepinId() == "dde-computer") {
//                urlList.removeOne(DesktopFileInfo::computerDesktopFileUrl());
//            }
//        }

//        if (urlList.contains(DesktopFileInfo::trashDesktopFileUrl())) {
//            DesktopFile df(DesktopFileInfo::trashDesktopFileUrl().toLocalFile());
//            if (df.getDeepinId() == "dde-trash") {
//                urlList.removeOne(DesktopFileInfo::trashDesktopFileUrl());
//            }
//        }

//        event->setData(urlList);
//        break;
//    }
//    default:
//        break;
//    }
//#endif

//#define CALL_CONTROLLER(Fun)\
//    eventProcess(this, event, &DAbstractFileController::Fun);

//    QVariant result;

//    switch (event->type()) {
//    case DFMEvent::OpenFile:
//        result = CALL_CONTROLLER(openFile);

//        if (result.toBool()) {
//            Q_EMIT fileOpened(event->url());
//        }
//        break;
//    case DFMEvent::OpenFileByApp:
//        result = CALL_CONTROLLER(openFileByApp);

//        if (result.toBool()) {
//            Q_EMIT fileOpened(event->url());
//        }

//        break;
//    case DFMEvent::CompressFiles:
//        result = CALL_CONTROLLER(compressFiles);
//        break;
//    case DFMEvent::DecompressFile:
//        result = CALL_CONTROLLER(decompressFile);
//        break;
//    case DFMEvent::DecompressFileHere:
//        result = CALL_CONTROLLER(decompressFileHere);
//        break;
//    case DFMEvent::WriteUrlsToClipboard:
//        result = CALL_CONTROLLER(writeFilesToClipboard);
//        break;
//    case DFMEvent::RenameFile: {
//        const QSharedPointer<DFMRenameEvent> &e = event.staticCast<DFMRenameEvent>();
//        const DAbstractFileInfoPointer &f = createFileInfo(this, e->toUrl());
//        if (f && f->exists() && !e->silent()) {
//            // sp3_fix: 添加文件命名规则  当文件名为..时，弹出提示，文件名不能为..
//            QString strFrom = e->fromUrl().toString();
//            QString strTo = e->toUrl().toString();
//            QString strGrandParent = strFrom.section(QDir::separator(), 0, -3);
//            if (strTo == strGrandParent) { // 如果目的目录是源目录的父目录的父目录（及目录名为..）
//                //                DThreadUtil::runInMainThread(dialogManager, &DialogManager::showRenameNameDotDotErrorDialog, *event.data());
//            } else {
//                //                DThreadUtil::runInMainThread(dialogManager, &DialogManager::showRenameNameSameErrorDialog, f->fileDisplayName(), *event.data());
//            }
//            result = false;
//        } else {
//            result = CALL_CONTROLLER(renameFile);

//            if (result.toBool()) {
//                Q_EMIT fileRenamed(e->fromUrl(), e->toUrl());
//            }
//        }

//        break;
//    }
//    case DFMEvent::DeleteFiles: {
//        // 解决撤销操作后文件删除不提示问题
//        for (const QUrl &QUrl : event->fileUrlList()) {
//            //书签保存已删除目录，在这里剔除对书签文件是否存在的判断
//            if (QUrl.scheme() == "bookmark") {
//                result = CALL_CONTROLLER(deleteFiles);

//                if (result.toBool()) {
//                    for (const QUrl &r : event->fileUrlList()) {
//                        Q_EMIT fileDeleted(r);
//                    }
//                }
//                break;
//            }
//            const DAbstractFileInfoPointer &f = createFileInfo(this, QUrl);
//            if (f && f->exists()) {
//                static bool lock = false;
//                // 如果传入的 event 中 silent 为 true，就不再弹框询问是否删除
//                auto delEvent = dynamic_cast<DFMDeleteEvent *>(event.data());
//                //                bool deleteFileSilently = delEvent && delEvent->silent();
//                if (!lock /*&& (deleteFileSilently || DThreadUtil::runInMainThread(dialogManager,
//                                                                                                         &DialogManager::showDeleteFilesClearTrashDialog,
//                                                                                                         DFMUrlListBaseEvent(nullptr, event->fileUrlList())) == DDialog::Accepted)*/) {
//                    lock = true;
//                    result = CALL_CONTROLLER(deleteFiles);
//                    lock = false;
//                    if (result.toBool()) {
//                        for (const QUrl &r : event->fileUrlList()) {
//                            Q_EMIT fileDeleted(r);
//                        }
//                    }
//                }

//                if (lock) {
//                    result = CALL_CONTROLLER(deleteFiles);
//                    if (result.toBool()) {
//                        for (const QUrl &r : event->fileUrlList()) {
//                            Q_EMIT fileDeleted(r);
//                        }
//                    }
//                } else {
//                    //fix bug 31324,判断当前操作是否是清空回收站，是就在结束时改变清空回收站状态
//                    if (event->fileUrlList().count() == 1 && event->fileUrlList().first().toString() == TRASH_ROOT) {
//                        setDoClearTrashState(false);
//                    }
//                }

//                break;
//            } else {
//                continue;
//            }
//        }
//        break;

//    }
//    case DFMEvent::MoveToTrash: {
//        if (event->fileUrlList().isEmpty())
//            break;

//        //handle system files should not be able to move to trash
//        //        Q_FOREACH (const QUrl &url, event->fileUrlList()) {
//        //            if (systemPathManager->isSystemPath(url.toLocalFile())) {
//        //                DThreadUtil::runInMainThread(dialogManager, &DialogManager::showDeleteSystemPathWarnDialog, event->windowId());
//        //                result = QVariant::fromValue(event->fileUrlList());
//        //                goto end;
//        //            }
//        //        }

//        //! 显示删除确认对话框
//        bool bShowConfimDlg = DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowDeleteConfirmDialog).toBool();
//        QUrl url = event->fileUrlList().first();
//        if (bShowConfimDlg
//                && d_ptr->m_isMoveToTrashOver
//                && !url.isTrashFile()
//                && !url.isRecentFile()
//                && url.scheme() != BURN_SCHEME
//                //                && !VaultController::isVaultFile(url.toLocalFile())
//                && !url.isUserShareFile()) {

//            //            if (DThreadUtil::runInMainThread(
//            //                        dialogManager,
//            //                        &DialogManager::showNormalDeleteConfirmDialog,
//            //                        DFMUrlListBaseEvent(nullptr, event->fileUrlList())) != DDialog::Accepted)
//            break;
//        }

//        d_ptr->m_isMoveToTrashOver = false;
//        result = CALL_CONTROLLER(moveToTrash);
//        d_ptr->m_isMoveToTrashOver = true;

//        const QList<QUrl> &list = event->fileUrlList();
//        const QList<QUrl> new_list = qvariant_cast<QList<QUrl>>(result);

//        for (int i = 0; i < new_list.count(); ++i) {
//            if (!new_list.at(i).isValid() /*|| VaultController::isVaultFile(new_list.at(i).path())*/)
//                continue;

//            Q_EMIT fileMovedToTrash(list.at(i), new_list.at(i));
//        }

//        break;
//    }
//    case DFMEvent::RestoreFromTrash: {
//        result = CALL_CONTROLLER(restoreFile);
//        break;
//    }
//    case DFMEvent::PasteFile: {
//        result = CALL_CONTROLLER(pasteFile);
//        break;
//    }
//    case DFMEvent::Mkdir:
//        result = CALL_CONTROLLER(mkdir);
//        break;
//    case DFMEvent::TouchFile:
//        result = CALL_CONTROLLER(touch);
//        break;
//    case DFMEvent::OpenFileLocation:
//        result = CALL_CONTROLLER(openFileLocation);
//        break;
//    case DFMEvent::AddToBookmark:
//        result = CALL_CONTROLLER(addToBookmark);
//        break;
//    case DFMEvent::RemoveBookmark:
//        result = CALL_CONTROLLER(removeBookmark);
//        break;
//    case DFMEvent::CreateSymlink:
//        result = CALL_CONTROLLER(createSymlink);
//        break;
//    case DFMEvent::FileShare:
//        result = CALL_CONTROLLER(shareFolder);
//        break;
//    case DFMEvent::CancelFileShare:
//        result = CALL_CONTROLLER(unShareFolder);
//        break;
//    case DFMEvent::OpenInTerminal:
//        result = CALL_CONTROLLER(openInTerminal);
//        break;
//    case DFMEvent::GetChildrens:
//        result = CALL_CONTROLLER(getChildren);
//        break;
//    case DFMEvent::CreateFileInfo:
//        result = CALL_CONTROLLER(createFileInfo);
//        break;
//    case DFMEvent::CreateDiriterator:
//        result = CALL_CONTROLLER(createDirIterator);
//        break;
//    case DFMEvent::CreateGetChildrensJob: {
//        result = CALL_CONTROLLER(createDirIterator);

//        //        const QSharedPointer<DFMCreateGetChildrensJob> &e = event.staticCast<DFMCreateGetChildrensJob>();

//        //        if (event->isAccepted()) {
//        //            result = QVariant::fromValue(new JobController(e->url(), qvariant_cast<DAbstractDirIteratorPointer>(result)));
//        //        } else {
//        //            result = QVariant::fromValue(new JobController(e->url(), e->nameFilters(), e->filters()));
//        //        }

//        break;
//    }
//    case DFMEvent::CreateFileWatcher:
//        result = CALL_CONTROLLER(createFileWatcher);
//        break;
//    case DFMEvent::CreateFileDevice:
//        result = CALL_CONTROLLER(createFileDevice);
//        break;
//    case DFMEvent::CreateFileHandler:
//        result = CALL_CONTROLLER(createFileHandler);
//        break;
//    case DFMEvent::CreateStorageInfo:
//        //        result = CALL_CONTROLLER(createStorageInfo);
//        break;
//    case DFMEvent::Tag: {
//        result = CALL_CONTROLLER(setFileTags);
//        break;
//    }
//    case DFMEvent::Untag:
//        result = CALL_CONTROLLER(removeTagsOfFile);
//        break;
//    case DFMEvent::GetTagsThroughFiles:
//        result = CALL_CONTROLLER(getTagsThroughFiles);
//        break;
//    case DFMEvent::SetFileExtraProperties:
//        result = CALL_CONTROLLER(setExtraProperties);
//        break;
//    case DFMEvent::SetPermission:
//        result = CALL_CONTROLLER(setPermissions);
//        break;
//    case DFMEvent::OpenFiles:
//        result = CALL_CONTROLLER(openFiles);
//        if (result.toBool()) {
//            for (auto QUrl : event->fileUrlList()) {
//                Q_EMIT fileOpened(QUrl);
//            }
//        }
//        break;
//    case DFMEvent::OpenFilesByApp:
//        result = CALL_CONTROLLER(openFilesByApp);
//        if (result.toBool()) {
//            for (auto QUrl : event->fileUrlList()) {
//                Q_EMIT fileOpened(QUrl);
//            }
//        }

//        break;
//    default:
//        return false;
//    }

//end:
//    if (resultData) {
//        *resultData = result;
//    }

//    return true;
//}

bool DFileService::isRegisted(const QString &scheme, const QString &host, const std::type_info &info)
{
    const HandlerType &type = HandlerType(scheme, host);

    Q_FOREACH (const HandlerCreatorType &value, DFileServicePrivate::controllerCreatorHash.values(type)) {
        if (value.first == info.name()) {
            return true;
        }
    }

    Q_FOREACH (const DAbstractFileController *controller, DFileServicePrivate::controllerHash.values(type)) {
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

    Q_FOREACH (const DAbstractFileController *value, DFileServicePrivate::controllerHash.values(type)) {
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

bool DFileService::openFile(const QObject *sender, const QUrl &url) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenFileEvent>(sender, url)).toBool();
}

bool DFileService::openFiles(const QObject *sender, const QList<QUrl> &list, const bool isEnter) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenFilesEvent>(sender, list, isEnter)).toBool();
}

bool DFileService::openFileByApp(const QObject *sender, const QString &appName, const QUrl &url) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenFileByAppEvent>(sender, appName, url)).toBool();
}

bool DFileService::openFilesByApp(const QObject *sender, const QString &appName, const QList<QUrl> &urllist, const bool isenter) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenFilesByAppEvent>(sender, appName, urllist, isenter)).toBool();
}

bool DFileService::compressFiles(const QObject *sender, const QList<QUrl> &list) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMCompressEvent>(sender, list)).toBool();
}

bool DFileService::decompressFile(const QObject *sender, const QList<QUrl> &list) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMDecompressEvent>(sender, list)).toBool();
}

bool DFileService::decompressFileHere(const QObject *sender, const QList<QUrl> &list) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMDecompressHereEvent>(sender, list)).toBool();
}

bool DFileService::writeFilesToClipboard(const QObject *sender, DFMGlobal::ClipboardAction action, const QList<QUrl> &list) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(sender, action, list)).toBool();
}

bool DFileService::renameFile(const QObject *sender, const QUrl &from, const QUrl &to, const bool silent) const
{
    bool ok = false;
    //    ok = DFMEventDispatcher::instance()->processEvent<DFMRenameEvent>(sender, from, to, silent).toBool();
    return ok;
}

bool DFileService::deleteFiles(const QObject *sender, const QList<QUrl> &list, bool confirmationDialog, bool slient, bool force) const
{
    Q_UNUSED(confirmationDialog)
    if (list.isEmpty())
        return false;

    //    Q_FOREACH (const QUrl &url, list) {
    //        if (systemPathManager->isSystemPath(url.toLocalFile())) {
    //            if (!slient) {
    //                DThreadUtil::runInMainThread(dialogManager, &DialogManager::showDeleteSystemPathWarnDialog, DFMEvent::windowIdByQObject(sender));
    //            }

    //            return false;
    //        }
    //    }

    //    if (!confirmationDialog || DThreadUtil::runInMainThread(dialogManager, &DialogManager::showDeleteFilesClearTrashDialog, DFMUrlListBaseEvent(sender, list)) == DDialog::Accepted) {
    //    if (!confirmationDialog ) {
    //    return DFMEventDispatcher::instance()->processEventWithEventLoop(dMakeEventPointer<DFMDeleteEvent>(sender, list, slient, force)).toBool();
    //    }

    return false;
}

QList<QUrl> DFileService::moveToTrash(const QObject *sender, const QList<QUrl> &list) const
{
    //    if (list.isEmpty()) {
    //        return list;
    //    }

    //    //fix bug#30027 删除文件时将剪切板中的该文件移除
    //    {
    //        QList<QUrl> org = DFMGlobal::instance()->clipboardFileUrlList();
    //        DFMGlobal::ClipboardAction action = DFMGlobal::instance()->clipboardAction();
    //        if (!org.isEmpty() && action != DFMGlobal::UnknowAction) {
    //            for (const QUrl &nd : list) {
    //                if (org.isEmpty())
    //                    break;
    //                org.removeAll(nd);
    //            }
    //            if (org.isEmpty()) //没有文件则清空剪切板
    //                DFMGlobal::clearClipboard();
    //            else
    //                DFMGlobal::setUrlsToClipboard(org, action);
    //        }
    //    }
    //end

    //    if (FileUtils::isGvfsMountFile(list.first().toLocalFile())) {
    //        deleteFiles(sender, list);
    //        return list;
    //    }

    //    const QList<QUrl> &result = qvariant_cast<QList<QUrl>>(DFMEventDispatcher::instance()->processEventWithEventLoop(dMakeEventPointer<DFMMoveToTrashEvent>(sender, list)));

    //    return result;
    return {};
}

void DFileService::pasteFileByClipboard(const QObject *sender, const QUrl &targetUrl) const
{
    DFMGlobal::ClipboardAction action = DFMGlobal::instance()->clipboardAction();

    if (action == DFMGlobal::UnknowAction) {
        return;
    }

    const QList<QUrl> &list = DFMGlobal::instance()->clipboardFileUrlList();

    if (action == DFMGlobal::CutAction) {
        // fix bug 63441
        // 如果是剪切操作，则禁止跨用户的粘贴操作
        QByteArray userId = qApp->clipboard()->mimeData()->data("userId");
        if (!userId.isEmpty() && (userId.toInt() != DFMGlobal::getUserId()))
            return;

        DFMGlobal::instance()->clearClipboard();
    }

    pasteFile(sender, action, targetUrl, list);
}

QList<QUrl> DFileService::pasteFile(const QObject *sender, DFMGlobal::ClipboardAction action, const QUrl &targetUrl, const QList<QUrl> &list) const
{
    //    const QSharedPointer<DFMPasteEvent> &event = dMakeEventPointer<DFMPasteEvent>(sender, action, targetUrl, list);
    //    const QList<QUrl> &new_list = qvariant_cast<QList<QUrl>>(DFMEventDispatcher::instance()->processEventWithEventLoop(event));

    //    return new_list;
    return {};
}

bool DFileService::restoreFile(const QObject *sender, const QList<QUrl> &list) const
{
    //    return DFMEventDispatcher::instance()->processEventWithEventLoop<DFMRestoreFromTrashEvent>(sender, list).toBool();
    return false;
}

bool DFileService::mkdir(const QObject *sender, const QUrl &targetUrl) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMMkdirEvent>(sender, targetUrl)).toBool();
    return false;
}

bool DFileService::touchFile(const QObject *sender, const QUrl &targetUrl) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMTouchFileEvent>(sender, targetUrl)).toBool();
    return false;
}

bool DFileService::openFileLocation(const QObject *sender, const QUrl &url) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenFileLocation>(sender, url)).toBool();
    return false;
}

bool DFileService::setPermissions(const QObject *sender, const QUrl &url, const QFileDevice::Permissions permissions) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMSetPermissionEvent>(sender, url, permissions)).toBool();
    return false;
}

bool DFileService::addToBookmark(const QObject *sender, const QUrl &fileUrl) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMAddToBookmarkEvent>(sender, fileUrl)).toBool();
    return false;
}

bool DFileService::removeBookmark(const QObject *sender, const QUrl &fileUrl) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMRemoveBookmarkEvent>(sender, fileUrl)).toBool();
    return false;
}

bool DFileService::createSymlink(const QObject *sender, const QUrl &fileUrl) const
{
    QString linkName = getSymlinkFileName(fileUrl);
    QString linkPath = QFileDialog::getSaveFileName(qobject_cast<const QWidget *>(sender) ? qobject_cast<const QWidget *>(sender)->window() : Q_NULLPTR,
                                                    QObject::tr("Create symlink"), linkName);

    //    //! QFileDialog::getSaveFileName not support vault file, so we need get path self.
    //    if (VaultController::ins()->isVaultFile(linkPath)) {
    //        QStringList strList = linkPath.split("/");
    //        if (strList.back() != linkName) {
    //            strList.removeLast();
    //            linkPath = strList.join("/") + "/" + linkName;
    //        }
    //    }

    //handle for cancel select file
    if (linkPath.isEmpty()) {
        return false;
    }

    return createSymlink(sender, fileUrl, QUrl::fromLocalFile(linkPath), true);
}

bool DFileService::createSymlink(const QObject *sender, const QUrl &fileUrl, const QUrl &linkToUrl, bool force/* = false*/) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMCreateSymlinkEvent>(sender, fileUrl, linkToUrl, force)).toBool();
    return false;
}

bool DFileService::sendToDesktop(const QObject *sender, const QList<QUrl> &urlList) const
{
    const QString &desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    if (desktopPath.isEmpty()) {
        return false;
    }

    const QDir &desktopDir(desktopPath);
    bool ok = true;

    for (const QUrl &QUrl : urlList) {
        const QString &linkName = getSymlinkFileName(QUrl, desktopDir);

        ok = ok && createSymlink(sender, QUrl, QUrl::fromLocalFile(desktopDir.filePath(linkName)));
    }

    return ok;
}

//void DFileService::sendToBluetooth(const QList<QUrl> &urlList) const
//{
//    dialogManager->showBluetoothTransferDlg(urlList);
//}

bool DFileService::shareFolder(const QObject *sender, const QUrl &fileUrl, const QString &name, bool isWritable, bool allowGuest)
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMFileShareEvent>(sender, fileUrl, name, isWritable, allowGuest)).toBool();
    return false;
}

bool DFileService::unShareFolder(const QObject *sender, const QUrl &fileUrl) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMCancelFileShareEvent>(sender, fileUrl)).toBool();
    return false;
}

bool DFileService::openInTerminal(const QObject *sender, const QUrl &fileUrl) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenInTerminalEvent>(sender, fileUrl)).toBool();
    return false;
}


///###: make file tag(s).
bool DFileService::setFileTags(const QObject *sender, const QUrl &url, const QList<QString> &tags) const
{
    //    QSharedPointer<DFMSetFileTagsEvent> event(new DFMSetFileTagsEvent(sender, url, tags));
    //    return DFMEventDispatcher::instance()->processEvent(event).toBool();
    return false;
}

bool DFileService::makeTagsOfFiles(const QObject *sender, const QList<QUrl> &urlList, const QStringList &tags, const QSet<QString> dirtyTagFilter) const
{
    //    QRegExp rx("[\\\\/\':\\*\\?\"<>|%&]");
    //    auto ret = std::any_of(tags.begin(), tags.end(), [rx](const QString & tag) {
    //        return tag.indexOf(rx) >= 0;
    //    });

    //    if (ret)
    //        return false;

    //    QStringList old_tagNames = getTagsThroughFiles(sender, urlList);//###: the mutual tags of multi files.
    //    QStringList dirty_tagNames; //###: for deleting.
    //    const QSet<QString> tags_set = QSet<QString>::fromList(tags);

    //    for (const QString &tag : old_tagNames) {
    //        if (!tags_set.contains(tag) && (dirtyTagFilter.isEmpty() || dirtyTagFilter.contains(tag))) {
    //            dirty_tagNames << tag;
    //        }
    //    }

    //    bool loopEvent = urlList.length() > 5;
    //    QList<DFMSetFileTagsEvent *> eventList;
    //    for (const QUrl &QUrl : urlList) {
    //        QStringList tags_of_file = getTagsThroughFiles(sender, {QUrl}, loopEvent);
    //        QSet<QString> tags_of_file_set = tags_set;

    //        tags_of_file_set += QSet<QString>::fromList(tags_of_file);

    //        for (const QString &dirty_tag : dirty_tagNames) {
    //            tags_of_file_set.remove(dirty_tag);
    //        }

    //        DFMSetFileTagsEvent *event = new DFMSetFileTagsEvent(sender, QUrl, tags_of_file_set.toList());
    //        eventList.append(event);
    //    }

    //    for (DFMSetFileTagsEvent *event : eventList) {
    //        QSharedPointer<DFMSetFileTagsEvent> spEvent(event);
    //        bool result = DFMEventDispatcher::instance()->processEventWithEventLoop(spEvent).toBool();

    //        if (!result)
    //            return false;
    //    }

    //    return true;
    return false;
}

///###: remove tag(s) of file.
bool DFileService::removeTagsOfFile(const QObject *sender, const QUrl &url, const QList<QString> &tags) const
{
    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMRemoveTagsOfFileEvent>(sender, url, tags)).toBool();

    return false;
}

QList<QString> DFileService::getTagsThroughFiles(const QObject *sender, const QList<QUrl> &urls, const bool loopEvent) const
{
    //    if (urls.count() > 10 || loopEvent)
    //        return DFMEventDispatcher::instance()->processEventWithEventLoop(dMakeEventPointer<DFMGetTagsThroughFilesEvent>(sender, urls)).value<QList<QString>>();

    //    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMGetTagsThroughFilesEvent>(sender, urls)).value<QList<QString>>();
    return {};
}

//const DFMLocalFileInfoPointer DFileService::createFileInfo(const QObject *sender, const QUrl &fileUrl) const
//{
//    return DFMLocalFileInfoPointer(new DFMLocalFileInfo(fileUrl));
//}

const DAbstractDirIteratorPointer DFileService::createDirIterator(const QObject *sender, const QUrl &fileUrl, const QStringList &nameFilters,
                                                          QDir::Filters filters, QDirIterator::IteratorFlags flags, bool silent, bool isgvfs) const
{


    //    const auto &&event = dMakeEventPointer<DFMCreateDiriterator>(sender, fileUrl, nameFilters, filters, flags, silent, isgvfs);
    //    return qvariant_cast<DAbstractDirIteratorPointer>(DFMEventDispatcher::instance()->processEvent(event));
    return DAbstractDirIteratorPointer();
}

const QList<DAbstractFileInfoPointer> DFileService::getChildren(const QObject *sender, const QUrl &fileUrl, const QStringList &nameFilters,
                                                                QDir::Filters filters, QDirIterator::IteratorFlags flags, bool silent, bool canconst) const
{
    //    const auto &&event = dMakeEventPointer<DFMGetChildrensEvent>(sender, fileUrl, nameFilters, filters, flags, silent, canconst);

    //    return qvariant_cast<QList<DAbstractFileInfoPointer>>(DFMEventDispatcher::instance()->processEvent(event));
    return {};
}

//JobController *DFileService::getChildrenJob(const QObject *sender, const QUrl &fileUrl, const QStringList &nameFilters,
//                                            QDir::Filters filters, QDirIterator::IteratorFlags flags, bool silent, const bool isgfvs) const
//{
//    const auto &&event = dMakeEventPointer<DFMCreateGetChildrensJob>(sender, fileUrl, nameFilters, filters, flags, silent, isgfvs);

//    return qvariant_cast<JobController *>(DFMEventDispatcher::instance()->processEvent(event));
//}



bool DFileService::setExtraProperties(const QObject *sender, const QUrl &fileUrl, const QVariantHash &ep) const
{
    //    const auto &&event = dMakeEventPointer<DFMSetFileExtraProperties>(sender, fileUrl, ep);

    //    return DFMEventDispatcher::instance()->processEvent(event).toBool();
    return false;
}

DFileDevice *DFileService::createFileDevice(const QObject *sender, const QUrl &url)
{
    //    const auto &&event = dMakeEventPointer<DFMUrlBaseEvent>(DFMEvent::CreateFileDevice, sender, url);

    //    return qvariant_cast<DFileDevice *>(DFMEventDispatcher::instance()->processEvent(event));
    return nullptr;
}

DFileHandler *DFileService::createFileHandler(const QObject *sender, const QUrl &url)
{
    //    const auto &&event = dMakeEventPointer<DFMUrlBaseEvent>(DFMEvent::CreateFileHandler, sender, url);

    //    return qvariant_cast<DFileHandler *>(DFMEventDispatcher::instance()->processEvent(event));
    return nullptr;
}

//DStorageInfo *DFileService::createStorageInfo(const QObject *sender, const QUrl &url)
//{
//    const auto &&event = dMakeEventPointer<DFMUrlBaseEvent>(DFMEvent::CreateStorageInfo, sender, url);

//    return qvariant_cast<DStorageInfo *>(DFMEventDispatcher::instance()->processEvent(event));
//}

void DFileService::setCursorBusyState(const bool bbusy)
{
    //fix bug 34594,当快速点击左边侧边栏会出现鼠标一直在转圈圈, 去掉全局判断，直接调用鼠标状态
    if (bbusy) {
        if (QApplication::overrideCursor() && QApplication::overrideCursor()->shape() == Qt::WaitCursor)
            return;
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    } else {
        //        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
        //! fix bug#39929 列表模式调整列表宽度无选中标记。
        QApplication::restoreOverrideCursor();
    }
}

//bool DFileService::checkGvfsMountfileBusy(const QUrl &url, const bool showdailog)
//{
//    //找出url的rootfile路径，判断rootfile是否存在
//    Q_D(DFileService);
//    //    printStacktrace(6);
//    //还原设置鼠标状态
//    setCursorBusyState(false);

//    QUrl rooturl;
//    QString urlpath = url.path();
//    QString rootfilename;
//    if (url.scheme() == DFMROOT_SCHEME /*&& urlpath.endsWith(SUFFIX_GVFSMP)*/) {
//        rootfilename = QUrl::fromPercentEncoding(url.path().toUtf8());
//        QStringList rootstrlist = rootfilename.split(QRegularExpression(GVFS_ROOT_MATCH));
//        if (rootstrlist.size() >= 2) {
//            rootfilename = rootstrlist.at(1);
//            rootfilename = rootfilename.replace(QString(".") + QString(SUFFIX_GVFSMP), "");
//        }
//        if (!(rootfilename.startsWith(SMB_SCHEME) || rootfilename.startsWith(FTP_SCHEME)
//              || rootfilename.startsWith(SFTP_SCHEME))) {
//            return false;
//        }
//        rooturl = url;
//    } else {
//        static QRegularExpression regExp(GVFS_MATCH_EX,
//                                         QRegularExpression::DotMatchesEverythingOption
//                                         | QRegularExpression::DontCaptureOption
//                                         | QRegularExpression::OptimizeOnFirstUsageOption);

//        if (!regExp.match(urlpath, 0, QRegularExpression::NormalMatch, QRegularExpression::DontCheckSubjectStringMatchOption).hasMatch()) {
//            return false;
//        }
//        int qi = 0;
//        QStringList urlpathlist = urlpath.split(QRegularExpression(GVFS_MATCH));
//        QString urlstr;
//        QString urllast;
//        if (urlpathlist.size() >= 2) {
//            urllast = urlpathlist[1];
//            urlstr = urlpath.left(urlpath.indexOf(urllast));
//        }

//        qi = urllast.indexOf("/");
//        QString path;
//        if (0 >= qi) {
//            path = urlpath;
//            QStringList rootstrlist = path.split(QRegularExpression(GVFS_MATCH));
//            if (rootstrlist.size() >= 2) {
//                rootfilename = rootstrlist.at(1);
//                rootfilename = rootfilename.replace(QString(".") + QString(SUFFIX_GVFSMP), "");
//            }
//        } else {
//            rootfilename = urllast.left(qi);
//            path = urlstr + urllast.left(qi);
//        }
//        if (path.isNull() || path.isEmpty() ||
//                !(rootfilename.startsWith(SMB_SCHEME) || rootfilename.startsWith(FTP_SCHEME)
//                  || rootfilename.startsWith(SFTP_SCHEME))) {
//            return false;
//        }
//        rooturl.setScheme(DFMROOT_SCHEME);
//        rooturl.setPath("/" + QUrl::toPercentEncoding(path) + "." SUFFIX_GVFSMP);
//    }
//    if (isSmbFtpContain(rooturl)) {
//        //文件不存在弹提示框
//        d->smbftpmutex.lock();
//        bool bbusy = d->m_rootsmbftpurllist.value(rooturl);
//        d->smbftpmutex.unlock();
//        if (bbusy && showdailog) {
//            dialogManager->showUnableToLocateDir(rootfilename);
//        }
//        return bbusy;
//    }
//    bool isbusy = checkGvfsMountfileBusy(rooturl, rootfilename, showdailog);
//    d->smbftpmutex.lock();
//    d->m_rootsmbftpurllist.insert(rooturl, isbusy);
//    d->smbftpmutex.unlock();
//    QTimer::singleShot(500, this, [rooturl, d]() {
//        QMutexLocker lk(&d->smbftpmutex);
//        d->m_rootsmbftpurllist.remove(rooturl);
//    });
//    return isbusy;
//}

//bool DFileService::checkGvfsMountfileBusy(const QUrl &rootUrl, const QString &rootFileName, const bool bShowDailog)
//{
//    Q_D(DFileService);

//    if (!rootUrl.isValid()) {
//        return false;
//    }
//    //设置鼠标状态，查看文件状态是否存在
//    setCursorBusyState(true);

//    if (rootFileName.startsWith(SMB_SCHEME)) {
//        DAbstractFileInfoPointer rootptr = createFileInfo(nullptr, rootUrl);
//        bool fileexit = rootptr->exists();
//        setCursorBusyState(false);
//        //文件不存在弹提示框
//        if (!fileexit && bShowDailog) {
//            dialogManager->showUnableToLocateDir(rootFileName);
//        }
//        return !fileexit;
//    }

//    //是网络文件，就去确定host和port端口号
//    bool bvist = true;
//    QString host, port;
//    QStringList ipInfoList = rootFileName.split(",");
//    if (!ipInfoList.isEmpty()) {
//        int spliteIndex = ipInfoList[0].indexOf("=");
//        host = ipInfoList[0].mid((spliteIndex >= 0 && spliteIndex < ipInfoList[0].length() - 1)
//                ? spliteIndex + 1 : 0);
//    } else {
//        return true;
//    }

//    if (ipInfoList.count() >= 2 && -1 != ipInfoList[1].indexOf("port=")) {
//        port = ipInfoList[1].replace("port=", "");
//    }

//    bvist = d->m_checknetwork.isHostAndPortConnect(host, port);

//    setCursorBusyState(false);

//    //文件不存在弹提示框
//    if (!bvist && bShowDailog) {
//        dialogManager->showUnableToLocateDir(rootFileName);
//    }
//    qDebug() << bvist;
//    return !bvist;
//}

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
#include <QProcess>
void DFileService::dealPasteEnd(const QSharedPointer<DFMEvent> &event, const QList<QUrl> &result)
{
    //    if (event->isAccepted()) {
    //        DFMUrlListBaseEvent e(event->sender(), result);

    //        //        e.setWindowId(event->windowId());
    //        laterRequestSelectFiles(e);
    //    }

    //    const QList<QUrl> &list = event->fileUrlList();
    //    const QList<QUrl> new_list = result;

    //    for (int i = 0; i < new_list.count(); ++i) {
    //        const QUrl &QUrl = new_list.at(i);

    //        if (QUrl.isEmpty())
    //            continue;

    //        DFMGlobal::ClipboardAction action = event.staticCast<DFMPasteEvent>()->action();

    //        if (action == DFMGlobal::ClipboardAction::CopyAction) {
    //            Q_EMIT fileCopied(list.at(i), QUrl);
    //        } else if (action == DFMGlobal::ClipboardAction::CutAction) {
    //            Q_EMIT fileRenamed(list.at(i), QUrl);
    //        }
    //    }

    // fix bug#202007010020 发送到光驱暂存区的文件保留了文件的原始权限信息，在刻录或删除的时候可能会报错，因此在文件复制完成时添加用户写权限，让去重/删除可以顺利往下走
    if (result.count() > 0 && result[0].path().contains(DISCBURN_CACHE_MID_PATH)) {
        QString strFilePath = result[0].path();
        static QRegularExpression reg("/_dev_sr[0-9]*/"); // 正则匹配 _dev_srN  N为任意数字，考虑可能接入多光驱的情况
        QRegularExpressionMatch match = reg.match(strFilePath);
        if (match.hasMatch()) {
            QString strTag = match.captured();
            QString strStagingPath = strFilePath.mid(0, strFilePath.indexOf(strTag) + strTag.length());
            QProcess::execute("chmod -R u+w " + strStagingPath); // 之前尝试使用 DLocalFileHandler 去处理权限问题但是会失败，因此这里采用命令去更改权限
        }
    }
}

bool DFileService::isSmbFtpContain(const QUrl &url)
{
    QMutexLocker lk(&d_ptr->smbftpmutex);
    return d_ptr->m_rootsmbftpurllist.contains(url);
}

void DFileService::onTagEditorChanged(const QStringList &tags, const QList<QUrl> &files)
{
    Q_D(DFileService);

    //暂停timer
    d->m_tagEditorChangeTimer.stop();

    //重设tag数据和files
    d->m_tagEditorTags.clear();
    d->m_tagEditorTags.append(tags);
    d->m_tagEditorFiles.clear();
    d->m_tagEditorFiles.append(files);

    //开始timer计时，500毫秒内连续的editor被编辑改变合并为一次改变
    d->m_tagEditorChangeTimer.start(500);
}

QList<DAbstractFileController *> DFileService::getHandlerTypeByUrl(const QUrl &fileUrl, bool ignoreHost, bool ignoreScheme)
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

QString DFileService::getSymlinkFileName(const QUrl &fileUrl, const QDir &targetDir)
{
    DFMLocalFileInfo info(fileUrl);

    if (info.exists()) {
        QString baseName = info.fileName();
        QString shortcut = QObject::tr("Shortcut");
        QString linkBaseName;

        int number = 1;

        Q_FOREVER {
            if (info.isFile()) {
                if (info.suffix().isEmpty()) {
                    if (number == 1) {

                        linkBaseName = QString("%1 %2").arg(baseName, shortcut);
                    } else {
                        linkBaseName = QString("%1 %2%3").arg(baseName, shortcut, QString::number(number));
                    }
                } else {
                    if (number == 1) {
                        linkBaseName = QString("%1 %2.%3").arg(baseName, shortcut, info.suffix());
                    } else {
                        linkBaseName = QString("%1 %2%3.%4").arg(baseName, shortcut, QString::number(number), info.suffix());
                    }
                }
            } else if (info.isDir()) {
                if (number == 1) {
                    linkBaseName = QString("%1 %2").arg(baseName, shortcut);
                } else {
                    linkBaseName = QString("%1 %2%3").arg(baseName, shortcut, QString::number(number));
                }
            } else if (info.isSymLink()) {
                return QString();
            }

            if (targetDir.path().isEmpty()) {
                return linkBaseName;
            }

            if (targetDir.exists(linkBaseName)) {
                ++number;
            } else {
                //链接文件失效后exists会返回false，通过lstat再次判断链接文件本身是否存在
                auto strLinkPath = targetDir.filePath(linkBaseName).toStdString();
                struct stat st;
                if ((lstat(strLinkPath.c_str(), &st) == 0) && S_ISLNK(st.st_mode))
                    ++number;
                else
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
//    FileSignalManager *manager = fileSignalManager;

//    TIMER_SINGLESHOT_OBJECT(manager, qMax(event.fileUrlList().count() * (10 + event.fileUrlList().count() / 150), 200), {
//                                manager->requestSelectFile(event);
//                            }, event, manager)
}

///###: replace
bool DFileService::multiFilesReplaceName(const QList<QUrl> &urls, const QPair<QString, QString> &pair)const
{
    //    auto alteredAndUnAltereQUrls = FileBatchProcess::instance()->replaceText(urls, pair);
    //    AppController::multiSelectionFilesCache.first = FileBatchProcess::batchProcessFile(alteredAndUnAltereQUrls).values();

    return DFileService::checkMultiSelectionFilesCache();
}


///###: add
bool DFileService::multiFilesAddStrToName(const QList<QUrl> &urls, const QPair<QString, DFileService::AddTextFlags> &pair)const
{
    //    auto alteredAndUnAltereQUrls = FileBatchProcess::instance()->addText(urls, pair);
    //    AppController::multiSelectionFilesCache.first = FileBatchProcess::batchProcessFile(alteredAndUnAltereQUrls).values();

    return DFileService::checkMultiSelectionFilesCache();
}

///###: customize
bool DFileService::multiFilesCustomName(const QList<QUrl> &urls, const QPair<QString, QString> &pair)const
{
    //    auto alteredAndUnAltereQUrls = FileBatchProcess::instance()->customText(urls, pair);
    //    AppController::multiSelectionFilesCache.first = FileBatchProcess::batchProcessFile(alteredAndUnAltereQUrls).values();

    return DFileService::checkMultiSelectionFilesCache();
}

///###: helper function.
bool DFileService::checkMultiSelectionFilesCache()
{
//    if (AppController::multiSelectionFilesCache.first.isEmpty()) {
//        return false;
//    }

    return true;
}

//打印函数的调用堆栈
void DFileService::printStacktrace(int level)
{
    int size = 16;
    void *array[16];
    int stack_num = backtrace(array, size);
    char **stacktrace = backtrace_symbols(array, stack_num);
    int total = std::min(level, stack_num);
    for (int i = 0; i < total; ++i) {
        printf("%s\n", stacktrace[i]);
    }
    free(stacktrace);
}
