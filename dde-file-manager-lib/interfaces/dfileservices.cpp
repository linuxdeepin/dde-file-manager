#include "dfileservices.h"
#include "dabstractfilecontroller.h"
#include "dabstractfileinfo.h"
#include "dabstractfilewatcher.h"

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
#include "widgets/singleton.h"

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
#include <QProcess>

DWIDGET_USE_NAMESPACE

#define FILTER(type)\
    ({bool ok = true;\
    if (type >= OpenFile) {\
        if (d_func()->whitelist >= OpenFile && !d_func()->whitelist.testFlag(type))\
            ok = false;\
        else if (d_func()->blacklist.testFlag(type)) \
            ok = false;\
    } ok;})
#define FILTER_RETURN(type, value...)\
    if (!FILTER(type)) return value;

#define TRAVERSE(url, Code)\
    do {\
        {const char *function_name = ({QByteArray name(__FUNCTION__); name[0] = name.at(0) & char(0xdf); name.data();});\
        FileOperatorType type = (FileOperatorType)d_func()->fileOperatorTypeEnum.keyToValue(function_name);\
        if (!FILTER(type)) break;}\
        QList<DAbstractFileController*> &&list = getHandlerTypeByUrl(url);\
        bool accepted = false;\
        for(DAbstractFileController *controller : list) {\
            Code\
        }\
        list = getHandlerTypeByUrl(url, true);\
        for(DAbstractFileController *controller : list) {\
            Code\
        }\
    } while(false);

class DFileServicePrivate
{
public:
    static QMultiHash<const HandlerType, DAbstractFileController*> controllerHash;
    static QHash<const DAbstractFileController*, HandlerType> handlerHash;
    static QMultiHash<const HandlerType, HandlerCreatorType> controllerCreatorHash;

    DFileService::FileOperatorTypes whitelist;
    DFileService::FileOperatorTypes blacklist;
    QMetaEnum fileOperatorTypeEnum;

    static QVariant processEventByController(DAbstractFileController *controller, const QSharedPointer<DFMEvent> &event);
};

QMultiHash<const HandlerType, DAbstractFileController*> DFileServicePrivate::controllerHash;
QHash<const DAbstractFileController*, HandlerType> DFileServicePrivate::handlerHash;
QMultiHash<const HandlerType, HandlerCreatorType> DFileServicePrivate::controllerCreatorHash;

QVariant DFileServicePrivate::processEventByController(DAbstractFileController *controller, const QSharedPointer<DFMEvent> &event)
{
    switch (event->type()) {
    case DFMEvent::OpenFile:
        return controller->openFile(event.dynamicCast<DFMOpenFileEvent>());
    case DFMEvent::OpenFileByApp:
        return controller->openFileByApp(event.dynamicCast<DFMOpenFileByAppEvent>());
    case DFMEvent::CompressFiles:
        return controller->compressFiles(event.dynamicCast<DFMCompressEvnet>());
    case DFMEvent::DecompressFile:
        return controller->decompressFile(event.dynamicCast<DFMDecompressEvnet>());
    case DFMEvent::DecompressFileHere:
        return controller->decompressFileHere(event.dynamicCast<DFMDecompressHereEvnet>());
    case DFMEvent::WriteUrlsToClipboard:
        return controller->writeFilesToClipboard(event.dynamicCast<DFMWriteUrlsToClipboardEvent>());
    case DFMEvent::RenameFile:
        return controller->renameFile(event.dynamicCast<DFMRenameEvent>());
    case DFMEvent::DeleteFiles:
        return controller->deleteFiles(event.dynamicCast<DFMDeleteEvent>());
    case DFMEvent::MoveToTrash:
        return QVariant::fromValue(controller->moveToTrash(event.dynamicCast<DFMMoveToTrashEvent>()));
    case DFMEvent::PasteFile:
        return QVariant::fromValue(controller->pasteFile(event.dynamicCast<DFMPasteEvent>()));
    case DFMEvent::NewFolder:
        return controller->newFolder(event.dynamicCast<DFMNewFolderEvent>());
    case DFMEvent::NewFile:
        return controller->newFile(event.dynamicCast<DFMNewFileEvent>());
    case DFMEvent::OpenFileLocation:
        return controller->openFileLocation(event.dynamicCast<DFMOpenFileLocation>());
    case DFMEvent::CreateSymlink:
        return controller->createSymlink(event.dynamicCast<DFMCreateSymlinkEvent>());
    case DFMEvent::FileShare:
        return controller->shareFolder(event.dynamicCast<DFMFileShareEvnet>());
    case DFMEvent::CancelFileShare:
        return controller->unShareFolder(event.dynamicCast<DFMCancelFileShareEvent>());
    case DFMEvent::OpenInTerminal:
        return controller->openInTerminal(event.dynamicCast<DFMOpenInTerminalEvent>());
    case DFMEvent::GetChildrens:
        return QVariant::fromValue(controller->getChildren(event.dynamicCast<DFMGetChildrensEvent>()));
    case DFMEvent::CreateFileInfo:
        return QVariant::fromValue(controller->createFileInfo(event.dynamicCast<DFMCreateFileInfoEvnet>()));
    case DFMEvent::CreateDiriterator:
        return QVariant::fromValue(controller->createDirIterator(event.dynamicCast<DFMCreateDiriterator>()));
    case DFMEvent::CreateGetChildrensJob: {
        const QSharedPointer<DFMCreateGetChildrensJob> &e = event.dynamicCast<DFMCreateGetChildrensJob>();
        const DDirIteratorPointer &iterator = controller->createDirIterator(e.staticCast<DFMCreateDiriterator>());

        if (iterator)
            return QVariant::fromValue(new JobController(e->fileUrl(), iterator, DFileService::instance()));

        return QVariant();
    }
    case DFMEvent::CreateFileWatcher:
        return QVariant::fromValue(controller->createFileWatcher(event.dynamicCast<DFMCreateFileWatcherEvent>()));
    default:
        return QVariant();
    }
}

