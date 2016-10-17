#include "dfileservices.h"
#include "dabstractfilecontroller.h"
#include "dabstractfileinfo.h"

#include "app/filesignalmanager.h"
#include "dfmevent.h"
#include "app/define.h"
#include "controllers/jobcontroller.h"
#include "views/windowmanager.h"
#include "models/fileinfo.h"
#include "models/trashfileinfo.h"

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

DWIDGET_USE_NAMESPACE

#define TRAVERSE(url, Code) \
    QList<DAbstractFileController*> &&list = getHandlerTypeByUrl(url);\
    bool accepted = false;\
    for(DAbstractFileController *controller : list) {\
        Code\
    }\
    list = getHandlerTypeByUrl(url, true);\
    for(DAbstractFileController *controller : list) {\
        Code\
    }

QMultiHash<const HandlerType, DAbstractFileController*> DFileService::m_controllerHash;
QHash<const DAbstractFileController*, HandlerType> DFileService::m_handlerHash;
QMultiHash<const HandlerType, std::function<DAbstractFileController*()>> DFileService::m_controllerCreatorHash;

DFileService::DFileService(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<DFMEvent>("FMEvent");
    qRegisterMetaType<QDir::Filters>("QDir::Filters");
    qRegisterMetaType<QList<AbstractFileInfoPointer>>("QList<AbstractFileInfoPointer>");
    qRegisterMetaType<DUrl>("DUrl");
}

DFileService *DFileService::instance()
{
    static DFileService services;

    return &services;
}

void DFileService::setFileUrlHandler(const QString &scheme, const QString &host,
                                     DAbstractFileController *controller)
{
    if(m_handlerHash.contains(controller))
        return;

    const HandlerType &type = HandlerType(scheme, host);

    m_handlerHash[controller] = type;
    m_controllerHash.insertMulti(type, controller);

    connect(controller, &DAbstractFileController::childrenAdded,
            instance(), &DFileService::childrenAdded);
    connect(controller, &DAbstractFileController::childrenRemoved,
            instance(), &DFileService::childrenRemoved);
    connect(controller, &DAbstractFileController::childrenUpdated,
            instance(), &DFileService::childrenUpdated);
}

void DFileService::unsetFileUrlHandler(DAbstractFileController *controller)
{
    if(!m_handlerHash.contains(controller))
        return;

    m_controllerHash.remove(m_handlerHash.value(controller), controller);

    disconnect(controller, &DAbstractFileController::childrenAdded,
            instance(), &DFileService::childrenAdded);
    disconnect(controller, &DAbstractFileController::childrenRemoved,
            instance(), &DFileService::childrenRemoved);
    disconnect(controller, &DAbstractFileController::childrenUpdated,
            instance(), &DFileService::childrenUpdated);
}

void DFileService::clearFileUrlHandler(const QString &scheme, const QString &host)
{
    const HandlerType handler(scheme, host);

    for(const DAbstractFileController *controller : m_controllerHash.values(handler)) {
        disconnect(controller, &DAbstractFileController::childrenAdded,
                instance(), &DFileService::childrenAdded);
        disconnect(controller, &DAbstractFileController::childrenRemoved,
                instance(), &DFileService::childrenRemoved);
        disconnect(controller, &DAbstractFileController::childrenUpdated,
                instance(), &DFileService::childrenUpdated);
    }

    m_controllerHash.remove(handler);
    m_controllerCreatorHash.remove(handler);
}

bool DFileService::openFile(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->openFile(fileUrl, accepted);

                 if(accepted) {
                     emit fileOpened(fileUrl);

                     return ok;
                 }
             })

            return false;
}

bool DFileService::compressFiles(const DUrlList &urlList) const
{
    if (urlList.isEmpty())
        return false;
    TRAVERSE(urlList.first(), {
                 bool ok = controller->compressFiles(urlList, accepted);

                 if(accepted) {
                     return ok;
                 }
             })

    return false;
}


bool DFileService::decompressFile(const DUrlList urllist) const{
    TRAVERSE(urllist.at(0), {
                 bool ok = controller->decompressFile(urllist, accepted);

                 if(accepted) {
                     return ok;
                 }
             })

    return false;
}

bool DFileService::decompressFileHere(const DUrlList urllist) const{
    TRAVERSE(urllist.at(0), {
                 bool ok = controller->decompressFileHere(urllist, accepted);

                 if(accepted) {
                     return ok;
                 }
             })

    return false;
}

bool DFileService::copyFiles(const DUrlList &urlList) const
{
    if(urlList.isEmpty())
        return false;

    TRAVERSE(urlList.first(), {
                 bool ok = controller->copyFiles(urlList, accepted);

                 if(accepted)
                     return ok;
             })

     return false;
}

