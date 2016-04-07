#include "fileservices.h"
#include "abstractfilecontroller.h"

#include "../app/filesignalmanager.h"
#include "../app/fmevent.h"

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
    qRegisterMetaType<FMEvent>("FMEvent");
    qRegisterMetaType<QDir::Filters>("QDir::Filters");
    qRegisterMetaType<QList<AbstractFileInfo*>>("QList<AbstractFileInfo*>");

    connect(fileSignalManager, &FileSignalManager::requestChildren,
            this, &FileServices::getChildren, Qt::QueuedConnection);
    connect(this, &FileServices::childrenUpdated,
            fileSignalManager, &FileSignalManager::childrenChanged,
            Qt::QueuedConnection);
}

FileServices *FileServices::instance()
{
    static FileServices services;

    return &services;
}

void FileServices::setFileUrlHandler(const QString &scheme, const QString &path,
                                     AbstractFileController *controller)
{
    if(m_handlerHash.contains(controller))
        return;

    const HandlerType type = HandlerType(scheme, path);

    m_handlerHash[controller] = type;
    m_controllerHash.insertMulti(type, controller);
}

void FileServices::unsetFileUrlHandler(AbstractFileController *controller)
{
    if(!m_handlerHash.contains(controller))
        return;

    m_controllerHash.remove(m_handlerHash.value(controller), controller);
}

void FileServices::clearFileUrlHandler(const QString &scheme, const QString &path)
{
    m_controllerHash.remove(HandlerType(scheme, path));
}

bool FileServices::openFile(const QString &fileUrl)
{
    TRAVERSE({
                 bool ok = controller->openFile(fileUrl, accepted);

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
                     return std::move(info);
             })

    return Q_NULLPTR;
}

void FileServices::getChildren(const FMEvent &event, QDir::Filters filters) const
{
    const QString &fileUrl = event.dir;

    TRAVERSE({
                 const QList<AbstractFileInfo*> &&list = controller->getChildren(fileUrl, filters, accepted);

                 if(accepted) {
                     emit childrenUpdated(event, std::move(list));

                     return;
                 }
             })
}

QList<AbstractFileController*> FileServices::getHandlerTypeByUrl(const QString &fileUrl,
                                                                 bool ignorePath, bool ignoreScheme)
{
    QUrl url(fileUrl);

    if(url.scheme().isEmpty())
        url = QUrl::fromLocalFile(fileUrl);

    return m_controllerHash.values(HandlerType(ignoreScheme ? "" : url.scheme(),
                                               ignorePath ? "" : url.path()));
}
