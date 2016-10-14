#include "fileservices.h"
#include "abstractfilecontroller.h"
#include "abstractfileinfo.h"

#include "app/filesignalmanager.h"
#include "fmevent.h"
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
    QList<AbstractFileController*> &&list = getHandlerTypeByUrl(url);\
    bool accepted = false;\
    for(AbstractFileController *controller : list) {\
        Code\
    }\
    list = getHandlerTypeByUrl(url, true);\
    for(AbstractFileController *controller : list) {\
        Code\
    }

QMultiHash<const HandlerType, AbstractFileController*> FileServices::m_controllerHash;
QHash<const AbstractFileController*, HandlerType> FileServices::m_handlerHash;
QMultiHash<const HandlerType, std::function<AbstractFileController*()>> FileServices::m_controllerCreatorHash;

FileServices::FileServices(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<FMEvent>("FMEvent");
    qRegisterMetaType<QDir::Filters>("QDir::Filters");
    qRegisterMetaType<QList<AbstractFileInfoPointer>>("QList<AbstractFileInfoPointer>");
    qRegisterMetaType<DUrl>("DUrl");
}

FileServices *FileServices::instance()
{
    static FileServices services;

    return &services;
}

void FileServices::setFileUrlHandler(const QString &scheme, const QString &host,
                                     AbstractFileController *controller)
{
    if(m_handlerHash.contains(controller))
        return;

    const HandlerType &type = HandlerType(scheme, host);

    m_handlerHash[controller] = type;
    m_controllerHash.insertMulti(type, controller);

    connect(controller, &AbstractFileController::childrenAdded,
            instance(), &FileServices::childrenAdded);
    connect(controller, &AbstractFileController::childrenRemoved,
            instance(), &FileServices::childrenRemoved);
    connect(controller, &AbstractFileController::childrenUpdated,
            instance(), &FileServices::childrenUpdated);
}

void FileServices::unsetFileUrlHandler(AbstractFileController *controller)
{
    if(!m_handlerHash.contains(controller))
        return;

    m_controllerHash.remove(m_handlerHash.value(controller), controller);

    disconnect(controller, &AbstractFileController::childrenAdded,
            instance(), &FileServices::childrenAdded);
    disconnect(controller, &AbstractFileController::childrenRemoved,
            instance(), &FileServices::childrenRemoved);
    disconnect(controller, &AbstractFileController::childrenUpdated,
            instance(), &FileServices::childrenUpdated);
}

void FileServices::clearFileUrlHandler(const QString &scheme, const QString &host)
{
    const HandlerType handler(scheme, host);

    for(const AbstractFileController *controller : m_controllerHash.values(handler)) {
        disconnect(controller, &AbstractFileController::childrenAdded,
                instance(), &FileServices::childrenAdded);
        disconnect(controller, &AbstractFileController::childrenRemoved,
                instance(), &FileServices::childrenRemoved);
        disconnect(controller, &AbstractFileController::childrenUpdated,
                instance(), &FileServices::childrenUpdated);
    }

    m_controllerHash.remove(handler);
    m_controllerCreatorHash.remove(handler);
}

bool FileServices::openFile(const DUrl &fileUrl) const
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

bool FileServices::compressFiles(const DUrlList &urlList) const
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


bool FileServices::decompressFile(const DUrlList urllist) const{
    TRAVERSE(urllist.at(0), {
                 bool ok = controller->decompressFile(urllist, accepted);

                 if(accepted) {
                     return ok;
                 }
             })

    return false;
}

bool FileServices::decompressFileHere(const DUrlList urllist) const{
    TRAVERSE(urllist.at(0), {
                 bool ok = controller->decompressFileHere(urllist, accepted);

                 if(accepted) {
                     return ok;
                 }
             })

    return false;
}

bool FileServices::copyFiles(const DUrlList &urlList) const
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

bool FileServices::renameFile(const DUrl &oldUrl, const DUrl &newUrl, const FMEvent &event) const
{
    const AbstractFileInfoPointer &f = createFileInfo(newUrl);

    if (f->exists()){
        dialogManager->showRenameNameSameErrorDialog(f->displayName(), event);
        return false;
    }

    if (renameFile(oldUrl, newUrl)) {
        FMEvent e = event;

        e << DUrlList() << newUrl;

        TIMER_SINGLESHOT(200, {
            emit fileSignalManager->requestSelectFile(e);
        }, e);

        return true;
    }

    return false;
}

bool FileServices::renameFile(const DUrl &oldUrl, const DUrl &newUrl) const
{
    TRAVERSE(oldUrl, {
                 bool ok = controller->renameFile(oldUrl, newUrl, accepted);

                 if (accepted)
                    return ok;
             })

    return false;
}

void FileServices::deleteFiles(const DUrlList &urlList, const FMEvent &event) const
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
            QtConcurrent::run(QThreadPool::globalInstance(), this, &FileServices::deleteFilesSync, urlList, event);
        }

        return;
    }

    deleteFilesSync(urlList, event);
}

bool FileServices::deleteFilesSync(const DUrlList &urlList, const FMEvent &event) const
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

void FileServices::moveToTrash(const DUrlList &urlList) const
{
    if (urlList.isEmpty())
        return;

    if (QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if (QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &FileServices::moveToTrashSync, urlList);

        return;
    }

    moveToTrashSync(urlList);
}

DUrlList FileServices::moveToTrashSync(const DUrlList &urlList) const
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

bool FileServices::cutFiles(const DUrlList &urlList) const
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

