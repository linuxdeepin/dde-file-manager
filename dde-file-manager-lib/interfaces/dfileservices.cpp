#include "dfileservices.h"
#include "dabstractfilecontroller.h"
#include "dabstractfileinfo.h"
#include "dabstractfilewatcher.h"
#include "dfmeventdispatcher.h"

#include "dfmfilecontrollerfactory.h"

#include "app/filesignalmanager.h"
#include "dfmevent.h"
#include "app/define.h"
#include "controllers/jobcontroller.h"
#include "controllers/appcontroller.h"
#include "views/windowmanager.h"
#include "dfileinfo.h"
#include "models/trashfileinfo.h"
#include "models/desktopfileinfo.h"
#include "controllers/pathmanager.h"
#include "dfmstandardpaths.h"
#include "views/windowmanager.h"
#include "dfmsetting.h"
#include "models/avfsfileinfo.h"

#include "shutil/fileutils.h"

#include "dialogs/dialogmanager.h"

#include "deviceinfo/udisklistener.h"
#include "interfaces/dfmglobal.h"
#include "singleton.h"

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

DWIDGET_USE_NAMESPACE

class DFileServicePrivate
{
public:
    static QMultiHash<const HandlerType, DAbstractFileController*> controllerHash;
    static QHash<const DAbstractFileController*, HandlerType> handlerHash;
    static QMultiHash<const HandlerType, HandlerCreatorType> controllerCreatorHash;
};

QMultiHash<const HandlerType, DAbstractFileController*> DFileServicePrivate::controllerHash;
QHash<const DAbstractFileController*, HandlerType> DFileServicePrivate::handlerHash;
QMultiHash<const HandlerType, HandlerCreatorType> DFileServicePrivate::controllerCreatorHash;

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
}

DFileService::~DFileService()
{

}

template<typename T>
QVariant eventProcess(DFileService *service, const QSharedPointer<DFMEvent> &event, T function)
{
    QSet<DAbstractFileController*> controller_set;
    QSet<QString> scheme_set;

    for (const DUrl &url : event->handleUrlList()) {
        QList<DAbstractFileController*> list = service->getHandlerTypeByUrl(url);

        if (!scheme_set.contains(url.scheme()))
            list << service->getHandlerTypeByUrl(url, true);
        else
            scheme_set << url.scheme();

        for (DAbstractFileController *controller : list) {
            if (controller_set.contains(controller))
                continue;

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
        if (df.getDeepinId() == "dde-computer")
            urlList.removeOne(DesktopFileInfo::computerDesktopFileUrl());
    }

    if (urlList.contains(DesktopFileInfo::trashDesktopFileUrl())) {
        DesktopFile df(DesktopFileInfo::trashDesktopFileUrl().toLocalFile());
        if (df.getDeepinId() == "dde-trash")
            urlList.removeOne(DesktopFileInfo::trashDesktopFileUrl());
    }

    event->setData(urlList);
    break;
    }
    default: break;
    }
#endif

#define CALL_CONTROLLER(Fun)\
    eventProcess(this, event, &DAbstractFileController::Fun);

    QVariant result;

    switch (event->type()) {
    case DFMEvent::OpenFile:
        result = CALL_CONTROLLER(openFile);
        break;
    case DFMEvent::OpenFileByApp:
        result = CALL_CONTROLLER(openFileByApp);
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

        if (f->exists()) {
            DThreadUtil::runInMainThread(dialogManager, &DialogManager::showRenameNameSameErrorDialog, f->fileDisplayName(), *event.data());
            result = false;
        } else {
            result = CALL_CONTROLLER(renameFile);

            if (result.toBool() && event->isAccepted()) {
                DFMUrlListBaseEvent newEvent(e->sender(), DUrlList() << e->toUrl());

                newEvent.setWindowId(e->windowId());

                TIMER_SINGLESHOT(200, {
                    emit fileSignalManager->requestSelectFile(newEvent);
                }, newEvent);

                result = true;
            }
        }

        break;
    }
    case DFMEvent::DeleteFiles: {
        bool slient = event.staticCast<DFMDeleteEvent>()->silent();

        foreach (const DUrl& url, event->fileUrlList()) {
            if (systemPathManager->isSystemPath(url.toLocalFile())) {
                if (!slient)
                    DThreadUtil::runInMainThread(dialogManager, &DialogManager::showDeleteSystemPathWarnDialog, event->windowId());
                result = false;
                goto end;
            }
        }

        if (slient || DThreadUtil::runInMainThread(dialogManager, &DialogManager::showDeleteFilesClearTrashDialog, DFMUrlListBaseEvent(event->sender(), event->fileUrlList())) == 1) {
            result = CALL_CONTROLLER(deleteFiles);
        } else {
            result = false;
        }

        break;
    }
    case DFMEvent::MoveToTrash: {
        //handle system files should not be able to move to trash
        foreach (const DUrl& url, event->fileUrlList()) {
            if (systemPathManager->isSystemPath(url.toLocalFile())){
                DThreadUtil::runInMainThread(dialogManager, &DialogManager::showDeleteSystemPathWarnDialog, event->windowId());
                result = QVariant::fromValue(event->fileUrlList());
                goto end;
            }
        }

        //handle files whom could not be moved to trash
        DUrlList enableList;
        foreach (const DUrl& url, event->fileUrlList()) {
            const DAbstractFileInfoPointer& info = createFileInfo(this, url);
            if(info->isDir() && !info->isWritable())
                continue;

            enableList << url;
        }

        event->setData(enableList);
        result = CALL_CONTROLLER(moveToTrash);

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

        break;
    }
    case DFMEvent::NewFolder:
        result = CALL_CONTROLLER(newFolder);
        break;
    case DFMEvent::NewFile:
        result = CALL_CONTROLLER(newFile);
        break;
    case DFMEvent::OpenFileLocation:
        result = CALL_CONTROLLER(openFileLocation);
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
    default:
        return false;
    }