bool DFileService::renameFile(const DUrl &oldUrl, const DUrl &newUrl, const DFMEvent &event) const
{
    const AbstractFileInfoPointer &f = createFileInfo(newUrl);

    if (f->exists()){
        dialogManager->showRenameNameSameErrorDialog(f->displayName(), event);
        return false;
    }

    if (renameFile(oldUrl, newUrl)) {
        DFMEvent e = event;

        e << DUrlList() << newUrl;

        TIMER_SINGLESHOT(200, {
            emit fileSignalManager->requestSelectFile(e);
        }, e);

        return true;
    }

    return false;
}

bool DFileService::renameFile(const DUrl &oldUrl, const DUrl &newUrl) const
{
    TRAVERSE(oldUrl, {
                 bool ok = controller->renameFile(oldUrl, newUrl, accepted);

                 if (accepted)
                    return ok;
             })

    return false;
}

void DFileService::deleteFiles(const DUrlList &urlList, const DFMEvent &event) const
{
    if (urlList.isEmpty())
        return;

    if (QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if (QThread::currentThread() == qApp->thread()) {
        int result = dialogManager->showDeleteFilesClearTrashDialog(event);

        if (result == 1) {
            QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileService::deleteFilesSync, urlList, event);
        }

        return;
    }

    deleteFilesSync(urlList, event);
}