DFileService::DFileService(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFileServicePrivate())
{
    d_ptr->fileOperatorTypeEnum = metaObject()->enumerator(metaObject()->indexOfEnumerator(QT_STRINGIFY(FileOperatorType)));

    /// init url handler register
    AppController::registerUrlHandle();
}

DFileService::~DFileService()
{

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
    if(!DFileServicePrivate::handlerHash.contains(controller))
        return;

    DFileServicePrivate::controllerHash.remove(DFileServicePrivate::handlerHash.value(controller), controller);
}

void DFileService::clearFileUrlHandler(const QString &scheme, const QString &host)
{
    const HandlerType handler(scheme, host);

    DFileServicePrivate::controllerHash.remove(handler);
    DFileServicePrivate::controllerCreatorHash.remove(handler);
}

void DFileService::setFileOperatorWhitelist(FileOperatorTypes list)
{
    Q_D(DFileService);

    d->whitelist = list;
}

DFileService::FileOperatorTypes DFileService::fileOperatorWhitelist() const
{
    Q_D(const DFileService);

    return d->whitelist;
}

void DFileService::setFileOperatorBlacklist(FileOperatorTypes list)
{
    Q_D(DFileService);

    d->blacklist = list;
}

DFileService::FileOperatorTypes DFileService::fileOperatorBlacklist() const
{
    Q_D(const DFileService);

    return d->blacklist;
}

bool DFileService::openFile(const DUrl &url, const QObject *sender) const
{
    return processEventSync(dMakeEventPointer<DFMOpenFileEvent>(url, sender)).toBool();
}

bool DFileService::openFileByApp(const QString &appName, const DUrl &url, const QObject *sender) const
{
    return processEventSync(dMakeEventPointer<DFMOpenFileByAppEvent>(appName, url, sender)).toBool();
}

bool DFileService::compressFiles(const DUrlList &list, const QObject *sender) const
{
    return processEventSync(dMakeEventPointer<DFMCompressEvnet>(list, sender)).toBool();
}

bool DFileService::decompressFile(const DUrlList &list, const QObject *sender) const
{
    return processEventSync(dMakeEventPointer<DFMDecompressEvnet>(list, sender)).toBool();
}

bool DFileService::decompressFileHere(const DUrlList &list, const QObject *sender) const
{
    return processEventSync(dMakeEventPointer<DFMDecompressHereEvnet>(list, sender)).toBool();
}

