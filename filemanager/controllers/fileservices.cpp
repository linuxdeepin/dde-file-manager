#include "fileservices.h"
#include "abstractfilecontroller.h"

#include "../app/filesignalmanager.h"
#include "../app/fmevent.h"
#include "../app/global.h"

#include "../models/abstractfileinfo.h"

#include <QUrl>
#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>

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

QMultiHash<HandlerType, AbstractFileController*> FileServices::m_controllerHash;
QHash<AbstractFileController*, HandlerType> FileServices::m_handlerHash;

FileServices::FileServices(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<FMEvent>("FMEvent");
    qRegisterMetaType<QDir::Filters>("QDir::Filters");
    qRegisterMetaType<QList<AbstractFileInfo*>>("QList<AbstractFileInfo*>");
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

    const HandlerType type = HandlerType(scheme, host);

    m_handlerHash[controller] = type;
    m_controllerHash.insertMulti(type, controller);

    connect(controller, &AbstractFileController::childrenAdded,
            instance(), &FileServices::childrenAdded);
    connect(controller, &AbstractFileController::childrenRemoved,
            instance(), &FileServices::childrenRemoved);
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
}

void FileServices::clearFileUrlHandler(const QString &scheme, const QString &host)
{
    const HandlerType handler(scheme, host);

    for(const AbstractFileController *controller : m_controllerHash.values(handler)) {
        connect(controller, &AbstractFileController::childrenAdded,
                instance(), &FileServices::childrenAdded);
        connect(controller, &AbstractFileController::childrenRemoved,
                instance(), &FileServices::childrenRemoved);
    }

    m_controllerHash.remove(handler);
}

bool FileServices::openFile(const QString &fileUrl) const
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

bool FileServices::copyFiles(const QList<QString> &urlList) const
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

bool FileServices::renameFile(const QString &oldUrl, const QString &newUrl) const
{
    TRAVERSE(oldUrl, {
                 bool ok = controller->renameFile(oldUrl, newUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

void FileServices::deleteFiles(const QList<QString> &urlList) const
{
    if(urlList.isEmpty())
        return;

    if(QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &FileServices::deleteFiles, urlList);

        return;
    }

    TRAVERSE(urlList.first(), {
                 controller->deleteFiles(urlList, accepted);

                 if(accepted)
                    return;
             })
}

void FileServices::moveToTrash(const QList<QString> &urlList) const
{
    if(urlList.isEmpty())
        return;

    if(QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &FileServices::moveToTrash, urlList);

        return;
    }

    TRAVERSE(urlList.first(), {
                 controller->moveToTrash(urlList, accepted);

                 if(accepted)
                    return;
             })
}

bool FileServices::cutFiles(const QList<QString> &urlList) const
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

void FileServices::pasteFile(const QString &toUrl) const
{
    const QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    const QByteArray &data = mimeData->data("x-special/gnome-copied-files");

    if(!data.isEmpty()) {
        QTextStream text(data);
        AbstractFileController::PasteType type = text.readLine() == "cut" ? AbstractFileController::CutType
                                                                          : AbstractFileController::CopyType;

        QList<QString> urls;

        while(!text.atEnd()) {
            urls.append(text.readLine());
        }

        pasteFile(type, urls, toUrl);
    }
}

void FileServices::pasteFile(AbstractFileController::PasteType type,
                             const QList<QString> &urlList, const QString &toUrl) const
{
    if(QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &FileServices::pasteFile, type, urlList, toUrl);

        return;
    }

    TRAVERSE(toUrl, {
                 controller->pasteFile(type, urlList, toUrl, accepted);

                 if(accepted)
                 return;
             })
}

bool FileServices::newFolder(const QString &toUrl) const
{
    TRAVERSE(toUrl, {
                 bool ok = controller->newFolder(toUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::newFile(const QString &toUrl) const
{
    TRAVERSE(toUrl, {
                 bool ok = controller->newFile(toUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::newDocument(const QString &toUrl) const
{
    TRAVERSE(toUrl, {
                 bool ok = controller->newDocument(toUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::addUrlMonitor(const QString &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->addUrlMonitor(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::removeUrlMonitor(const QString &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->removeUrlMonitor(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::openFileLocation(const QString &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->openFileLocation(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

void FileServices::openNewWindow(const QString &fileUrl) const
{
    emit fileSignalManager->requestOpenNewWindowByUrl(fileUrl);
}

AbstractFileInfo *FileServices::createFileInfo(const QString &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 AbstractFileInfo *info = controller->createFileInfo(fileUrl, accepted);

                 if(accepted)
                     return info;
             })

    return Q_NULLPTR;
}

void FileServices::getChildren(const FMEvent &event, QDir::Filters filters) const
{
    if(QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &FileServices::getChildren, event, filters);

        return;
    }

    const QString &fileUrl = event.fileUrl();

    TRAVERSE(fileUrl, {
                 const QList<AbstractFileInfo*> &&list = controller->getChildren(fileUrl, filters, accepted);

                 if(accepted) {
                     emit childrenUpdated(event, std::move(list));

                     return;
                 }
             })
}

QList<AbstractFileController*> FileServices::getHandlerTypeByUrl(const QString &fileUrl,
                                                                 bool ignoreHost, bool ignoreScheme)
{
    QUrl url(fileUrl);

    return m_controllerHash.values(HandlerType(ignoreScheme ? "" : url.scheme(),
                                               ignoreHost ? "" : url.path()));
}

void FileServices::openUrl(const FMEvent &event) const
{
    AbstractFileInfo *fileInfo = createFileInfo(event.fileUrl());

    if(fileInfo && fileInfo->isDir()) {
        fileSignalManager->requestChangeCurrentUrl(event);
        delete fileInfo;

        return;
    }

    delete fileInfo;

    openFile(event.fileUrl());
}
