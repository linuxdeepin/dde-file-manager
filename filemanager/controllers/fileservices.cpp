#include "fileservices.h"
#include "abstractfilecontroller.h"

#include "../app/filesignalmanager.h"
#include "../app/fmevent.h"

#include "../models/abstractfileinfo.h"

#include <QUrl>
#include <QDebug>

#define TRAVERSE(Code) \
    QList<AbstractFileController*> &&list = getHandlerTypeByUrl(fileUrl);\
    bool accepted = false;\
    for(AbstractFileController *controller : list) {\
        Code\
    }\
    list = getHandlerTypeByUrl(fileUrl, true);\
    for(AbstractFileController *controller : list) {\
        Code\
    }

QMultiHash<HandlerType, AbstractFileController*> FileServices::m_controllerHash;
QHash<AbstractFileController*, HandlerType> FileServices::m_handlerHash;

FileServices::FileServices()
    : QObject()
{
    m_thread = new QThread(this);

    moveToThread(m_thread);

    qRegisterMetaType<FMEvent>("FMEvent");
    qRegisterMetaType<QDir::Filters>("QDir::Filters");
    qRegisterMetaType<QList<AbstractFileInfo*>>("QList<AbstractFileInfo*>");

    connect(fileSignalManager, &FileSignalManager::requestChildren,
            this, &FileServices::getChildren, Qt::QueuedConnection);
    connect(this, &FileServices::childrenUpdated,
            fileSignalManager, &FileSignalManager::childrenChanged,
            Qt::QueuedConnection);
    connect(fileSignalManager, &FileSignalManager::requestOpenFile,
            this, &FileServices::openFile, Qt::QueuedConnection);
    connect(this, &FileServices::fileOpened,
            fileSignalManager, &FileSignalManager::fileOpened
            , Qt::QueuedConnection);

    m_thread->start();
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
    TRAVERSE({
                 bool ok = controller->openFile(fileUrl, accepted);

                 if(accepted) {
                     emit fileOpened(fileUrl);

                     return ok;
                 }
             })

    return false;
}

bool FileServices::renameFile(const QString &fileUrl, const QString &newUrl) const
{
    TRAVERSE({
                 bool ok = controller->renameFile(fileUrl, newUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::addUrlMonitor(const QString &fileUrl) const
{
    TRAVERSE({
                 bool ok = controller->addUrlMonitor(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

    return false;
}

bool FileServices::removeUrlMonitor(const QString &fileUrl) const
{
    TRAVERSE({
                 bool ok = controller->removeUrlMonitor(fileUrl, accepted);

                 if(accepted)
                    return ok;
             })

     return false;
}

AbstractFileInfo *FileServices::createFileInfo(const QString &fileUrl) const
{
    TRAVERSE({
                 AbstractFileInfo *info = controller->createFileInfo(fileUrl, accepted);

                 if(accepted)
                     return info;
             })

    return Q_NULLPTR;
}

void FileServices::getChildren(const FMEvent &event, QDir::Filters filters) const
{
    if(QThread::currentThread() != m_thread) {
        QMetaObject::invokeMethod(const_cast<FileServices*>(this), "getChildren", Qt::QueuedConnection,
                                  Q_ARG(FMEvent, event), Q_ARG(QDir::Filters, filters));

        return;
    }

    const QString &fileUrl = event.fileUrl();

    TRAVERSE({
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