bool DFileService::writeFilesToClipboard(DFMGlobal::ClipboardAction action, const DUrlList &list, const QObject *sender) const
{
#ifdef DDE_COMPUTER_TRASH
    DUrlList urlList = event->urlList();

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
#endif

    return processEventSync(dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(action, list, sender)).toBool();
}

bool DFileService::renameFile(const DUrl &from, const DUrl &to, const QObject *sender) const
{
    FILTER_RETURN(RenameFile, false)

    const DAbstractFileInfoPointer &f = createFileInfo(to);

    const QSharedPointer<DFMRenameEvent> &event = dMakeEventPointer<DFMRenameEvent>(from, to, sender);

    if (f->exists()) {
        dialogManager->showRenameNameSameErrorDialog(f->fileDisplayName(), *event.data());
        return false;
    }

    bool ok = processEventSync(event).toBool();

    if (ok && event->isAccepted()) {
        DFMEvent e = *event.data();

        e << (DUrlList() << to);

        TIMER_SINGLESHOT(200, {
            emit fileSignalManager->requestSelectFile(e);
        }, e);

        return true;
    }

    return false;
}

bool DFileService::deleteFiles(const DUrlList &list, const QObject *sender) const
{
    FILTER_RETURN(DeleteFiles, false)

    DUrlList urlList = list;
#ifdef DDE_COMPUTER_TRASH
    if(urlList.contains(DesktopFileInfo::computerDesktopFileUrl())){
        DesktopFile df(DesktopFileInfo::computerDesktopFileUrl().toLocalFile());
        if(df.getDeepinId() == "dde-computer")
            urlList.removeOne(DesktopFileInfo::computerDesktopFileUrl());
    }

    if(urlList.contains(DesktopFileInfo::trashDesktopFileUrl())){
        DesktopFile df(DesktopFileInfo::trashDesktopFileUrl().toLocalFile());
        if(df.getDeepinId() == "dde-trash")
            urlList.removeOne(DesktopFileInfo::trashDesktopFileUrl());
    }
#endif

    if (urlList.isEmpty())
        return true;

    foreach (const DUrl& url, urlList) {
        if(systemPathManager->isSystemPath(url.toLocalFile())){
            dialogManager->showDeleteSystemPathWarnDialog();
            return false;
        }
    }

    if (QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return false;
    }

    DFMEvent e;

    e << urlList;

    int result = dialogManager->showDeleteFilesClearTrashDialog(e);

    if (result == 1) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileService::deleteFilesSync, urlList, sender);
        return true;
    }

    return false;
}

bool DFileService::deleteFilesSync(const DUrlList &list, const QObject *sender) const
{
    return processEventSync(dMakeEventPointer<DFMDeleteEvent>(list, sender)).toBool();
}

void DFileService::moveToTrash(const DUrlList &list, const QObject *sender) const
{
    FILTER_RETURN(MoveToTrash)

    DUrlList urlList = list;
#ifdef DDE_COMPUTER_TRASH
    if(urlList.contains(DesktopFileInfo::computerDesktopFileUrl())){
        DesktopFile df(DesktopFileInfo::computerDesktopFileUrl().toLocalFile());
        if(df.getDeepinId() == "dde-computer")
            urlList.removeOne(DesktopFileInfo::computerDesktopFileUrl());
    }

    if(urlList.contains(DesktopFileInfo::trashDesktopFileUrl())){
        DesktopFile df(DesktopFileInfo::trashDesktopFileUrl().toLocalFile());
        if(df.getDeepinId() == "dde-trash")
            urlList.removeOne(DesktopFileInfo::trashDesktopFileUrl());
    }
#endif

    if (urlList.isEmpty())
        return;

    //handle system files should not be able to move to trash
    foreach (const DUrl& url, urlList) {
        if(systemPathManager->isSystemPath(url.toLocalFile())){
            dialogManager->showDeleteSystemPathWarnDialog();
            return;
        }
    }

    if (FileUtils::isGvfsMountFile(urlList.first().toLocalFile())){
        deleteFiles(urlList, sender);
        return;
    }

    //handle files whom could not be moved to trash
    DUrlList enableList;
    foreach (const DUrl& url, urlList) {
        const DAbstractFileInfoPointer& info = createFileInfo(url);
        if(info->isDir() && !info->isWritable())
            continue;

        enableList << url;
    }

    urlList = enableList;

    if (QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if (QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileService::moveToTrashSync, urlList, sender);

        return;
    }

    moveToTrashSync(urlList, sender);
}

