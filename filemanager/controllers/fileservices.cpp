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

QMultiHash<const HandlerType, AbstractFileController*> FileServices::m_controllerHash;
QHash<const AbstractFileController*, HandlerType> FileServices::m_handlerHash;
QMultiHash<const HandlerType, std::function<AbstractFileController*()>> FileServices::m_controllerCreatorHash;

FileServices::FileServices(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<FMEvent>("FMEvent");
    qRegisterMetaType<QDir::Filters>("QDir::Filters");
    qRegisterMetaType<QList<AbstractFileInfo*>>("QList<AbstractFileInfo*>");
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
        connect(controller, &AbstractFileController::childrenUpdated,
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

bool FileServices::decompressFile(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 bool ok = controller->decompressFile(fileUrl, accepted);

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

bool FileServices::renameFile(const DUrl &oldUrl, const DUrl &newUrl) const
{
    TRAVERSE(oldUrl, {
                 bool ok = controller->renameFile(oldUrl, newUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

void FileServices::deleteFiles(const DUrlList &urlList) const
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

void FileServices::moveToTrash(const DUrlList &urlList) const
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
    const QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    const QByteArray &data = mimeData->data("x-special/gnome-copied-files");

    if(!data.isEmpty()) {
        QTextStream text(data);
        AbstractFileController::PasteType type = text.readLine() == "cut" ? AbstractFileController::CutType
                                                                          : AbstractFileController::CopyType;

        DUrlList urls;

        while(!text.atEnd()) {
            urls.append(DUrl(text.readLine()));
        }

        pasteFile(type, urls, event);
    }
}

void FileServices::pasteFile(AbstractFileController::PasteType type,
                             const DUrlList &urlList, const FMEvent &event) const
{
    if(QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &FileServices::pasteFile, type, urlList, event);

        return;
    }

    TRAVERSE(event.fileUrl(), {
                 controller->pasteFile(type, urlList, event, accepted);

                 if(accepted)
                 return;
             })
}

bool FileServices::newFolder(const DUrl &toUrl) const
{
    TRAVERSE(toUrl, {
                 bool ok = controller->newFolder(toUrl, accepted);

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

void FileServices::openNewWindow(const DUrl &fileUrl) const
{
    emit fileSignalManager->requestOpenNewWindowByUrl(fileUrl);
}

AbstractFileInfo *FileServices::createFileInfo(const DUrl &fileUrl) const
{
    TRAVERSE(fileUrl, {
                 AbstractFileInfo *info = controller->createFileInfo(fileUrl, accepted);

                 if(accepted)
                     return info;
             })

            return Q_NULLPTR;
}

const QList<AbstractFileInfo*> FileServices::getChildren(const DUrl &fileUrl, QDir::Filters filters, bool *ok) const
{
    QList<AbstractFileInfo*> childrenList;

    TRAVERSE(fileUrl, {
                 childrenList = controller->getChildren(fileUrl, filters, accepted);

                 if(accepted) {
                     if(ok)
                        *ok = accepted;
                     return childrenList;
                 }
             })

     if(ok)
         *ok = accepted;

     return childrenList;
}

void FileServices::getChildren(const FMEvent &event, QDir::Filters filters) const
{
    if(QThread::currentThread() == qApp->thread()) {
        QtConcurrent::run(QThreadPool::globalInstance(), this, &FileServices::getChildren, event, filters);

        return;
    }

    const DUrl &fileUrl = event.fileUrl();

    bool accepted = false;

    const QList<AbstractFileInfo*> &childrenList = getChildren(fileUrl, filters, &accepted);

    if(accepted)
        emit updateChildren(event, childrenList);
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

void FileServices::openUrl(const FMEvent &event) const
{
    qDebug() << event;
    AbstractFileInfo *fileInfo = createFileInfo(event.fileUrl());

    if(fileInfo && fileInfo->isDir()) {
        fileSignalManager->requestChangeCurrentUrl(event);
        delete fileInfo;

        return;
    }

    delete fileInfo;

    openFile(event.fileUrl());
}