bool DFileService::deleteFilesSync(const DUrlList &urlList, const DFMEvent &event) const
{
    if (urlList.isEmpty())
        return false;

    TRAVERSE(urlList.first(), {
                 bool ok =controller->deleteFiles(urlList, event, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

void DFileService::moveToTrash(const DUrlList &urlList) const
{
    if (urlList.isEmpty())
        return;

    if (QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if (QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileService::moveToTrashSync, urlList);

        return;
    }

    moveToTrashSync(urlList);
}

DUrlList DFileService::moveToTrashSync(const DUrlList &urlList) const
{
    if (urlList.isEmpty())
        return urlList;

    TRAVERSE(urlList.first(), {
                 DUrlList list = controller->moveToTrash(urlList, accepted);

                 if (accepted)
                    return list;
             })

    return DUrlList();
}

bool DFileService::cutFiles(const DUrlList &urlList) const
{
    if(urlList.isEmpty())
        return false;

    TRAVERSE(urlList.first(), {
                 bool ok = controller->cutFiles(urlList, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

void DFileService::pasteFile(const DFMEvent &event) const
{
    DFMGlobal::ClipboardAction action = DFMGlobal::instance()->clipboardAction();

    if (action == DFMGlobal::UnknowAction)
        return;

    DAbstractFileController::PasteType type = (action == DFMGlobal::CutAction) ? DAbstractFileController::CutType
                                                                              : DAbstractFileController::CopyType;

    pasteFile(type, DUrl::fromQUrlList(DFMGlobal::instance()->clipboardFileUrlList()), event);
}

void DFileService::pasteFile(DAbstractFileController::PasteType type,
                             const DUrlList &urlList, const DFMEvent &event) const
{
    if(QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if(QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &DFileService::pasteFile, type, urlList, event);

        return;
    }

    TRAVERSE(event.fileUrl(), {
                 DUrlList list = controller->pasteFile(type, urlList, event, accepted);

                 if(accepted) {
                     DFMEvent e = event;

                     e << list;

                     metaObject()->invokeMethod(const_cast<DFileService*>(this), "laterRequestSelectFiles",
                                                Qt::QueuedConnection, Q_ARG(DFMEvent, e));

                     return;
                 }
             })
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

bool DFileService::newFolder(const DFMEvent &event) const
{
    TRAVERSE(event.fileUrl(), {
                 bool ok = controller->newFolder(event, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool DFileService::newFile(const DUrl &toUrl) const
{
    TRAVERSE(toUrl, {
                 bool ok = controller->newFile(toUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool DFileService::newDocument(const DUrl &toUrl) const
{
    TRAVERSE(toUrl, {
                 bool ok = controller->newDocument(toUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool DFileService::addUrlMonitor(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->addUrlMonitor(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool DFileService::removeUrlMonitor(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->removeUrlMonitor(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool DFileService::openFileLocation(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->openFileLocation(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool DFileService::createSymlink(const DUrl &fileUrl, const DFMEvent &event) const
{
//    QString linkName = getSymlinkFileName(fileUrl);
//    QString linkPath = QFileDialog::getSaveFileName(WindowManager::getWindowById(event.windowId()),
//                                                    QObject::tr("Create symlink"), linkName);

//    return createSymlink(fileUrl, DUrl::fromLocalFile(linkPath));

    int windowId = event.windowId();
    FileUtils::createSoftLink(windowId, fileUrl.toLocalFile());

    /// TODO
    return true;
}

bool DFileService::createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->createSymlink(fileUrl, linkToUrl, accepted);

                 if(accepted)
                     return ok;
             })

    return false;
}

bool DFileService::sendToDesktop(const DFMEvent &event) const
{
    const DUrlList& urls = event.fileUrlList();
    FileUtils::sendToDesktop(urls);

    /// TODO
    return true;
}

bool DFileService::openInTerminal(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->openInTerminal(fileUrl, accepted);

                 if(accepted)
                     return ok;
             })

    return false;
}

void DFileService::openNewWindow(const DUrl &fileUrl) const
{
    emit fileSignalManager->requestOpenNewWindowByUrl(fileUrl, true);
}

const AbstractFileInfoPointer DFileService::createFileInfo(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 const AbstractFileInfoPointer &info = controller->createFileInfo(fileUrl, accepted);

                 if(accepted)
                     return info;
             })

    return AbstractFileInfoPointer();
}

const DDirIteratorPointer DFileService::createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                          QDir::Filters filters, QDirIterator::IteratorFlags flags) const
{
    TRAVERSE(fileUrl, {
                 const DDirIteratorPointer iterator = controller->createDirIterator(fileUrl, nameFilters, filters, flags, accepted);

                 if(accepted)
                     return iterator;
             })

    return DDirIteratorPointer();
}

const QList<AbstractFileInfoPointer> DFileService::getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                               QDir::Filters filters, QDirIterator::IteratorFlags flags, bool *ok)
{
    TRAVERSE(fileUrl, {
                 const QList<AbstractFileInfoPointer> list = controller->getChildren(fileUrl, nameFilters, filters, flags, accepted);

                 if (ok)
                    *ok = accepted;

                 if(accepted)
                     return list;
             })

    if (ok)
        *ok = false;

    return QList<AbstractFileInfoPointer>();
}

JobController *DFileService::getChildrenJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                            QDir::Filters filters, QDirIterator::IteratorFlags flags) const
{
    const DDirIteratorPointer &iterator = createDirIterator(fileUrl, nameFilters, filters, flags);

    if (iterator)
        return new JobController(iterator, const_cast<DFileService*>(this));

    return new JobController(fileUrl, nameFilters, filters, const_cast<DFileService*>(this));
}

QList<DAbstractFileController*> DFileService::getHandlerTypeByUrl(const DUrl &fileUrl,
                                                                 bool ignoreHost, bool ignoreScheme)
{
    HandlerType handlerType(ignoreScheme ? "" : fileUrl.scheme(), ignoreHost ? "" : fileUrl.path());
    if(m_controllerCreatorHash.contains(handlerType)) {
        QList<DAbstractFileController*> list = m_controllerHash.values(handlerType);

        for(const std::function<DAbstractFileController*()> &creator : m_controllerCreatorHash.values(handlerType)) {
            DAbstractFileController *controller = creator();

            setFileUrlHandler(handlerType.first, handlerType.second, controller);

            list << controller;
        }

        m_controllerCreatorHash.remove(handlerType);

        return list;
    } else {
        return m_controllerHash.values(handlerType);
    }
}

QString DFileService::getSymlinkFileName(const DUrl &fileUrl)
{
    const AbstractFileInfoPointer &fileInfo = instance()->createFileInfo(fileUrl);

    QString fileName = fileInfo->fileName();

    if (fileInfo->isFile()) {
        int index = fileName.lastIndexOf('.');

        if (index >= 0)
            fileName.insert(index, " link");
        else
            fileName.append(" link");
    } else {
        return fileName + " link";
    }

    return fileName;
}

void DFileService::openUrl(const DFMEvent &event) const
{
    const AbstractFileInfoPointer &fileInfo = createFileInfo(event.fileUrl());

    if (fileInfo && fileInfo->isDir()) {
        emit fileSignalManager->requestChangeCurrentUrl(event);
    } else if (deviceListener->isDeviceFolder(event.fileUrl().path()) && fileInfo->isDir()) {
        emit fileSignalManager->requestChangeCurrentUrl(event);
    } else {
        openFile(event.fileUrl());
    }
}

void DFileService::laterRequestSelectFiles(const DFMEvent &event) const
{
    FileSignalManager *manager = fileSignalManager;

    TIMER_SINGLESHOT_OBJECT(manager, 200, {
                                manager->requestSelectFile(event);
                            }, event, manager)
}