DUrlList DFileService::moveToTrashSync(const DUrlList &list, const QObject *sender) const
{
    return qvariant_cast<DUrlList>(processEventSync(dMakeEventPointer<DFMMoveToTrashEvent>(list, sender)));
}

void DFileService::pasteFileByClipboard(const DUrl &targetUrl, const QObject *sender) const
{
    FILTER_RETURN(PasteFileByClipboard)

    DFMGlobal::ClipboardAction action = DFMGlobal::instance()->clipboardAction();

    if (action == DFMGlobal::UnknowAction)
        return;

    pasteFile(action, targetUrl, DUrl::fromQUrlList(DFMGlobal::instance()->clipboardFileUrlList()), sender);
}

void DFileService::pasteFile(DFMGlobal::ClipboardAction action, const DUrl &targetUrl, const DUrlList &list, const QObject *sender) const
{
    if (QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if (QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileService::pasteFile, action, targetUrl, list, sender);

        return;
    }

    const QSharedPointer<DFMPasteEvent> &event = dMakeEventPointer<DFMPasteEvent>(action, targetUrl, list, sender);
    const DUrlList &result = qvariant_cast<DUrlList>(processEventSync(event));

    if (event->isAccepted()) {
        DFMEvent e = *event.data();

        e << result;

        metaObject()->invokeMethod(const_cast<DFileService*>(this), "laterRequestSelectFiles",
                                   Qt::QueuedConnection, Q_ARG(DFMEvent, e));

        return;
    }
}

void DFileService::restoreFile(const DUrl &srcUrl, const DUrl &tarUrl, const DFMEvent &event) const
{
    if(QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if(QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileService::restoreFile, srcUrl, tarUrl, event);

        return;
    }
    TRAVERSE(srcUrl, {
                 controller->restoreFile(srcUrl, tarUrl, event, accepted);

                 if(accepted)
                 return;
             })
}

bool DFileService::newFolder(const DUrl &targetUrl, const QObject *sender) const
{
    return processEventSync(dMakeEventPointer<DFMNewFolderEvent>(targetUrl, sender)).toBool();
}

bool DFileService::newFile(const DUrl &targetUrl, const QString &fileSuffix, const QObject *sender) const
{
    return processEventSync(dMakeEventPointer<DFMNewFileEvent>(targetUrl, fileSuffix, sender)).toBool();
}

bool DFileService::openFileLocation(const DUrl &url, const QObject *sender) const
{
    return processEventSync(dMakeEventPointer<DFMOpenFileLocation>(url, sender)).toBool();
}

bool DFileService::createSymlink(const DUrl &fileUrl, const QObject *sender) const
{
    FILTER_RETURN(CreateSymlink, false)

    QString linkName = getSymlinkFileName(fileUrl);
    QString linkPath = QFileDialog::getSaveFileName(qobject_cast<const QWidget*>(sender) ? qobject_cast<const QWidget*>(sender)->window() : Q_NULLPTR,
                                                    QObject::tr("Create symlink"), linkName);
    //handle for cancel select file
    if (linkPath.isEmpty())
        return false;

    Q_D(const DFileService);

    DFileServicePrivate *ptr = const_cast<DFileServicePrivate*>(d);

    if (ptr->whitelist >= OpenFile) {
        ptr->whitelist |= CreateSymlink;
    } else {
        ptr->blacklist &= (~CreateSymlink);
    }

    return createSymlink(fileUrl, DUrl::fromLocalFile(linkPath), sender);
}

bool DFileService::createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl, const QObject *sender) const
{
    return processEventSync(dMakeEventPointer<DFMCreateSymlinkEvent>(fileUrl, linkToUrl, sender)).toBool();
}