void FileServices::pasteFile(const FMEvent &event) const
{
    DFMGlobal::ClipboardAction action = DFMGlobal::instance()->clipboardAction();

    if (action == DFMGlobal::UnknowAction)
        return;

    AbstractFileController::PasteType type = (action == DFMGlobal::CutAction) ? AbstractFileController::CutType
                                                                              : AbstractFileController::CopyType;

    pasteFile(type, DUrl::fromQUrlList(DFMGlobal::instance()->clipboardFileUrlList()), event);
}

void FileServices::pasteFile(AbstractFileController::PasteType type,
                             const DUrlList &urlList, const FMEvent &event) const
{
    if(QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if(QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &FileServices::pasteFile, type, urlList, event);

        return;
    }

    TRAVERSE(event.fileUrl(), {
                 DUrlList list = controller->pasteFile(type, urlList, event, accepted);

                 if(accepted) {
                     FMEvent e = event;

                     e << list;

                     metaObject()->invokeMethod(const_cast<FileServices*>(this), "laterRequestSelectFiles",
                                                Qt::QueuedConnection, Q_ARG(FMEvent, e));

                     return;
                 }
             })
}

void FileServices::restoreFile(const DUrl &srcUrl, const DUrl &tarUrl, const FMEvent &event) const
{
    if(QThreadPool::globalInstance()->activeThreadCount() >= MAX_THREAD_COUNT) {
        qDebug() << "Beyond the maximum number of threads!";
        return;
    }

    if(QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &FileServices::restoreFile, srcUrl, tarUrl, event);

        return;
    }
    TRAVERSE(srcUrl, {
                 controller->restoreFile(srcUrl, tarUrl, event, accepted);

                 if(accepted)
                 return;
             })
}

bool FileServices::newFolder(const FMEvent &event) const
{
    TRAVERSE(event.fileUrl(), {
                 bool ok = controller->newFolder(event, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::newFile(const DUrl &toUrl) const
{
    TRAVERSE(toUrl, {
                 bool ok = controller->newFile(toUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::newDocument(const DUrl &toUrl) const
{
    TRAVERSE(toUrl, {
                 bool ok = controller->newDocument(toUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::addUrlMonitor(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->addUrlMonitor(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::removeUrlMonitor(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->removeUrlMonitor(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::openFileLocation(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->openFileLocation(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::createSymlink(const DUrl &fileUrl, const FMEvent &event) const
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

bool FileServices::createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->createSymlink(fileUrl, linkToUrl, accepted);

                 if(accepted)
                     return ok;
             })

    return false;
}

bool FileServices::sendToDesktop(const FMEvent &event) const
{
    const DUrlList& urls = event.fileUrlList();
    FileUtils::sendToDesktop(urls);

    /// TODO
    return true;
}

bool FileServices::openInTerminal(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->openInTerminal(fileUrl, accepted);

                 if(accepted)
                     return ok;
             })

    return false;
}

void FileServices::openNewWindow(const DUrl &fileUrl) const
{
    emit fileSignalManager->requestOpenNewWindowByUrl(fileUrl, true);
}

const AbstractFileInfoPointer FileServices::createFileInfo(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 const AbstractFileInfoPointer &info = controller->createFileInfo(fileUrl, accepted);

                 if(accepted)
                     return info;
             })

    return AbstractFileInfoPointer();
}

const DDirIteratorPointer FileServices::createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                          QDir::Filters filters, QDirIterator::IteratorFlags flags) const
{
    TRAVERSE(fileUrl, {
                 const DDirIteratorPointer iterator = controller->createDirIterator(fileUrl, nameFilters, filters, flags, accepted);

                 if(accepted)
                     return iterator;
             })

    return DDirIteratorPointer();
}

const QList<AbstractFileInfoPointer> FileServices::getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
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

JobController *FileServices::getChildrenJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                            QDir::Filters filters, QDirIterator::IteratorFlags flags) const
{
    const DDirIteratorPointer &iterator = createDirIterator(fileUrl, nameFilters, filters, flags);

    if (iterator)
        return new JobController(iterator, const_cast<FileServices*>(this));

    return new JobController(fileUrl, nameFilters, filters, const_cast<FileServices*>(this));
}

QList<AbstractFileController*> FileServices::getHandlerTypeByUrl(const DUrl &fileUrl,
                                                                 bool ignoreHost, bool ignoreScheme)
{
    HandlerType handlerType(ignoreScheme ? "" : fileUrl.scheme(), ignoreHost ? "" : fileUrl.path());
    if(m_controllerCreatorHash.contains(handlerType)) {
        QList<AbstractFileController*> list = m_controllerHash.values(handlerType);

        for(const std::function<AbstractFileController*()> &creator : m_controllerCreatorHash.values(handlerType)) {
            AbstractFileController *controller = creator();

            setFileUrlHandler(handlerType.first, handlerType.second, controller);

            list << controller;
        }

        m_controllerCreatorHash.remove(handlerType);

        return list;
    } else {
        return m_controllerHash.values(handlerType);
    }
}

QString FileServices::getSymlinkFileName(const DUrl &fileUrl)
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

void FileServices::openUrl(const FMEvent &event) const
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

void FileServices::laterRequestSelectFiles(const FMEvent &event) const
{
    FileSignalManager *manager = fileSignalManager;

    TIMER_SINGLESHOT_OBJECT(manager, 200, {
                                manager->requestSelectFile(event);
                            }, event, manager)
}