end:
    if (resultData)
        *resultData = result;

    return true;
}

bool DFileService::isRegisted(const QString &scheme, const QString &host, const std::type_info &info)
{
    const HandlerType &type = HandlerType(scheme, host);

    foreach (const HandlerCreatorType &value, DFileServicePrivate::controllerCreatorHash.values(type)) {
        if (value.first == info.name())
            return true;
    }

    foreach (const DAbstractFileController *controller, DFileServicePrivate::controllerHash.values(type)) {
        if (typeid(*controller) == info)
            return true;
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
    if (DFileServicePrivate::handlerHash.contains(controller))
        return true;

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
    if (!DFileServicePrivate::handlerHash.contains(controller))
        return;

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

bool DFileService::openFileByApp(const QObject *sender, const QString &appName, const DUrl &url) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenFileByAppEvent>(sender, appName, url)).toBool();
}

bool DFileService::compressFiles(const QObject *sender, const DUrlList &list) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMCompressEvnet>(sender, list)).toBool();
}

bool DFileService::decompressFile(const QObject *sender, const DUrlList &list) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMDecompressEvnet>(sender, list)).toBool();
}

bool DFileService::decompressFileHere(const QObject *sender, const DUrlList &list) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMDecompressHereEvnet>(sender, list)).toBool();
}

bool DFileService::writeFilesToClipboard(const QObject *sender, DFMGlobal::ClipboardAction action, const DUrlList &list) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(sender, action, list)).toBool();
}

bool DFileService::renameFile(const QObject *sender, const DUrl &from, const DUrl &to) const
{
    return DFMEventDispatcher::instance()->processEvent<DFMRenameEvent>(sender, from, to).toBool();
}

bool DFileService::deleteFiles(const QObject *sender, const DUrlList &list, bool slient) const
{
    return DFMEventDispatcher::instance()->processEventWithEventLoop(dMakeEventPointer<DFMDeleteEvent>(sender, list, slient)).toBool();
}

DUrlList DFileService::moveToTrash(const QObject *sender, const DUrlList &list) const
{
    if (list.isEmpty())
        return list;

    if (FileUtils::isGvfsMountFile(list.first().toLocalFile())) {
        deleteFiles(sender, list);
        return list;
    }

    return qvariant_cast<DUrlList>(DFMEventDispatcher::instance()->processEventWithEventLoop(dMakeEventPointer<DFMMoveToTrashEvent>(sender, list)));
}

void DFileService::pasteFileByClipboard(const QObject *sender, const DUrl &targetUrl) const
{
    DFMGlobal::ClipboardAction action = DFMGlobal::instance()->clipboardAction();

    if (action == DFMGlobal::UnknowAction)
        return;

    pasteFile(sender, action, targetUrl, DUrl::fromQUrlList(DFMGlobal::instance()->clipboardFileUrlList()));
}

DUrlList DFileService::pasteFile(const QObject *sender, DFMGlobal::ClipboardAction action, const DUrl &targetUrl, const DUrlList &list) const
{
    const QSharedPointer<DFMPasteEvent> &event = dMakeEventPointer<DFMPasteEvent>(sender, action, targetUrl, list);
    return qvariant_cast<DUrlList>(DFMEventDispatcher::instance()->processEventWithEventLoop(event));
}

bool DFileService::restoreFile(const QObject *sender, const DUrlList &list) const
{
    return DFMEventDispatcher::instance()->processEventWithEventLoop<DFMRestoreFromTrashEvent>(sender, list).toBool();
}

bool DFileService::newFolder(const QObject *sender, const DUrl &targetUrl) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMNewFolderEvent>(sender, targetUrl)).toBool();
}

bool DFileService::newFile(const QObject *sender, const DUrl &targetUrl, const QString &fileSuffix) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMNewFileEvent>(sender, targetUrl, fileSuffix)).toBool();
}

bool DFileService::openFileLocation(const QObject *sender, const DUrl &url) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenFileLocation>(sender, url)).toBool();
}