bool DFileService::sendToDesktop(const DUrlList &urlList, const QObject *sender) const
{
    const QString &desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    if (desktopPath.isEmpty())
        return false;

    const QDir &desktopDir(desktopPath);
    bool ok = true;

    for (const DUrl &url : urlList) {
        const QString &linkName = getSymlinkFileName(url, desktopDir);

        ok = ok && createSymlink(url, DUrl::fromLocalFile(desktopDir.filePath(linkName)), sender);
    }

    return ok;
}

bool DFileService::shareFolder(const DUrl &fileUrl, const QString &name, bool isWritable, bool allowGuest, const QObject *sender)
{
    return processEventSync(dMakeEventPointer<DFMFileShareEvnet>(fileUrl, name, isWritable, allowGuest, sender)).toBool();
}

bool DFileService::unShareFolder(const DUrl &fileUrl, const QObject *sender) const
{
    return processEventSync(dMakeEventPointer<DFMCancelFileShareEvent>(fileUrl, sender)).toBool();
}

bool DFileService::openInTerminal(const DUrl &fileUrl, const QObject *sender) const
{
    return processEventSync(dMakeEventPointer<DFMOpenInTerminalEvent>(fileUrl, sender)).toBool();
}

void DFileService::openNewWindow(const DFMEvent &event, const bool &isNewWindow) const
{
    foreach (const DUrl& url, event.fileUrlList()) {
        emit fileSignalManager->requestOpenNewWindowByUrl(url, isNewWindow);
    }
}

void DFileService::openInCurrentWindow(const DFMEvent &event) const
{
    const DAbstractFileInfoPointer &fileInfo = createFileInfo(event.fileUrl());

    if (fileInfo && fileInfo->isDir()){
        emit fileSignalManager->requestChangeCurrentUrl(event);
    }
}

const DAbstractFileInfoPointer DFileService::createFileInfo(const DUrl &fileUrl, const QObject *sender) const
{
    const DAbstractFileInfoPointer &info = DAbstractFileInfo::getFileInfo(fileUrl);

    if (info)
        return info;

    const auto&& event = dMakeEventPointer<DFMCreateFileInfoEvnet>(fileUrl, sender);

    return qvariant_cast<DAbstractFileInfoPointer>(processEventSync(event));
}

const DDirIteratorPointer DFileService::createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                          QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                          const QObject *sender) const
{
    const auto&& event = dMakeEventPointer<DFMCreateDiriterator>(fileUrl, nameFilters, filters, flags, sender);

    return qvariant_cast<DDirIteratorPointer>(processEventSync(event));
}

const QList<DAbstractFileInfoPointer> DFileService::getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                               QDir::Filters filters, QDirIterator::IteratorFlags flags, const QObject *sender)
{
    const auto&& event = dMakeEventPointer<DFMGetChildrensEvent>(fileUrl, nameFilters, filters, flags, sender);

    return qvariant_cast<QList<DAbstractFileInfoPointer>>(processEventSync(event));
}

JobController *DFileService::getChildrenJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                            QDir::Filters filters, QDirIterator::IteratorFlags flags, const QObject *sender) const
{
    const auto&& event = dMakeEventPointer<DFMCreateGetChildrensJob>(fileUrl, nameFilters, filters, flags, sender);

    return qvariant_cast<JobController*>(processEventSync(event));
}

DAbstractFileWatcher *DFileService::createFileWatcher(const DUrl &fileUrl, QObject *parent, const QObject *sender) const
{
    DAbstractFileWatcher *w = qvariant_cast<DAbstractFileWatcher*>(processEventSync(dMakeEventPointer<DFMCreateFileWatcherEvent>(fileUrl, sender)));

    if (w)
        w->setParent(parent);

    return w;
}

bool DFileService::isAvfsMounted() const
{
    QProcess p;
    QString cmd = "/bin/bash";
    QStringList args;
    args << "-c" << "ps -ax -o 'cmd'|grep '.avfs$'";
    p.start(cmd, args);
    p.waitForFinished();
    QString avfsBase = qgetenv("AVFSBASE");
    QString avfsdDir;
    if(avfsBase.isEmpty()){
        QString home = qgetenv("HOME");
        avfsdDir = home + "/.avfs";
    } else{
        avfsdDir = avfsBase + "/.avfs";
    }

    while (!p.atEnd()) {
        QString result = p.readLine().trimmed();
        if(!result.isEmpty()){
            QStringList datas = result.split(" ");

            if(datas.count() == 2){
                //compare current user's avfs path
                if(datas.last() != avfsdDir)
                    continue;

                if(datas.first() == "avfsd" && QFile::exists(datas.last()))
                    return true;
            }
        }
    }
    return false;
}

QVariant DFileService::processEventSync(const QSharedPointer<DFMEvent> &event) const
{
    QSet<DAbstractFileController*> controller_set;
    QSet<QString> scheme_set;

    for (const DUrl &url : event->handleUrlList()) {
        QList<DAbstractFileController*> list = getHandlerTypeByUrl(url);

        if (!scheme_set.contains(url.scheme()))
            list << getHandlerTypeByUrl(url, true);
        else
            scheme_set << url.scheme();

        for (DAbstractFileController *controller : list) {
            if (controller_set.contains(controller))
                continue;

            controller_set << controller;

            const QVariant &result = DFileServicePrivate::processEventByController(controller, event);

            if (event->isAccepted()) {
                return result;
            }
        }
    }

    if (event->type() == DFMEvent::CreateGetChildrensJob) {
        const QSharedPointer<DFMCreateGetChildrensJob> &e = event.dynamicCast<DFMCreateGetChildrensJob>();

        return QVariant::fromValue(new JobController(e->fileUrl(), e->nameFilters(), e->filters()));
    }

    return QVariant();
}

void DFileService::processEvent(const QSharedPointer<DFMEvent> &event) const
{
    processEventSync(event);
}

QList<DAbstractFileController*> DFileService::getHandlerTypeByUrl(const DUrl &fileUrl,
                                                                 bool ignoreHost, bool ignoreScheme)
{
    HandlerType handlerType(ignoreScheme ? "" : fileUrl.scheme(), ignoreHost ? "" : fileUrl.path());

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
    const DAbstractFileInfoPointer &pInfo =  instance()->createFileInfo(fileUrl);

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

void DFileService::openUrl(const DFMEvent &event, const bool &isOpenInNewWindow, const bool &isOpenInCurrentWindow) const
{
    FILTER_RETURN(OpenUrl)

    if(event.fileUrlList().count() == 0){
        DUrlList urlList;
        urlList << event.fileUrl();
        const_cast<DFMEvent&>(event) << urlList;
    }

    //sort urls by files and dirs`
    DUrlList dirList;
    DFMEvent dirsEvent(event);
    foreach (const DUrl& url, event.fileUrlList()) {

        const DAbstractFileInfoPointer &fileInfo = createFileInfo(url);

        if(globalSetting->isCompressFilePreview()
                && isAvfsMounted()
                && FileUtils::isArchive(url.toLocalFile())
                && fileInfo->mimeType().name() != "application/vnd.debian.binary-package"){
            DAbstractFileInfoPointer info = createFileInfo(DUrl::fromAVFSFile(url.path()));
            if(info->exists()){
                const_cast<DUrl&>(url).setScheme(AVFS_SCHEME);
                dirList << url;
                continue;
            }
        }

        if(fileInfo){
            bool isDir = fileInfo->isDir();
            if(isDir)
                dirList << url;
            else
                openFile(url, event.sender());
        }

        //computer url is virtual dir
        if(url == DUrl::fromComputerFile("/"))
            dirList << url;
    }

    dirsEvent << dirList;

    if(!isOpenInCurrentWindow){
        if(dirList.count() > 0)
            openNewWindow(dirsEvent, isOpenInNewWindow);
    } else{

        if(dirsEvent.fileUrlList().count() == 1){
            //replace dirsEvent's file url with dirsEvent file list's first url which is avfs file
            if(dirsEvent.fileUrlList().first().isAVFSFile())
                dirsEvent << dirsEvent.fileUrlList().first();

            openInCurrentWindow(dirsEvent);
        }
    }
}

void DFileService::laterRequestSelectFiles(const DFMEvent &event) const
{
    FileSignalManager *manager = fileSignalManager;

    TIMER_SINGLESHOT_OBJECT(manager, 200, {
                                manager->requestSelectFile(event);
                            }, event, manager)
}