bool DFileService::createSymlink(const QObject *sender, const DUrl &fileUrl) const
{
    QString linkName = getSymlinkFileName(fileUrl);
    QString linkPath = QFileDialog::getSaveFileName(qobject_cast<const QWidget*>(sender) ? qobject_cast<const QWidget*>(sender)->window() : Q_NULLPTR,
                                                    QObject::tr("Create symlink"), linkName);
    //handle for cancel select file
    if (linkPath.isEmpty())
        return false;

    return createSymlink(sender, fileUrl, DUrl::fromLocalFile(linkPath));
}

bool DFileService::createSymlink(const QObject *sender, const DUrl &fileUrl, const DUrl &linkToUrl) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMCreateSymlinkEvent>(sender, fileUrl, linkToUrl)).toBool();
}

bool DFileService::sendToDesktop(const QObject *sender, const DUrlList &urlList) const
{
    const QString &desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    if (desktopPath.isEmpty())
        return false;

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
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMFileShareEvnet>(sender, fileUrl, name, isWritable, allowGuest)).toBool();
}

bool DFileService::unShareFolder(const QObject *sender, const DUrl &fileUrl) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMCancelFileShareEvent>(sender, fileUrl)).toBool();
}

bool DFileService::openInTerminal(const QObject *sender, const DUrl &fileUrl) const
{
    return DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMOpenInTerminalEvent>(sender, fileUrl)).toBool();
}

const DAbstractFileInfoPointer DFileService::createFileInfo(const QObject *sender, const DUrl &fileUrl) const
{
    const DAbstractFileInfoPointer &info = DAbstractFileInfo::getFileInfo(fileUrl);

    if (info)
        return info;

    const auto&& event = dMakeEventPointer<DFMCreateFileInfoEvnet>(sender, fileUrl);

    return qvariant_cast<DAbstractFileInfoPointer>(DFMEventDispatcher::instance()->processEvent(event));
}

const DDirIteratorPointer DFileService::createDirIterator(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                                          QDir::Filters filters, QDirIterator::IteratorFlags flags) const
{
    const auto&& event = dMakeEventPointer<DFMCreateDiriterator>(sender, fileUrl, nameFilters, filters, flags);

    return qvariant_cast<DDirIteratorPointer>(DFMEventDispatcher::instance()->processEvent(event));
}

const QList<DAbstractFileInfoPointer> DFileService::getChildren(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                                               QDir::Filters filters, QDirIterator::IteratorFlags flags)
{
    const auto&& event = dMakeEventPointer<DFMGetChildrensEvent>(sender, fileUrl, nameFilters, filters, flags);

    return qvariant_cast<QList<DAbstractFileInfoPointer>>(DFMEventDispatcher::instance()->processEvent(event));
}

JobController *DFileService::getChildrenJob(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                            QDir::Filters filters, QDirIterator::IteratorFlags flags) const
{
    const auto&& event = dMakeEventPointer<DFMCreateGetChildrensJob>(sender, fileUrl, nameFilters, filters, flags);

    return qvariant_cast<JobController*>(DFMEventDispatcher::instance()->processEvent(event));
}

DAbstractFileWatcher *DFileService::createFileWatcher(const QObject *sender, const DUrl &fileUrl, QObject *parent) const
{
    DAbstractFileWatcher *w = qvariant_cast<DAbstractFileWatcher*>(DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMCreateFileWatcherEvent>(sender, fileUrl)));

    if (w)
        w->setParent(parent);

    return w;
}

QList<DAbstractFileController*> DFileService::getHandlerTypeByUrl(const DUrl &fileUrl, bool ignoreHost, bool ignoreScheme)
{
    HandlerType handlerType(ignoreScheme ? "" : fileUrl.scheme(), ignoreHost ? "" : fileUrl.host());

    if (DFileServicePrivate::controllerCreatorHash.contains(handlerType)) {
        QList<DAbstractFileController*> list = DFileServicePrivate::controllerHash.values(handlerType);

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
    const DAbstractFileInfoPointer &pInfo =  instance()->createFileInfo(Q_NULLPTR, fileUrl);

    if (pInfo->exists()) {
        QString baseName = pInfo->baseName();
        QString shortcut = QObject::tr("Shortcut");
        QString linkBaseName;

        int number = 1;

        forever {
            if (pInfo->isFile()) {
                if (number == 1) {
                    linkBaseName = QString("%1 %2.%3").arg(baseName, shortcut, pInfo->suffix());
                } else {
                    linkBaseName = QString("%1 %2%3.%4").arg(baseName, shortcut, QString::number(number), pInfo->suffix());
                }
            } else if (pInfo->isDir()) {
                if (number == 1) {
                    linkBaseName = QString("%1 %2").arg(baseName, shortcut);
                }else{
                    linkBaseName = QString("%1 %2%3").arg(baseName, shortcut, QString::number(number));
                }
            } else if (pInfo->isSymLink()) {
                return QString();
            }

            if (targetDir.path().isEmpty())
                return linkBaseName;

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

    TIMER_SINGLESHOT_OBJECT(manager, 200, {
                                manager->requestSelectFile(event);
                            }, event, manager)
}
